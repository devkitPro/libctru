#include <sys/iosupport.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/env.h>
#include <3ds/os.h>
#include <3ds/synchronization.h>

#include "../internal.h"

void __ctru_exit(int rc);

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

int __SYSCALL(clock_gettime)(clockid_t clock_id, struct timespec *tp) {

	if (clock_id == CLOCK_REALTIME)
	{
		if (tp != NULL)
		{
			// Retrieve current time, adjusting epoch from 1900 to 1970
			s64 ms_since_epoch = osGetTime() - 2208988800000ULL;
			tp->tv_sec = ms_since_epoch / 1000;
			tp->tv_nsec = (ms_since_epoch % 1000) * 1000000;
		}
	}
	else if (clock_id == CLOCK_MONOTONIC)
	{
		if (tp != NULL)
		{
			// Use the ticks directly, as it offer the highest precision
			u64 ticks_since_boot = svcGetSystemTick();

			tp->tv_sec = ticks_since_boot / SYSCLOCK_ARM11;
			tp->tv_nsec = ((ticks_since_boot % SYSCLOCK_ARM11) * 1000000000ULL) / SYSCLOCK_ARM11;
		}
	}
	else
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int __SYSCALL(clock_getres)(clockid_t clock_id, struct timespec *res) {
	if (clock_id == CLOCK_REALTIME)
	{
		if (res != NULL)
		{
			res->tv_sec = 0;
			res->tv_nsec = 1000000;
		}
	}
	else if (clock_id == CLOCK_MONOTONIC)
	{
		if (res != NULL)
		{
			res->tv_sec = 0;
			res->tv_nsec = 1;
		}
	}
	else
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}

//---------------------------------------------------------------------------------
int __SYSCALL(gettod_r)(struct _reent *ptr, struct timeval *tp, struct timezone *tz) {
//---------------------------------------------------------------------------------
        if (tp != NULL) {
                // Retrieve current time, adjusting epoch from 1900 to 1970
                s64 now = osGetTime() - 2208988800000ULL;

                // Convert to struct timeval
                tp->tv_sec = now / 1000;
                tp->tv_usec = (now % 1000) * 1000;
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

int  __SYSCALL(cond_signal)(_COND_T *cond)
{
	CondVar_Signal((CondVar*)cond);
	return 0;
}

int  __SYSCALL(cond_broadcast)(_COND_T *cond)
{
	CondVar_Broadcast((CondVar*)cond);
	return 0;
}

int  __SYSCALL(cond_wait)(_COND_T *cond, _LOCK_T *lock, uint64_t timeout_ns)
{
	return CondVar_WaitTimeout((CondVar*)cond, lock, timeout_ns) ? ETIMEDOUT : 0;
}

int  __SYSCALL(cond_wait_recursive)(_COND_T *cond, _LOCK_RECURSIVE_T *lock, uint64_t timeout_ns)
{
	uint32_t thread_tag_backup = 0;
	if (lock->counter != 1)
		return EBADF;

	thread_tag_backup = lock->thread_tag;
	lock->thread_tag = 0;
	lock->counter = 0;

	int err = CondVar_WaitTimeout((CondVar*)cond, &lock->lock, timeout_ns);

	lock->thread_tag = thread_tag_backup;
	lock->counter = 1;

	return err ? ETIMEDOUT : 0;
}

int  __SYSCALL(thread_create)(struct __pthread_t **thread, void* (*func)(void*), void *arg, void *stack_addr, size_t stack_size)
{
	if (stack_addr) {
		return EINVAL;
	}

	if (!stack_size) {
		stack_size = 32*1024;
	}

	Thread t = threadCreate((ThreadFunc)func, arg, stack_size, 0x3F, 0, false);
	if (t) {
		*thread = (struct __pthread_t*)t;
		return 0;
	}

	return ENOMEM;
}

void*__SYSCALL(thread_join)(struct __pthread_t *thread)
{
	threadJoin((Thread)thread, U64_MAX);
	void* rc = (void*)threadGetExitCode((Thread)thread);
	threadFree((Thread)thread);
	return rc;
}

int  __SYSCALL(thread_detach)(struct __pthread_t *thread)
{
	threadDetach((Thread)thread);
	return 0;
}

void __SYSCALL(thread_exit)(void *value)
{
	threadExit((int)value);
}

struct __pthread_t *__SYSCALL(thread_self)(void)
{
	return (struct __pthread_t*)threadGetCurrent();
}

void initThreadVars(struct Thread_tag *thread)
{
	ThreadVars* tv = getThreadVars();
	tv->magic = THREADVARS_MAGIC;
	tv->reent = thread != NULL ? &thread->reent : _impure_ptr;
	tv->thread_ptr = thread;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	tv->tls_tp = (thread != NULL ? (u8*)thread->stacktop : __tls_start) - 8; // Arm ELF TLS ABI mandates an 8-byte header
#pragma GCC diagnostic pop
	tv->srv_blocking_policy = false;

	// Kernel does not initialize fpscr at all, so we must do it ourselves
	// https://developer.arm.com/documentation/ddi0360/f/vfp-programmers-model/vfp11-system-registers/floating-point-status-and-control-register--fpscr

	// All flags clear, all interrupts disabled, all instruction scalar.
	// As for the 3 below fields: default NaN mode, flush-to-zero both enabled & round to nearest.
	__builtin_arm_set_fpscr(BIT(25) | BIT(24) | (0u << 22));
}

void __system_initSyscalls(void)
{
	// Initialize thread vars for the main thread
	initThreadVars(NULL);
	u32 tls_size = __tdata_lma_end - __tdata_lma;
	size_t tdata_start = alignTo((size_t)__tls_start, __tdata_align);
	if (tls_size)
		memcpy((void*)tdata_start, __tdata_lma, tls_size);
}
