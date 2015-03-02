#include <3ds/types.h>
#include <3ds/svc.h>

extern u32 __linear_heap;
extern u32 __heapBase;
extern u32 __heap_size, __linear_heap_size;
extern void (*__system_retAddr)(void);

void __destroy_handle_list(void);
void __appExit();

void __libc_fini_array(void);

void __attribute__((weak)) __attribute__((noreturn)) __libctru_exit(int rc)
{
	u32 tmp=0;

	// Unmap the linear heap
	svcControlMemory(&tmp, __linear_heap, 0x0, __linear_heap_size, MEMOP_FREE, 0x0);

	// Unmap the application heap
	svcControlMemory(&tmp, __heapBase, 0x0, __heap_size, MEMOP_FREE, 0x0);

	// Close some handles
	__destroy_handle_list();

	// Jump to the loader if it provided a callback
	if (__system_retAddr)
		__system_retAddr();

	// Since above did not jump, end this process
	svcExitProcess();
}
