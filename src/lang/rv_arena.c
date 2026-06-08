#include "rv_arena.h"

rv_arena*
rv_arena_create(
	rv_arena_flags flags,
	rv_u64			reserve_size,
	rv_u64			commit_size,
	void*		backing_buffer,
	const rv_c8*	file,
	int			line
) {

	void* base = backing_buffer;

	rv_u64 actual_reserve_size = reserve_size;
	rv_u64 actual_commit_size	= commit_size;

	if(base == RV_NULL) {
		if (flags & RV_ARENA_LARGE_PAGES) {
			actual_reserve_size = align_pow2(actual_reserve_size, rv_os_mem.large_page_size);
			actual_commit_size	 = align_pow2(actual_commit_size, rv_os_mem.large_page_size);
			base = rv_os_reserve_large(actual_commit_size);
			rv_os_commit_large(base, actual_commit_size);
		} else {
			actual_reserve_size = align_pow2(actual_reserve_size, rv_os_mem.page_size);
			actual_commit_size	 = align_pow2(actual_commit_size, rv_os_mem.page_size);
			base = rv_os_reserve(actual_reserve_size);
			rv_os_commit(base, actual_commit_size);
		}

		RV_ASAN_POISON_MEM(base, actual_commit_size);
	} else {
		RV_ASAN_POISON_MEM(base, actual_reserve_size);
	}

	RV_ASAN_UNPOISON_MEM(base, RV_ARENA_HEADER_SIZE);
	rv_arena* arena	     = base;
	arena->current		 = arena;
	arena->flags			 = flags;
	arena->size_committed = commit_size;
	arena->size_reserved	 = reserve_size;
	arena->position		 = RV_ARENA_HEADER_SIZE;
	arena->position_base	 = 0;
	arena->commited		 = actual_commit_size;
	arena->reserved		 = actual_reserve_size;
	arena->file			 = file;
	arena->line			 = line;
	arena->free_last		 = RV_NULL;

	RV_ASAN_UNPOISON_MEM(base, RV_ARENA_HEADER_SIZE);

	return arena;
}

void
rv_arena_release(rv_arena* a) {

	for (rv_arena *n = a->current, *prev = RV_NULL; n != RV_NULL; n = prev) {
		prev = n->previous;
		RV_ASAN_UNPOISON_MEM(n, n->commited);
		rv_os_release(n, n->reserved);
	}
}

void*
rv_arena_push(
	rv_arena* arena,
	rv_u64	 size,
	rv_u64	 alignment,
	rv_bool zero
) {
	rv_arena*  c		 = arena->current;
	rv_u64	   pre_pos	 = align_pow2(c->position, alignment);
	rv_u64	   post_pos = pre_pos + size;

	rv_u64 size_to_zero = 0;
	if (zero) {
		size_to_zero = (c->commited < post_pos ? c->commited : post_pos) - pre_pos;
	}

	if (c->reserved < post_pos && !(arena->flags & RV_ARENA_NO_CHAIN)) {
		rv_arena* new_block = RV_NULL;

		// Free list
		{
			rv_arena* prev_block;
			for (
				new_block = arena->free_last, prev_block = RV_NULL;
				new_block != RV_NULL;
				prev_block = new_block, new_block = new_block->previous
			) {
				if (new_block->reserved >= align_pow2(new_block->position, alignment) + size) {
					if (prev_block != RV_NULL) {
						prev_block->previous = new_block->previous;
					} else {
						arena->free_last = new_block->previous;
					}
					break;
				}
			}
		}

		if (new_block == RV_NULL) {
			rv_u64 reserve_size = c->size_reserved;
			rv_u64 commit_size	 = c->size_committed;

			if (size + RV_ARENA_HEADER_SIZE > reserve_size) {
				reserve_size = align_pow2(size + RV_ARENA_HEADER_SIZE, alignment);
				commit_size	 = align_pow2(size + RV_ARENA_HEADER_SIZE, alignment);
			}

			new_block = rv_arena_create(
				c->flags,
				reserve_size,
				commit_size,
				RV_NULL,
				c->file,
				c->line
			);

			size_to_zero = 0;
		} else {
			size_to_zero = size;
		}

		new_block->position_base = c->position_base + c->reserved;
		new_block->previous      = arena->current, arena->current = new_block;

		c		  = new_block;
		pre_pos  = align_pow2(c->position, alignment);
		post_pos = pre_pos + size;
	}




	if (c->commited < post_pos) {
		rv_u64 cmt_post_a = post_pos + c->size_committed - 1;
		cmt_post_a -= cmt_post_a % c->size_committed;
		rv_u64 cmt_post_clamp = (cmt_post_a < c->reserved ? cmt_post_a : c->reserved);
		rv_u64 cmt_size = cmt_post_clamp - c->commited;
		rv_u8* cmt_ptr	= (rv_u8*)(c) + c->commited;

		if (c->flags & RV_ARENA_LARGE_PAGES) {
			rv_os_commit_large(cmt_ptr, cmt_size);
		} else {
			rv_os_commit(cmt_ptr, cmt_size);
		}
		RV_ASAN_POISON_MEM(cmt_ptr, cmt_size);
		c->commited += cmt_post_clamp;
	}

	void* result = RV_NULL;

	if (c->commited >= post_pos) {
		result			  = (rv_u8*)(c) + pre_pos;
		c->position = post_pos;
		RV_ASAN_UNPOISON_MEM(result, size);
		if (size_to_zero != 0) {
			rv_os_memset((rv_u8*)(result), 0, size_to_zero);
		}
	}

	return result;
}

rv_u64
rv_arena_get_pos(rv_arena* arena) {
	return arena->current->position_base + arena->current->position;
}


void
rv_arena_pop_to(rv_arena* arena, rv_u64 pos) {
	rv_u64	   big_pos = RV_ARENA_HEADER_SIZE > pos ? RV_ARENA_HEADER_SIZE : pos;
	rv_arena*  current = arena->current;

	for (rv_arena* prev = 0; current->position_base >= big_pos; current = prev) {
		prev			  = current->previous;
		current->position = RV_ARENA_HEADER_SIZE;
		current->previous = arena->free_last, arena->free_last = current;
		RV_ASAN_POISON_MEM(
			(rv_u8*)(current) + RV_ARENA_HEADER_SIZE,
			current->reserved - RV_ARENA_HEADER_SIZE
		);
	}

	arena->current			 = current;
	rv_u64 new_pos = big_pos - current->position_base;
	RV_ASSERT(new_pos <= current->position);
	RV_ASAN_POISON_MEM(
		(rv_u8*)(current) + new_pos, current->position - new_pos
	);
	current->position = new_pos;
}

void
rv_arena_pop(rv_arena* arena, rv_u64 amount) {
	rv_u64 pos_old = rv_arena_get_pos(arena);
	rv_u64 pos_new = pos_old;
	if(amount < pos_old) {
		pos_new = pos_old - amount;
	}

	rv_arena_pop_to(arena, pos_new);

}

void
rv_arena_clear(rv_arena* arena) {
	rv_arena_pop_to(arena, 0);
}

rv_arena_marker
rv_arena_start_marker(rv_arena* arena) {
	return (rv_arena_marker){ arena, rv_arena_get_pos(arena) };
}

void
rv_arena_end_marker(rv_arena* arena, rv_arena_marker marker) {
	rv_arena_pop_to(arena, marker.position);
}
