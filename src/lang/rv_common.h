#pragma once

#ifdef __cplusplus
#define RV_OPT(...) = __VA_ARGS__
#else
#define RV_OPT(...)
#endif

#include "rv_alloc.h"
