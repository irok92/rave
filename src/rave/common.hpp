#pragma once

#include <corecrt_memcpy_s.h>
#include <corecrt_wstring.h>
#include <cstdint>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>


#include <mimalloc.h>

#include <type_traits>

#ifndef RAVE_ASSERT
	#ifdef NDEBUG
		#define RAVE_ASSERT(cond) ((void)0)
	#else
		#if defined(_MSC_VER)
			#define RAVE_ASSERT(cond)                                                              \
				do {                                                                               \
					if (!(cond))                                                                   \
						__debugbreak();                                                            \
				} while (0)
		#elif defined(__GNUC__) || defined(__clang__)
			#define RAVE_ASSERT(cond)                                                              \
				do {                                                                               \
					if (!(cond))                                                                   \
						__builtin_trap();                                                          \
				} while (0)
		#else
			#include <cstdlib>
			#define RAVE_ASSERT(cond)                                                              \
				do {                                                                               \
					if (!(cond))                                                                   \
						abort();                                                                   \
				} while (0)
		#endif
	#endif
#endif

namespace Rave {

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using b8	= bool;
using usize = size_t;

using c8  = char;
using c16 = char16_t;
using c32 = char32_t;

template <typename T>
struct Ptr {
	T* data;

  public:
	T*
	operator->() const {
		return data;
	}

	T
	operator*() const {
		return *data;
	}

	operator T() const {
		return *data;
	}

	operator T*() const {
		return data;
	}
};

template <typename T>
struct SpanIterator {
	using value_type	  = T;
	using difference_type = usize;
	using pointer		  = Ptr<T>;
	using reference		  = T&;

	T* ptr;

	reference
	operator*() const {
		return *ptr;
	}

	pointer
	operator->() const {
		return {ptr};
	}

	SpanIterator&
	operator++() {
		++ptr;
		return *this;
	}

	SpanIterator
	operator++(int) {
		auto tmp = *this;
		++ptr;
		return tmp;
	}

	friend bool
	operator==(
		const SpanIterator& a,
		const SpanIterator& b
	) {
		return a.ptr == b.ptr;
	}

	friend bool
	operator!=(
		const SpanIterator& a,
		const SpanIterator& b
	) {
		return a.ptr != b.ptr;
	}
};

template <typename T>
struct Span {
  public:
	T*	  data;
	usize size;

	operator T*() const {
		return data;
	}

	Ptr<T>
	operator[](size_t index) const {
		return data + index;
	}

	Span<T>
	slice(
		usize start,
		usize count
	) {
		RAVE_ASSERT((start + count) < size);
		return {data + start, count};
	}

	SpanIterator<T>
	begin() {
		return {data};
	}

	SpanIterator<T>
	end() {
		return {data + size};
	}

	SpanIterator<T>
	slice_or_end(usize count) {
		return {data + (count < size ? count : size)};
	}
};

template <typename T>
Ptr<T>
New() {
	T* data = reinterpret_cast<T*>(mi_calloc(1, sizeof(T)));
	if constexpr (std::is_destructible_v<T>) {
		data->~T();
	}
	return {data};
}

template <
	typename T,
	typename... Args>
Ptr<T>
New(Args... args) {
	T* data = mi_calloc(1, sizeof(T));
	data->T(args...);
	return {data};
}

template <typename T>
Span<T>
AllocArray(usize count) {
	T* ptr = mi_callocn(count, sizeof(T));
	if (!ptr) {
		// TODO: Replace with better error handling
		return {nullptr, 0};
	}

	return {ptr, count};
}

template <typename T>
Span<T>
NewArray(usize count) {
	T* ptr = mi_callocn(count, sizeof(T));
	if (!ptr) {
		// TODO: Replace with better error handling.
		return {nullptr, 0};
	}

	for (usize i = 0; i < count; i++) {
		if constexpr(std::is_destructible_v<T>) {	
			ptr[i]->T();
		}
	}

	return {ptr, count};
}

template <
	typename T,
	typename... TArgs>
Span<T>
NewArray(
	usize count,
	TArgs... args
) {
	T* ptr = mi_callocn(count, sizeof(T));
	if (!ptr) {
		// TODO: Replace with better error handling.
		return {nullptr, 0};
	}

	for (usize i = 0; i < count; i++) {
		if(std::is_constructible_v<T>) {
			ptr[i]->T(args...);
		}
	}

	return {ptr, count};
}

template <typename T>
void
Delete(T* ptr) {
	if (ptr) {
		if constexpr(std::is_destructible_v<T>) {
			ptr->~T();
		}
		mi_free(ptr);
	}
}

template <typename T>
void
Delete(Ptr<T> ptr) {
	if (ptr.data) {
		Delete(ptr.data);
	}
}

template <typename T>
void
Delete(T& value) {
	if constexpr(std::is_destructible_v<T>) {
		value.~T();
	}
}

template <typename T>
void
Delete(Span<T> span) {
	for (usize i = 0; i < span.size; i++) {
		Delete(span.data + i);
	}
	mi_free(span.data);
}

template <typename T>
struct Vector {
	Span<T> buffer;
	usize	cursor;

