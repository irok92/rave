#pragma once

#ifndef RV_COMMON_H
#define RV_COMMON_H

#ifdef __cplusplus
#define RV_OPT(...) = __VA_ARGS__
#else
#define RV_OPT(...)
#endif

#define RV_ASSERT(x)				  assert(x)
#define RV_STATIC_ASSERT(cond, str) static_assert(cond, str)

#define RV_SIZEOF(x)	(sizeof(x))
#define RV_COUNTOF(x)   (sizeof(x) / sizeof(x[0]))

#if defined(_MSC_VER)
	#if defined(__SANITIZE_ADDRESS__)
		#define RV_ASAN_ENABLED 1
		#define RV_NO_ASAN		 __declspec(no_sanitize_address)
	#else
		#define RV_NO_ASAN
	#endif
#elif defined(__clang__)
	#if defined(__has_feature)
		#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
			#define RV_ASAN_ENABLED 1
		#endif
	#endif
	#define RV_NO_ASAN __attribute__((no_sanitize("address")))
#else
	#define RV_NO_ASAN
#endif

#if RV_ASAN_ENABLED
C_LINKAGE void
__asan_poison_memory_region(
	void const volatile* addr,
	size_t				 size
);
C_LINKAGE void
__asan_unpoison_memory_region(
	void const volatile* addr,
	size_t				 size
);
	#define RV_ASAN_POISON_MEM(addr, size)	__asan_poison_memory_region((addr), (size))
	#define RV_ASAN_UNPOISON_MEM(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
	#define RV_ASAN_POISON_MEM(addr, size)	((void)(addr), (void)(size))
	#define RV_ASAN_UNPOISON_MEM(addr, size) ((void)(addr), (void)(size))
#endif

#ifndef RV_HAS_BUILTIN
	#ifdef __has_builtin
		#define RV_HAS_BUILTIN(x) __has_builtin(x)
	#else
		#define RV_HAS_BUILTIN(x) 0
	#endif
#endif

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>


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

#define RV_MAX(a, b) (((a) > (b)) ? (a) : (b))

#endif // RV_COMMON_H
