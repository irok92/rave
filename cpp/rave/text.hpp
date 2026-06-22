#pragma once

#include "arena.hpp"

namespace Rave {

    namespace Text {



        template<typename T = char>
        struct Span {
            const T* data;
            size_t size;
        };


        template<typename T = char>
        struct Rope {
            Span<T> left;
            Span<T> right;
        };

        template<typename T = char>
        struct Pool {

        };

        template<typename T = char>
        struct Index {

        };

        template<typename T = char>
        struct Context {
            Arena* arena;
            Pool<T>* pool;
            Index<T>* index;
        };
    }

}