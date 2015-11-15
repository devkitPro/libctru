#include <sys/iosupport.h>
#include <sys/time.h>
#include <string.h>

#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/env.h>

void (*__system_retAddr)(void);

void __system_allocateHeaps();
void __system_initArgv();
void __appInit();


void __ctru_exit(int rc);
int __libctru_gtod(struct _reent *ptr, struct timeval *tp, struct timezone *tz);

Result __sync_init(void) __attribute__((weak));

void __attribute__((weak)) __libctru_init(void (*retAddr)(void))
{

	// Register newlib exit() syscall
	__syscalls.exit = __ctru_exit;
    __syscalls.gettod_r = __libctru_gtod;

	__system_retAddr = envIsHomebrew() ? retAddr : NULL;

	if (__sync_init)
		__sync_init();

	__system_allocateHeaps();

	// Build argc/argv if present
	__system_initArgv();

}
