#include <sys/iosupport.h>
#include <sys/time.h>

#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>

void (*__system_retAddr)(void);

// Data from _prm structure
extern void* __service_ptr; // used to detect if we're run from a homebrew launcher

void __system_allocateHeaps();
void __system_initArgv();
void __appInit();


void __ctru_exit(int rc);
int __libctru_gtod(struct _reent *ptr, struct timeval *tp, struct timezone *tz);

void __attribute__((weak)) __libctru_init(void (*retAddr)(void))
{

	// Register newlib exit() syscall
	__syscalls.exit = __ctru_exit;
    __syscalls.gettod_r = __libctru_gtod;

	__system_retAddr = __service_ptr ? retAddr : NULL;

	__system_allocateHeaps();

	// Build argc/argv if present
	__system_initArgv();

}
