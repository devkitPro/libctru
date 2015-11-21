#include <sys/iosupport.h>
#include <sys/time.h>
#include <string.h>

#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/env.h>

void (*__system_retAddr)(void);

void __system_initSyscalls(void);
void __system_allocateHeaps(void);
void __system_initArgv(void);
void __appInit(void);

Result __sync_init(void);

void __attribute__((weak)) __libctru_init(void (*retAddr)(void))
{
	// Store the return address
	__system_retAddr = envIsHomebrew() ? retAddr : NULL;

	// Initialize the synchronization subsystem
	__sync_init();

	// Initialize newlib support system calls
	__system_initSyscalls();

	// Allocate application and linear heaps
	__system_allocateHeaps();

	// Build argc/argv if present
	__system_initArgv();
}
