#ifndef RV_ALLOC_H
#define RV_ALLOC_H

#include "rv_common.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define rv_os_malloc(count) rv_os_mem.malloc(count)
#define rv_os_calloc(num, size) rv_os_mem.calloc(num, size)
#define rv_os_realloc(ptr, size) rv_os_mem.realloc(ptr, size)
#define rv_os_free(ptr) rv_os_mem.free(ptr)
#define rv_os_alloc_t(T) (T *)rv_os_calloc(1, sizeof(T))
#define rv_os_alloc_n(T, N) (T *)rv_os_calloc(N, sizeof(T))
#define rv_os_memset(ptr, value, count) memset(ptr, value, count)
#define rv_os_memcpy(dest, src, count) memcpy(dest, src, count)
#define rv_os_memmove(dest, src, count) memmove(dest, src, count)


#define rv_os_aligned_alloc(alignment, size) rv_os_mem.aligned_alloc(alignment, size)
#define rv_os_aligned_free(ptr) rv_os_mem.aligned_free(ptr)
#define rv_os_reserve(size) rv_os_mem.reserve(size)
#define rv_os_reserve_large(size) rv_os_mem.reserve_large(size)
#define rv_os_commit(ptr, size) rv_os_mem.commit(ptr, size)
#define rv_os_commit_large(ptr, size) rv_os_mem.commit_large(ptr, size)
#define rv_os_decommit(ptr, size) rv_os_mem.decommit(ptr, size)
#define rv_os_release(ptr, size) rv_os_mem.release(ptr, size)


// Regular allocations functions
typedef void *(*rv_os_fn_malloc)(rv_usize count);
typedef void *(*rv_os_fn_calloc)(rv_usize num, rv_usize size);
typedef void *(*rv_os_fn_realloc)(void *ptr, rv_usize size);
typedef void (*rv_os_fn_free)(void *ptr);

// Aligned allocation functions
typedef void* (*rv_os_fn_aligned_alloc)(rv_usize alignment, rv_usize size);
typedef void (*rv_os_fn_aligned_free)(void *ptr);

// memory mapped api functions.
typedef void* (*rv_os_fn_reserve)(rv_u64 size);
typedef void* (*rv_os_fn_reserve_large)(rv_u64 size);
typedef rv_bool (*rv_os_fn_commit)(void* ptr, rv_u64 size);
typedef rv_bool (*rv_os_fn_commit_large)(void* ptr, rv_u64 size);
typedef void (*rv_os_fn_decommit)(void* ptr, rv_u64 size);
typedef void (*rv_os_fn_release)(void* ptr, rv_u64 size);


typedef struct rv_os_mem_api {
  rv_os_fn_malloc malloc;
  rv_os_fn_calloc calloc;
  rv_os_fn_realloc realloc;
  rv_os_fn_free free;

  rv_os_fn_aligned_alloc aligned_alloc;
  rv_os_fn_aligned_free aligned_free;

  rv_os_fn_reserve reserve;
  rv_os_fn_reserve_large reserve_large;
  rv_os_fn_commit commit;
  rv_os_fn_commit_large commit_large;
  rv_os_fn_decommit decommit;
  rv_os_fn_release release;

  rv_u64 large_page_size;
  rv_u64 page_size;
} rv_os_mem_api;


extern rv_os_mem_api rv_os_mem;

void rv_os_mem_init();

#ifdef __cplusplus
}
#endif

#endif// RV_ALLOC_H
