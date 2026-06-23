#include "arena.hpp"
#include <cstring>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <sys/mman.h>
	#include <unistd.h>
#endif

namespace Rave {

void*
mem_copy(
	void*		dst,
	const void* src,
	size_t		size
) {
	return memcpy(dst, src, size);
}

static size_t
get_page_size() {
	static size_t page_size = [] {
#ifdef _WIN32
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return info.dwPageSize;
#else
		return sysconf(_SC_PAGE_SIZE);
#endif
	}();
	return page_size;
}

// ----------------------------------------------------------------
// Arena
// ----------------------------------------------------------------

Arena
Arena::create() {
	Arena a		= {};
	a.page_size = get_page_size();
	a.reserved	= ARENA_RESERVE_SIZE;

#ifdef _WIN32
	a.start = VirtualAlloc(NULL, a.reserved, MEM_RESERVE, PAGE_NOACCESS);
#else
	a.start = mmap(NULL, a.reserved, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (a.start == MAP_FAILED)
		a.start = nullptr;
#endif

	a.current	= a.start;
	a.committed = 0;
	return a;
}

void
Arena::destroy() {
	if (!start)
		return;
#ifdef _WIN32
	VirtualFree(start, 0, MEM_RELEASE);
#else
	munmap(start, reserved);
#endif
	start	  = nullptr;
	current	  = nullptr;
	reserved  = 0;
	committed = 0;
}

void*
Arena::pos() {
	return current;
}

void
Arena::align(size_t alignment) {
	size_t mask = alignment - 1;
	current		= (void*)(((uintptr_t)current + mask) & ~mask);
}

void*
Arena::push(
	size_t size,
	size_t alignment
) {
	align(alignment);
	void* ptr = current;
	void* end = (void*)((uintptr_t)ptr + size);
	if ((uintptr_t)end > (uintptr_t)start + committed) {
		size_t need	  = (uintptr_t)end - ((uintptr_t)start + committed);
		size_t commit = (need + ARENA_COMMIT_SIZE - 1) & ~(ARENA_COMMIT_SIZE - 1);
#ifdef _WIN32
		void* result = VirtualAlloc((char*)start + committed, commit, MEM_COMMIT, PAGE_READWRITE);
		if (!result)
			return nullptr;
#else
		void* result = mmap(
			(char*)start + committed, commit, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0
		);
		if (result == MAP_FAILED)
			return nullptr;
#endif
		committed += commit;
	}
	current = end;
	return ptr;
}

void
Arena::pop_to(void* pos) {
	if (pos >= start && pos <= current) {
		current = pos;
	}
}

void
Arena::pop_to(size_t size) {
	void* target = (void*)((uintptr_t)current - size);
	if (target >= start) {
		current = target;
	}
}

// ----------------------------------------------------------------
// ArenaBlock
// ----------------------------------------------------------------

ArenaBlock*
ArenaBlock::create(
	Arena* a,
	size_t size,
	size_t alignment
) {
	ArenaBlock* hdr = a->push<ArenaBlock>();
	if (!hdr)
		return nullptr;
	hdr->arena = a;
	hdr->start = a->push(size, alignment);
	if (!hdr->start)
		return nullptr;
	hdr->current = hdr->start;
	hdr->end	 = (void*)((uintptr_t)hdr->start + size);
	return hdr;
}

void*
ArenaBlock::pos() {
	return current;
}

void*
ArenaBlock::capacity() {
	return end;
}

void
ArenaBlock::align(size_t alignment) {
	size_t mask = alignment - 1;
	current		= (void*)(((uintptr_t)current + mask) & ~mask);
}

void*
ArenaBlock::push(
	size_t size,
	size_t alignment
) {
	RAVE_ASSERT(start >= arena->start && end <= arena->current);
	align(alignment);
	void* ptr = current;
	current	  = (void*)((uintptr_t)current + size);
	if (current > end) {
		current = ptr;
		return nullptr;
	}
	return ptr;
}

bool
ArenaBlock::can_push(
	size_t size,
	size_t alignment
) {
	RAVE_ASSERT(start >= arena->start && end <= arena->current);
	size_t mask	   = alignment - 1;
	void*  aligned = (void*)(((uintptr_t)current + mask) & ~mask);
	return (void*)((uintptr_t)aligned + size) <= end;
}

void
ArenaBlock::pop_to(void* pos) {
	if (pos >= start && pos <= current) {
		current = pos;
	}
}

void
ArenaBlock::pop_to(size_t size) {
	void* target = (void*)((uintptr_t)current - size);
	if (target >= start) {
		current = target;
	}
}

} // namespace Rave
