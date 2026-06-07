#pragma once

#include "rv_common.h"
#include "rv_os_mem.h"

#define RAVE_NEW_ARENA(...) rv_arena_create(__VA_ARGS__, __FILE__, __LINE__)


typedef enum rv_arena_flags : rv_u64 {
	RV_ARENA_NO_FLAGS	  = 0,
	RV_ARENA_NO_CHAIN	  = 1 << 0,
	RV_ARENA_LARGE_PAGES = 1 << 1,
} rv_arena_flags;

typedef struct rv_arena rv_arena;

typedef struct rv_arena {
	rv_arena*		previous;
	rv_arena*		current;
	rv_arena_flags flags;
	rv_u64			size_committed;
	rv_u64			size_reserved;
	rv_u64			position;
	rv_u64			position_base;
	rv_u64			commited;
	rv_u64			reserved;
	const rv_c8*	file;
	rv_s32			line;
	rv_arena*		free_last;
} rv_arena;

typedef struct arena_marker {
	rv_arena* arena;
	rv_u64	   position;
} arena_marker;

const rv_u64 static RV_ARENA_DEFAULT_COMMIT_SIZE	 = 64 * 1024;
const rv_u64 static RV_ARENA_DEFAULT_RESERVE_SIZE	 = 64 * 1024 * 1024;
const rv_arena_flags static RV_ARENA_DEFAULT_FLAGS = RV_ARENA_NO_FLAGS;
const rv_u64 static RV_ARENA_HEADER_SIZE			 = 128;

rv_u64 inline align_pow2(
	rv_u64 x,
	rv_u64 b
) {
	return (x + (b - 1)) & ~(b - 1);
}



static_assert(
	sizeof(rv_arena) <= RV_ARENA_HEADER_SIZE,
	"arena size too large"
);


rv_arena*
rv_arena_create(
	rv_arena_flags flags,
	rv_u64			reserve_size,
	rv_u64			commit_size,
	void*		backing_buffer,
	const rv_c8*	file,
	int			line
) {
	if (flags & RV_ARENA_LARGE_PAGES) {
		reserve_size = align_pow2(reserve_size, rv_os_mem.large_page_size);
		commit_size	 = align_pow2(commit_size, rv_os_mem.large_page_size);
	} else {
		reserve_size = align_pow2(reserve_size, rv_os_mem.page_size);
		commit_size	 = align_pow2(commit_size, rv_os_mem.page_size);
	}

	void* base = backing_buffer;
	if (base == RV_NULL) {
		if (flags & RV_ARENA_LARGE_PAGES) {
			base = rv_os_reserve_large(reserve_size);
			rv_os_commit_large(base, commit_size);
		} else {
			base = rv_os_reserve(reserve_size);
			rv_os_commit(base, commit_size);
		}
	}

	rv_arena* self			 = (rv_arena*)(base);
	self->current		 = self;
	self->flags			 = flags;
	self->size_committed = commit_size;
	self->size_reserved	 = reserve_size;
	self->position		 = RV_ARENA_HEADER_SIZE;
	self->position_base	 = 0;
	self->commited		 = commit_size;
	self->reserved		 = reserve_size;
	self->file			 = file;
	self->line			 = line;
	self->free_last		 = RV_NULL;

	ASAN_POISON_MEMORY_REGION(base, commit_size);
	ASAN_UNPOISON_MEMORY_REGION(base, RV_ARENA_HEADER_SIZE);

	return self;
}