	Vector(usize capacity = 0) {
		if (capacity > 0) {
			resize(capacity);
		}
	}

	~Vector() {
		cursor = 0;
		Delete(buffer);
	}

	bool
	resize(usize new_size) {
		if (buffer->data == nullptr) {
			buffer = AllocArray<T>(new_size);
			RAVE_ASSERT(buffer != nullptr);
		} else {
			buffer.data = mi_realloc(buffer.data, new_size);
			buffer.size = new_size;
		}
	}

	bool
	grow(usize grow_by = 0) {
		usize new_size = buffer.size + grow_by;
		if (grow_by == 0) {
			// TODO Plan in better allocation strategy.
			// Increase by 50%.
			new_size = buffer.size + (buffer.size >> 1);
		}
		return resize(new_size);
	}

	void
	push(T& value) {
		usize index = cursor;
		if ((cursor + 2) > buffer.size) {
			grow();
		}
		buffer[++cursor] = value;
	}

	/*
	 * Returns a span that can be iterated through with new elements at the end of the array.
	 */
	const Span<T>
	push_n(usize count) {
		if ((cursor + count + 1) > buffer.size) {
			grow(count);
		}
		Span<T> slice = buffer.slice(cursor, count);
		cursor += count;

		return slice;
	}

	void
	pop() {
		cursor--;
		RAVE_ASSERT(cursor >= 0);
	}

	void 
	pop_n(usize count) {
		cursor = max(0, cursor - count);
	}

	void
	pop_delete() {
		buffer[cursor]->~T();
		cursor--;
		RAVE_ASSERT(cursor >= 0);
	}

	T&
	operator[](usize index) {
		return buffer[index];
	}

	SpanIterator<T>
	begin() {
		return buffer.begin();
	};

	SpanIterator<T>
	end() {
		return buffer.slice_or_end(cursor);
	};

	const Span<T>
	slice(usize start, usize count) {
		return buffer.slice(start, (start+count > cursor) ? cursor :  count);
	}
};


static constexpr inline usize MAX_STRLEN_SIZE = UINT32_MAX;

/* Make copying from Cstring explicit to be aware of cost */
template <typename TChar>
static const inline Span<TChar>
CopyFromCstr(
	const TChar* start,
	TChar*		 end = nullptr
) {
	// Size has to be calculated if end is nullptr.
	usize size = end - start;
	if (end == nullptr) {
		if constexpr (std::is_same_v<TChar, c8>()) {
			size = strnlen_s(start, MAX_STRLEN_SIZE);
		} else if constexpr (std::is_same_v<TChar, c16>()) {
			size = wcsnlen_s(start, MAX_STRLEN_SIZE);
		}
		end = start + size;
	}

	Span<TChar> arr = NewArray(size);

	if constexpr (std::is_same_v<TChar, c8>) {
		memcpy_s(arr.data, arr.size, start, size);
	} else if constexpr (std::is_same_v<TChar, c16>) {
		wmemcpy_s(arr.data, arr.size, start, size);
	}

	return arr;
}

using UTF8Span		= Span<c8>;
using Unicode16Span = Span<c16>;


template<typename TChar>
struct StringBuffer {
	Vector<TChar> data;

	StringBuffer(TChar* start, TChar* end = nullptr) {
		data.buffer = CopyFromCstr(start, end);
	}


};

} // namespace Rave