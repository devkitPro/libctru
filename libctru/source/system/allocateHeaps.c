#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/env.h>
#include <3ds/os.h>

extern char* fake_heap_start;
extern char* fake_heap_end;

u32 __ctru_heap;
u32 __ctru_linear_heap;

__attribute__((weak)) u32 __ctru_heap_size        = 0;
__attribute__((weak)) u32 __ctru_linear_heap_size = 32*1024*1024;

void __attribute__((weak)) __system_allocateHeaps(void) {
	u32 tmp=0;

	if (!__ctru_heap_size) {
		// Automatically allocate all remaining free memory, aligning to page size.
		__ctru_heap_size = osGetMemRegionFree(MEMREGION_APPLICATION) &~ 0xFFF;
		if (__ctru_heap_size <= __ctru_linear_heap_size)
			svcBreak(USERBREAK_PANIC);
		__ctru_heap_size -= __ctru_linear_heap_size;
	}

	// Allocate the application heap
	__ctru_heap = 0x08000000;
	svcControlMemory(&tmp, __ctru_heap, 0x0, __ctru_heap_size, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE);

	// Allocate the linear heap
	svcControlMemory(&__ctru_linear_heap, 0x0, 0x0, __ctru_linear_heap_size, MEMOP_ALLOC_LINEAR, MEMPERM_READ | MEMPERM_WRITE);

	// Set up newlib heap
	fake_heap_start = (char*)__ctru_heap;
	fake_heap_end = fake_heap_start + __ctru_heap_size;

}
