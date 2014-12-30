#include <sys/iosupport.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>

void (*__system_retAddr)(void);

// Data from _prm structure
extern void* __service_ptr; // used to detect if we're run from a homebrew launcher

void __system_allocateHeaps();
void __system_initArgv();
void __appInit();

// newlib definitions we need
void __libc_init_array(void);


void __ctru_exit(int rc);

void __attribute__((weak)) initSystem(void (*retAddr)(void))
{

	// Register newlib exit() syscall
	__syscalls.exit = __ctru_exit;
	__system_retAddr = __service_ptr ? retAddr : NULL;

	__system_allocateHeaps();

	// Build argc/argv if present
	__system_initArgv();

	__appInit();

	// Run the global constructors
	__libc_init_array();
}
