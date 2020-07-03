#include <3ds/svc.h>
#include <3ds/allocator/mappable.h>
#include <3ds/env.h>
#include <3ds/os.h>
#include <3ds/result.h>

#define HEAP_SPLIT_SIZE_CAP  (24 << 20) // 24MB
#define LINEAR_HEAP_SIZE_CAP (32 << 20) // 32MB

extern char* fake_heap_start;
extern char* fake_heap_end;

u32 __ctru_heap;
u32 __ctru_linear_heap;

__attribute__((weak)) u32 __ctru_heap_size        = 0;
__attribute__((weak)) u32 __ctru_linear_heap_size = 0;

void __attribute__((weak)) __system_allocateHeaps(void) {
	Result rc;

	// Retrieve handle to the resource limit object for our process
	Handle reslimit = 0;
	rc = svcGetResourceLimit(&reslimit, CUR_PROCESS_HANDLE);
	if (R_FAILED(rc))
		svcBreak(USERBREAK_PANIC);

	// Retrieve information about total/used memory
	s64 maxCommit = 0, currentCommit = 0;
	ResourceLimitType reslimitType = RESLIMIT_COMMIT;
	svcGetResourceLimitLimitValues(&maxCommit, reslimit, &reslimitType, 1); // for APPLICATION this is equal to APPMEMALLOC at all times
	svcGetResourceLimitCurrentValues(&currentCommit, reslimit, &reslimitType, 1);
	svcCloseHandle(reslimit);

	// Calculate how much remaining free memory is available
	u32 remaining = (u32)(maxCommit - currentCommit) &~ 0xFFF;

	if (__ctru_heap_size + __ctru_linear_heap_size > remaining)
		svcBreak(USERBREAK_PANIC);

	if (__ctru_heap_size == 0 && __ctru_linear_heap_size == 0) {
		// Split available memory equally between linear and application heaps (with rounding in favor of the latter)
		__ctru_linear_heap_size = (remaining / 2) & ~0xFFF;
		__ctru_heap_size = remaining - __ctru_linear_heap_size;

		// If the application heap size is bigger than the cap, prefer to grow linear heap instead
		if (__ctru_heap_size > HEAP_SPLIT_SIZE_CAP) {
			__ctru_heap_size = HEAP_SPLIT_SIZE_CAP;
			__ctru_linear_heap_size = remaining - __ctru_heap_size;

			// However if the linear heap size is bigger than the cap, prefer to grow application heap
			if (__ctru_linear_heap_size > LINEAR_HEAP_SIZE_CAP) {
				__ctru_linear_heap_size = LINEAR_HEAP_SIZE_CAP;
				__ctru_heap_size = remaining - __ctru_linear_heap_size;
			}
		}
	} else if (__ctru_heap_size == 0) {
		__ctru_heap_size = remaining - __ctru_linear_heap_size;
	} else if (__ctru_linear_heap_size == 0) {
		__ctru_linear_heap_size = remaining - __ctru_heap_size;
	}

	// Allocate the application heap
	rc = svcControlMemory(&__ctru_heap, OS_HEAP_AREA_BEGIN, 0x0, __ctru_heap_size, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE);
	if (R_FAILED(rc))
		svcBreak(USERBREAK_PANIC);

	// Allocate the linear heap
	rc = svcControlMemory(&__ctru_linear_heap, 0x0, 0x0, __ctru_linear_heap_size, MEMOP_ALLOC_LINEAR, MEMPERM_READ | MEMPERM_WRITE);
	if (R_FAILED(rc))
		svcBreak(USERBREAK_PANIC);

	// Mappable allocator init
	mappableInit(OS_MAP_AREA_BEGIN, OS_MAP_AREA_END);

	// Set up newlib heap
	fake_heap_start = (char*)__ctru_heap;
	fake_heap_end = fake_heap_start + __ctru_heap_size;

}
