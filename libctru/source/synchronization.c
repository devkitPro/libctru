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

Handle __sync_get_arbiter(void)
{
	return arbiter;
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

static inline void LightEvent_SetState(LightEvent* event, int state)
{
	do
		__ldrex(&event->state);
	while (__strex(&event->state, state));
}

static inline int LightEvent_TryReset(LightEvent* event)
{
	do
	{
		if (__ldrex(&event->state))
		{
			__clrex();
			return 0;
		}
	} while (__strex(&event->state, -1));
	return 1;
}

void LightEvent_Init(LightEvent* event, ResetType reset_type)
{
	LightLock_Init(&event->lock);
	LightEvent_SetState(event, reset_type == RESET_STICKY ? -2 : -1);
}

void LightEvent_Clear(LightEvent* event)
{
	if (event->state == 1)
	{
		LightLock_Lock(&event->lock);
		LightEvent_SetState(event, -2);
		LightLock_Unlock(&event->lock);
	} else if (event->state == 0)
		LightEvent_SetState(event, -1);
}

void LightEvent_Pulse(LightEvent* event)
{
	if (event->state == -2)
		svcArbitrateAddress(arbiter, (u32)event, ARBITRATION_SIGNAL, -1, 0);
	else if (event->state == -1)
		svcArbitrateAddress(arbiter, (u32)event, ARBITRATION_SIGNAL, 1, 0);
	else
		LightEvent_Clear(event);
}

void LightEvent_Signal(LightEvent* event)
{
	if (event->state == -1)
	{
		LightEvent_SetState(event, 0);
		svcArbitrateAddress(arbiter, (u32)event, ARBITRATION_SIGNAL, 1, 0);
	} else if (event->state == -2)
	{
		LightLock_Lock(&event->lock);
		LightEvent_SetState(event, 1);
		svcArbitrateAddress(arbiter, (u32)event, ARBITRATION_SIGNAL, -1, 0);
		LightLock_Unlock(&event->lock);
	}
}

int LightEvent_TryWait(LightEvent* event)
{
	if (event->state == 1)
		return 1;
	return LightEvent_TryReset(event);
}

void LightEvent_Wait(LightEvent* event)
{
	for (;;)
	{
		if (event->state == -2)
		{
			svcArbitrateAddress(arbiter, (u32)event, ARBITRATION_WAIT_IF_LESS_THAN, 0, 0);
			return;
		}
		if (event->state != -1)
		{
			if (event->state == 1)
				return;
			if (event->state == 0 && LightEvent_TryReset(event))
				return;
		}
		svcArbitrateAddress(arbiter, (u32)event, ARBITRATION_WAIT_IF_LESS_THAN, 0, 0);
	}
}

void LightSemaphore_Init(LightSemaphore* semaphore, s16 initial_count, s16 max_count)
{
	semaphore->current_count = (s32)initial_count;
	semaphore->num_threads_acq = 0;
	semaphore->max_count = max_count;
}

void LightSemaphore_Acquire(LightSemaphore* semaphore, s32 count)
{
	s32 old_count;
	s16 num_threads_acq;

	do
	{
		for (;;)
		{
			old_count = __ldrex(&semaphore->current_count);
			if (old_count > 0)
				break;
			__clrex();

			do
				num_threads_acq = (s16)__ldrexh((u16 *)&semaphore->num_threads_acq);
			while (__strexh((u16 *)&semaphore->num_threads_acq, num_threads_acq + 1));

			svcArbitrateAddress(arbiter, (u32)semaphore, ARBITRATION_WAIT_IF_LESS_THAN, count, 0);

			do
				num_threads_acq = (s16)__ldrexh((u16 *)&semaphore->num_threads_acq);
			while (__strexh((u16 *)&semaphore->num_threads_acq, num_threads_acq - 1));
		}
	} while (__strex(&semaphore->current_count, old_count - count));
}

void LightSemaphore_Release(LightSemaphore* semaphore, s32 count)
{
	s32 old_count, new_count;
	do
	{
		old_count = __ldrex(&semaphore->current_count);
		new_count = old_count + count;
		if (new_count >= semaphore->max_count)
			new_count = semaphore->max_count;
	} while (__strex(&semaphore->current_count, new_count));

	if(old_count <= 0 || semaphore->num_threads_acq > 0)
		svcArbitrateAddress(arbiter, (u32)semaphore, ARBITRATION_SIGNAL, count, 0);
}
