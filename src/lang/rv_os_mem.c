#include "rv_os_mem.h"
#include "rv_common.h"

#include <stdlib.h>




#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/mman.h>
#endif

void
rv_os_mem_init() {
#ifdef _WIN32
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	rv_os_mem.large_page_size = GetLargePageMinimum();
	rv_os_mem.page_size		  = sys_info.dwPageSize;
#else // Unix
	rv_os_mem.large_page_size =
		sysconf(_SC_PAGESIZE) * 512; // 512 pages is a common large page size (2MB on x86-64)
	rv_os_mem.page_size = sysconf(_SC_PAGESIZE);
#endif
};


void* rv_os_mem_impl_aligned_alloc(rv_usize alignment, rv_usize size) {
#ifdef _WIN32
	return _aligned_malloc(size, alignment);
#else // Unix
	void* ptr = RV_NULL;
	if (posix_memalign(&ptr, alignment, size) != 0) {
		return RV_NULL;
	}
	return ptr;
#endif
}

void rv_os_mem_impl_aligned_free(void* ptr) {
#ifdef _WIN32
	_aligned_free(ptr);
#else // Unix
	free(ptr);
#endif
}

void*
rv_os_mem_impl_reserve(rv_u64 size) {
#ifdef _WIN32
	return VirtualAlloc(RV_NULL, (rv_usize)size, MEM_RESERVE, PAGE_READWRITE);
#else // Unix
	return mmap(RV_NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
}

void*
rv_os_mem_api_reserve_large(rv_u64 size) {
#ifdef _WIN32
	return VirtualAlloc(
		RV_NULL, (rv_usize)size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE
	);
#else // Unix
	return mmap(RV_NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
#endif
}


rv_bool rv_os_mem_impl_commit(void* ptr, rv_u64 size) {
#ifdef _WIN32
	return VirtualAlloc(ptr, (rv_usize)size, MEM_COMMIT, PAGE_READWRITE) != RV_NULL;
#else // Unix
	return mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0;
#endif
}

rv_bool rv_os_mem_impl_commit_large(void* ptr, rv_u64 size) {
#ifdef _WIN32
	return VirtualAlloc(ptr, (rv_usize)size, MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE) != RV_NULL;
#else // Unix
	return mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0;
#endif
}

void rv_os_mem_impl_decommit(void* ptr, rv_u64 size) {
#ifdef _WIN32
	VirtualFree(ptr, (rv_usize)size, MEM_DECOMMIT);
#else // Unix
	mprotect(ptr, size, PROT_NONE);
#endif
}

void rv_os_mem_impl_release(void* ptr, rv_u64 size) {
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else // Unix
	munmap(ptr, size);
#endif
}


rv_os_mem_api rv_os_mem = {
	malloc,
	calloc,
	realloc,
	free,
	rv_os_mem_impl_aligned_alloc,
	rv_os_mem_impl_aligned_free,
	rv_os_mem_impl_reserve,
	rv_os_mem_api_reserve_large,
	rv_os_mem_impl_commit,
	rv_os_mem_impl_commit_large,
	rv_os_mem_impl_decommit,
	rv_os_mem_impl_release,
	0, // large_page_size will be initialized in rv_os_mem_init()
	0  // page_size will be initialized in rv_os_mem_init()
};
