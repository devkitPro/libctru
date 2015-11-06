#include <3ds/types.h>
#include <3ds/svc.h>

extern char* fake_heap_start;
extern char* fake_heap_end;
u32 __linear_heap;
u32 __heapBase;
extern u32 __heap_size, __linear_heap_size;

u32 __attribute__((weak)) __stacksize__ = 0x8000;
u32 __stack_bottom;
u32 __allocated_stack_size;

void __system_allocateStack() {
	u32 tmp=0;
	MemInfo memInfo;
	PageInfo pageInfo;

	register u32 sp_val __asm__("sp");
	svcQueryMemory(&memInfo, &pageInfo, sp_val);

	__stacksize__ += 0xFFF;
	__stacksize__ &= ~0xFFF;
	__allocated_stack_size = __stacksize__ > memInfo.size ? __stacksize__ - memInfo.size: 0;
	__stack_bottom = memInfo.base_addr - __allocated_stack_size;

	if (__allocated_stack_size)
		svcControlMemory(&tmp, __stack_bottom, 0x0, __allocated_stack_size, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE);
}

void __attribute__((weak)) __system_allocateHeaps() {
	u32 tmp=0;

	// Allocate the application heap
	__heapBase = 0x08000000;
	svcControlMemory(&tmp, __heapBase, 0x0, __heap_size - __allocated_stack_size, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE);

	// Allocate the linear heap
	svcControlMemory(&__linear_heap, 0x0, 0x0, __linear_heap_size, MEMOP_ALLOC_LINEAR, MEMPERM_READ | MEMPERM_WRITE);
	// Set up newlib heap
	fake_heap_start = (char*)__heapBase;
	fake_heap_end = fake_heap_start + __heap_size - __allocated_stack_size;

}