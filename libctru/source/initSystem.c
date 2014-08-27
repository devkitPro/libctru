#include <sys/iosupport.h>
#include <string.h>
#include <3ds.h>

// System globals we define here
int __system_argc;
char** __system_argv;
void (*__system_retAddr)(void);
u32 __linear_heap;

// Data from _prm structure
extern void* __service_ptr; // used to detect if we're run from a homebrew launcher
extern u32 __heap_size, __linear_heap_size;
extern const char* __system_arglist;

// newlib definitions we need
void __libc_init_array(void);
void __libc_fini_array(void);
extern char* fake_heap_start;
extern char* fake_heap_end;

static void initArgv();
static u32 heapBase;

void __attribute__((noreturn)) __ctru_exit(int rc)
{
	// Run the global destructors
	__libc_fini_array();

	// TODO: APT exit goes here

	// Unmap the linear heap
	svcControlMemory(&__linear_heap, __linear_heap, 0x0, __linear_heap_size, MEMOP_FREE, 0x0);

	// Unmap the application heap
	svcControlMemory(&heapBase, heapBase, 0x0, __heap_size, MEMOP_FREE, 0x0);

	// Jump to the loader if it provided a callback
	if (__system_retAddr)
		__system_retAddr();

	// Since above did not jump, end this process
	svcExitProcess();
}

void initSystem(void (*retAddr)(void))
{
	// Register newlib exit() syscall
	__syscalls.exit = __ctru_exit;
	__system_retAddr = __service_ptr ? retAddr : NULL;

	// Allocate the application heap
	heapBase = 0x08000000;
	svcControlMemory(&heapBase, heapBase, 0x0, __heap_size, MEMOP_ALLOC, 0x3);

	// Allocate the linear heap
	svcControlMemory(&__linear_heap, 0x0, 0x0, __linear_heap_size, MEMOP_ALLOC_LINEAR, 0x3);

	// Set up newlib heap
	fake_heap_start = (char*)heapBase;
	fake_heap_end = fake_heap_start + __heap_size;

	// Build argc/argv if present
	initArgv();

	// TODO: APT init goes here

	// Run the global constructors
	__libc_init_array();
}

void initArgv()
{
	int i;
	const char* temp = __system_arglist;

	// Check if the argument list is present
	if (!temp)
		return;

	// Retrieve argc
	__system_argc = *(u32*)temp;
	temp += sizeof(u32);

	// Find the end of the argument data
	for (i = 0; i < __system_argc; i ++)
	{
		for (; *temp; temp ++);
		temp ++;
	}

	// Reserve heap memory for argv data
	u32 argSize = temp - __system_arglist - sizeof(u32);
	__system_argv = (char**)fake_heap_start;
	fake_heap_start += sizeof(char**)*(__system_argc + 1);
	char* argCopy = fake_heap_start;
	fake_heap_start += argSize;

	// Fill argv array
	memcpy(argCopy, __system_arglist, argSize);
	temp = argCopy;
	for (i = 0; i < __system_argc; i ++)
	{
		__system_argv[i] = (char*)temp;
		for (; *temp; temp ++);
		temp ++;
	}
	__system_argv[__system_argc] = NULL;
}
