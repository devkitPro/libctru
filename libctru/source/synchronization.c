#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>

static Handle arbiter;

Result __sync_init(void)
{
	return svcCreateAddressArbiter(&arbiter);
}

void __sync_fini(void)
{
	if (arbiter)
		svcCloseHandle(arbiter);
}

void LightLock_Init(LightLock* lock)
{
	do
		__ldrex(lock);
	while (__strex(lock, 1));
}

void LightLock_Lock(LightLock* lock)
{
	s32 val;
_begin:
	do
	{
		val = __ldrex(lock);
		if (val < 0)
		{
			// Add ourselves to the list of threads blocked on this lock
			if (__strex(lock, val-1))
				goto _begin; // strex failed, try to lock again

		_wait:
			// Wait for a thread to wake us up
			svcArbitrateAddress(arbiter, (u32)lock, ARBITRATION_WAIT_IF_LESS_THAN, 0, 0);

			// Try to lock again
			do
			{
				val = __ldrex(lock);
				if (val < 0)
				{
					// Lock is still locked - keep waiting
					__clrex();
					goto _wait;
				}
			} while (__strex(lock, -(val-1)));
			return;
		}
	} while (__strex(lock, -val));
}

int LightLock_TryLock(LightLock* lock)
{
	s32 val;
	do
	{
		val = __ldrex(lock);
		if (val < 0)
		{
			__clrex();
			return 1; // Failure
		}
	} while (__strex(lock, -val));
	return 0; // Success
}

void LightLock_Unlock(LightLock* lock)
{
	s32 val;
	do
		val = -__ldrex(lock);
	while (__strex(lock, val));

	if (val > 1)
		// Wake up exactly one thread
		svcArbitrateAddress(arbiter, (u32)lock, ARBITRATION_SIGNAL, 1, 0);
}

void RecursiveLock_Init(RecursiveLock* lock)
{
	LightLock_Init(&lock->lock);
	lock->thread_tag = 0;
	lock->counter = 0;
}

void RecursiveLock_Lock(RecursiveLock* lock)
{
	u32 tag = (u32)getThreadLocalStorage();
	if (lock->thread_tag != tag)
	{
		LightLock_Lock(&lock->lock);
		lock->thread_tag = tag;
	}
	lock->counter ++;
}

int RecursiveLock_TryLock(RecursiveLock* lock)
{
	u32 tag = (u32)getThreadLocalStorage();
	if (lock->thread_tag != tag)
	{
		if (LightLock_TryLock(&lock->lock))
			return 1; // Failure
		lock->thread_tag = tag;
	}
	lock->counter ++;
	return 0; // Success
}

void RecursiveLock_Unlock(RecursiveLock* lock)
{
	if (!--lock->counter)
	{
		lock->thread_tag = 0;
		LightLock_Unlock(&lock->lock);
	}
}