void
rv_arena_release(rv_arena* a) {

	for (rv_arena *n = a->current, *prev = RV_NULL; n != RV_NULL; n = prev) {
		prev = n->previous;
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
	rv_arena* current		 = arena->current;
	rv_u64	   pre_position	 = align_pow2(current->position, alignment);
	rv_u64	   post_position = pre_position + size;

	if (current->reserved < post_position && !(arena->flags & RV_ARENA_NO_CHAIN)) {
		rv_arena* new_block = RV_NULL;

		{
			rv_arena* prev_block;
			for (new_block = arena->free_last, prev_block = RV_NULL; new_block != RV_NULL;
				 prev_block = new_block, new_block = new_block->previous) {
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
			rv_u64 reserve_size = current->size_reserved;
			rv_u64 commit_size	 = current->size_committed;

			if (size + RV_ARENA_HEADER_SIZE > reserve_size) {
				reserve_size = align_pow2(size + RV_ARENA_HEADER_SIZE, alignment);
				commit_size	 = align_pow2(size + RV_ARENA_HEADER_SIZE, alignment);
			}

			new_block = rv_arena_create(
				current->flags, reserve_size, commit_size, RV_NULL, current->file, current->line
			);
		}

		new_block->position_base = current->position_base + current->reserved;
		// push stack n
		new_block->previous = arena->current, arena->current = new_block;

		current		  = new_block;
		pre_position  = align_pow2(current->position, alignment);
		post_position = pre_position + size;
	}

	rv_u64 size_to_zero = 0;
	if (zero) {
		size_to_zero = (current->commited < post_position ? current->commited : post_position) -
					   pre_position;
	}

	if (current->commited < post_position) {
		rv_u64 commit_post_aligned = post_position + current->size_committed - 1;
		commit_post_aligned -= commit_post_aligned % current->size_committed;
		rv_u64 commit_post_clamped =
			(commit_post_aligned < current->reserved ? commit_post_aligned : current->reserved);
		rv_u64 commit_size = commit_post_clamped - current->commited;
		rv_u8* commit_ptr	= (rv_u8*)(current) + current->commited;
		if (current->flags & RV_ARENA_LARGE_PAGES) {
			rv_os_commit_large(commit_ptr, commit_size);
		} else {
			rv_os_commit(commit_ptr, commit_size);
		}

		current->commited += commit_post_clamped;
	}

	void* result = RV_NULL;

	if (current->commited >= post_position) {
		result			  = (rv_u8*)(current) + pre_position;
		current->position = post_position;
		ASAN_UNPOISON_MEMORY_REGION(result, size);
		if (size_to_zero != 0) {
			rv_os_memset((rv_u8*)(result), 0, size_to_zero);
		}
	}

	return result;
}

inline rv_u64
rv_arena_get_pos(rv_arena* arena) {
	return arena->current->position_base + arena->current->position;
}

inline void
rv_arena_pop_to(rv_arena* arena, rv_u64 position) {
	rv_u64	   big_position = RV_ARENA_HEADER_SIZE > position ? RV_ARENA_HEADER_SIZE : position;
	rv_arena* current		= arena->current;

	for (rv_arena* prev = 0; current->position_base >= big_position; current = prev) {
		prev			  = current->previous;
		current->position = RV_ARENA_HEADER_SIZE;
		current->previous = arena->free_last, arena->free_last = current;
		ASAN_POISON_MEMORY_REGION(
			(rv_u8*)(current) + RV_ARENA_HEADER_SIZE,
			current->reserved - RV_ARENA_HEADER_SIZE
		);
	}
	current			 = current;
	rv_u64 new_position = big_position - current->position_base;
	RV_ASSERT(new_position <= current->position);
	ASAN_POISON_MEMORY_REGION(
		(rv_u8*)(current) + new_position, current->position - new_position
	);
	current->position = new_position;
}

void
rv_arena_clear(rv_arena* arena) {
	rv_arena_pop_to(arena, 0);
}

arena_marker
rv_arena_start_marker(rv_arena* arena) {
	return (arena_marker){ arena, rv_arena_get_pos(arena) };
}

void
rv_arena_end_marker(rv_arena* arena, arena_marker marker) {
	rv_arena_pop_to(arena, marker.position);
}

#ifdef __cplusplus
template <typename T>
T*
rv_arena_new(rv_arena* arena) {
	rv_u64	  size = sizeof(T);
	void* mem  = rv_arena_push(arena, size, alignof(T), false);
	return new (mem) T;
}

template <typename T>
T*
rv_arena_new_n(
	rv_arena* arena,
	rv_u64	   count
) {
	rv_u64	  size = sizeof(T) * count;
	void* mem  = rv_arena_push(arena, size, alignof(T), false);
	return new (mem) T[count];
}

template <typename T>
T inline*
rv_arena_push_n_no_zero_aligned(
	rv_arena* arena,
	rv_u64	   count,
	rv_u64	   align
) {
	return (T*)rv_arena_push(arena, sizeof(T) * count, align, false);
}

template <typename T>
T inline*
rv_arena_push_n_aligned(
	rv_arena* arena,
	rv_u64	   count,
	rv_u64	   align
) {
	return (T*)push_arena(arena, sizeof(T) * count, align, true);
}

template <typename T>
T inline*
rv_arena_push_array_no_zero(
	rv_arena* arena,
	rv_u64	   count
) {
	return rv_arena_push_n_no_zero_aligned<T>(arena, count, max(8ULL, alignof(T)));
}

template <typename T>
T inline*
rv_arena_push_n(
	rv_arena* arena,
	rv_u64	   count
) {
	return rv_arena_push_n_aligned<T>(arena, count, max(8ULL, alignof(T)));
}
#endif
