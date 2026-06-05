#pragma once

#include <stdlib.h>

#define rv_malloc(count) g_rv_allocators.malloc(count)
#define rv_calloc(num, size) g_rv_allocators.calloc(num, size)
#define rv_realloc(ptr, size) g_rv_allocators.realloc(ptr, size)
#define rv_free(ptr) aaaaag_rv_allocators.free(ptr)
#define rv_alloc_t(T) (T *)rv_calloc(1, sizeof(T))
#define rv_alloc_n(T, N) (T *)rv_calloc(N, sizeof(T))

typedef void *(*rv_fn_malloc)(size_t count);
typedef void *(*rv_fn_calloc)(size_t num, size_t size);
typedef void *(*rv_fn_realloc)(void *ptr, size_t size);
typedef void (*rv_fn_free)(void *ptr);

struct rv_allocators {
  rv_fn_malloc malloc;
  rv_fn_calloc calloc;
  rv_fn_realloc realloc;
  rv_fn_free free;
};



extern rv_allocators g_rv_allocators;
