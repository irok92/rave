#pragma once

#include <cstddef>

#ifndef RAVE_ASSERT
#ifdef NDEBUG
#define RAVE_ASSERT(cond) ((void)0)
#else
#if defined(_MSC_VER)
#define RAVE_ASSERT(cond) do { if (!(cond)) __debugbreak(); } while(0)
#elif defined(__GNUC__) || defined(__clang__)
#define RAVE_ASSERT(cond) do { if (!(cond)) __builtin_trap(); } while(0)
#else
#include <cstdlib>
#define RAVE_ASSERT(cond) do { if (!(cond)) abort(); } while(0)
#endif
#endif
#endif

namespace Rave {

    constexpr size_t ARENA_RESERVE_SIZE = 1ull << 30;
    constexpr size_t ARENA_COMMIT_SIZE  = 1ull << 16;

    void* mem_copy(void* dst, const void* src, size_t size);

    struct Arena;

    struct ArenaBlock {
        Arena* arena;
        void*  start;
        void*  current;
        void*  end;

        static ArenaBlock* create(Arena* a, size_t size, size_t alignment);

        void* pos();
        void* capacity();

        void  align(size_t alignment);
        void* push(size_t size, size_t alignment);
        bool  can_push(size_t size, size_t alignment);

        void pop_to(void* pos);
        void pop_to(size_t size);

        template<typename T>
        inline T* push() {
            return static_cast<T*>(push(sizeof(T), alignof(T)));
        }

        template<typename T>
        inline T* push(size_t count) {
            return static_cast<T*>(push(sizeof(T) * count, alignof(T)));
        }

        template<typename T>
        inline T* copy(const T& value) {
            T* p = push<T>();
            mem_copy(p, &value, sizeof(T));
            return p;
        }

        template<typename T>
        inline T* copy_arr(const T* data, size_t count) {
            T* p = push<T>(count);
            mem_copy(p, data, sizeof(T) * count);
            return p;
        }
    };

    struct Arena {
        void*  start;
        void*  current;
        size_t reserved;
        size_t committed;
        size_t page_size;

        static Arena create();

        void destroy();
        void* pos();

        void  align(size_t alignment);
        void* push(size_t size, size_t alignment);

        void pop_to(void* pos);
        void pop_to(size_t size);

        template<typename T>
        inline T* push() {
            return static_cast<T*>(push(sizeof(T), alignof(T)));
        }

        template<typename T>
        inline T* push(size_t count) {
            return static_cast<T*>(push(sizeof(T) * count, alignof(T)));
        }

        template<typename T, size_t Alignment>
        inline T* push_aligned() {
            return static_cast<T*>(push(sizeof(T), Alignment));
        }

        template<typename T, size_t Alignment>
        inline T* push_aligned(size_t count) {
            return static_cast<T*>(push(sizeof(T) * count, Alignment));
        }

        template<typename T>
        inline T* copy(const T& value) {
            T* p = push<T>();
            mem_copy(p, &value, sizeof(T));
            return p;
        }

        template<typename T>
        inline T* copy_arr(const T* data, size_t count) {
            T* p = push<T>(count);
            mem_copy(p, data, sizeof(T) * count);
            return p;
        }
    };

}
