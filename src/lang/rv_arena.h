#pragma once

#include "rv_common.h"
#include "rv_os_mem.h"

#define RAVE_NEW_ARENA(...) rv_arena_create(__VA_ARGS__, __FILE__, __LINE__)

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct rv_arena_marker {
	rv_arena* arena;
	rv_u64	   position;
} rv_arena_marker;

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
);

void
rv_arena_release(rv_arena* a);

void*
rv_arena_push(
	rv_arena* arena,
	rv_u64	 size,
	rv_u64	 alignment,
	rv_bool zero
);

rv_u64
rv_arena_get_pos(rv_arena* arena);

void
rv_arena_pop_to(rv_arena* arena, rv_u64 position);

void
rv_arena_pop(rv_arena* arena, rv_u64 amount);

void
rv_arena_clear(rv_arena* arena);

rv_arena_marker
rv_arena_start_marker(rv_arena* arena);

void
rv_arena_end_marker(rv_arena* arena, rv_arena_marker marker);


#define rv_arena_push_t(arena, T) (T *)rv_arena_push(arena, sizeof(T), alignof(T), false)
#define rv_arena_push_t_zero(arena, T) (T *)rv_arena_push(arena, sizeof(T), alignof(T), true)
#define rv_arena_push_array(arena, T, count) (T *)rv_arena_push(arena, sizeof(T) * count, alignof(T), false)
#define rv_arena_push_array_zero(arena, T, count) (T *)rv_arena_push(arena, sizeof(T) * count, alignof(T), true)
#define rv_arena_push_array_aligned(arena, T, count, align) (T *)rv_arena_push(arena, sizeof(T) * count, align, false)
#define rv_arena_push_array_zero_aligned(arena, T, count, align) (T *)rv_arena_push(arena, sizeof(T) * count, align, true)

#define rv_arena_pop_t(arena, T) rv_arena_pop(arena, sizeof(T))
#define rv_arena_pop_array_t(arena, T, count) rv_arena_pop(arena, sizeof(T) * count)

#ifdef __cplusplus
}
#endif

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
	return (T*)rv_arena_push(arena, sizeof(T) * count, align, true);
}

template <typename T>
T inline*
rv_arena_push_array_no_zero(
	rv_arena* arena,
	rv_u64	   count
) {
	return rv_arena_push_n_no_zero_aligned<T>(arena, count, RV_MAX(8ULL, alignof(T)));
}

template <typename T>
T inline*
rv_arena_push_n(
	rv_arena* arena,
	rv_u64	   count
) {
	return rv_arena_push_n_aligned<T>(arena, count, RV_MAX(8ULL, alignof(T)));
}
#endif
