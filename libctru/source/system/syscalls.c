#include <sys/iosupport.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <string.h>

#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/env.h>
#include <3ds/os.h>
#include <3ds/synchronization.h>
#include "../internal.h"

void __ctru_exit(int rc);

extern const u8 __tdata_lma[];
extern const u8 __tdata_lma_end[];
extern u8 __tls_start[];

struct _reent* __SYSCALL(getreent)()
{
	ThreadVars* tv = getThreadVars();
	if (tv->magic != THREADVARS_MAGIC)
	{
		svcBreak(USERBREAK_PANIC);
		for (;;);
	}
	return tv->reent;
}

//---------------------------------------------------------------------------------
int __SYSCALL(gettod_r)(struct _reent *ptr, struct timeval *tp, struct timezone *tz) {
//---------------------------------------------------------------------------------
        if (tp != NULL) {
                // Retrieve current time, adjusting epoch from 1900 to 1970
                s64 now = osGetTime() - 2208988800000ULL;

                // Convert to struct timeval
                tp->tv_sec = now / 1000;
                tp->tv_usec = (now - 1000*tp->tv_sec) * 1000;
        }

        if (tz != NULL) {
                // Provide dummy information, as the 3DS does not have the concept of timezones
                tz->tz_minuteswest = 0;
                tz->tz_dsttime = 0;
        }

        return 0;
}

int __SYSCALL(nanosleep)(const struct timespec *req, struct timespec *rem)
{
	svcSleepThread(req->tv_sec * 1000000000ull + req->tv_nsec);
	return 0;
}

void __SYSCALL(lock_init) (_LOCK_T *lock)
{
	LightLock_Init(lock);
}

void __SYSCALL(lock_acquire) (_LOCK_T *lock)
{
	LightLock_Lock(lock);
}

int  __SYSCALL(lock_try_acquire) (_LOCK_T *lock)
{
	return LightLock_TryLock(lock);
}

void __SYSCALL(lock_release) (_LOCK_T *lock)
{
	LightLock_Unlock(lock);
}

void __SYSCALL(lock_init_recursive) (_LOCK_RECURSIVE_T *lock)
{
	RecursiveLock_Init(lock);
}

void __SYSCALL(lock_acquire_recursive) (_LOCK_RECURSIVE_T *lock)
{
	RecursiveLock_Lock(lock);
}

int  __SYSCALL(lock_try_acquire_recursive) (_LOCK_RECURSIVE_T *lock)
{
	return RecursiveLock_TryLock(lock);
}

void __SYSCALL(lock_release_recursive) (_LOCK_RECURSIVE_T *lock)
{
	RecursiveLock_Unlock(lock);
}

void __SYSCALL(exit)(int rc) {
	__ctru_exit(rc);
}


void __system_initSyscalls(void)
{

	// Initialize thread vars for the main thread
	ThreadVars* tv = getThreadVars();
	tv->magic = THREADVARS_MAGIC;
	tv->reent = _impure_ptr;
	tv->thread_ptr = NULL;
	tv->tls_tp = __tls_start-8; // ARM ELF TLS ABI mandates an 8-byte header
	tv->srv_blocking_policy = false;

	u32 tls_size = __tdata_lma_end - __tdata_lma;
	if (tls_size)
		memcpy(__tls_start, __tdata_lma, tls_size);
}
