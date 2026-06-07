#pragma once

#ifndef RV_COMMON_H
#define RV_COMMON_H

#ifdef __cplusplus
#define RV_OPT(...) = __VA_ARGS__
#else
#define RV_OPT(...)
#endif

#include <stdint.h>
#include <stdbool.h>


typedef int8_t rv_s8;
typedef int16_t rv_s16;
typedef int32_t rv_s32;
typedef int64_t rv_s64;
typedef uint8_t rv_u8;
typedef uint16_t rv_u16;
typedef uint32_t rv_u32;
typedef uint64_t rv_u64;
typedef float rv_f32;
typedef double rv_f64;
typedef char rv_c8;
typedef wchar_t rv_c16;
typedef size_t rv_usize;
typedef bool rv_bool;

#define RV_TRUE true
#define RV_FALSE false
#ifdef __cplusplus
#define RV_NULL nullptr
#else
#define RV_NULL NULL
#endif

#endif // RV_COMMON_H
