#include "rv_os_mem.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif


void rv_os_mem_init() {
	#ifdef _WIN32
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	rv_os_mem.large_page_size = GetLargePageMinimum();
	rv_os_mem.page_size = sys_info.dwPageSize;
	#else // Unix
	rv_os_mem.large_page_size = sysconf(_SC_PAGESIZE) * 512; // 512 pages is a common large page size (2MB on x86-64)
	rv_os_mem.page_size = sysconf(_SC_PAGESIZE);
	#endif
};

rv_os_mem_api rv_os_mem = {
    malloc,
    calloc,
    realloc,
    free,
};
