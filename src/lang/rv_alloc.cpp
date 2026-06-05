#include <cstdlib>
#include <rv_alloc.h>


rv_allocators g_rv_allocators = {
    malloc,
    calloc,
    realloc,
    free
};

