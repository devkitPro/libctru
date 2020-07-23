#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/result.h>
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

Result syncArbitrateAddress(s32* addr, ArbitrationType type, s32 value)
{
	return svcArbitrateAddressNoTimeout(arbiter, (u32)addr, type, value);
}

Result syncArbitrateAddressWithTimeout(s32* addr, ArbitrationType type, s32 value, s64 timeout_ns)
{
	return svcArbitrateAddress(arbiter, (u32)addr, type, value, timeout_ns);
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
	bool bAlreadyLocked;

	// Try to lock, or if that's not possible, increment the number of waiting threads
	do
	{
		// Read the current lock state
		val = __ldrex(lock);
		if (val == 0) val = 1; // 0 is an invalid state - treat it as 1 (unlocked)
		bAlreadyLocked = val < 0;

		// Calculate the desired next state of the lock
		if (!bAlreadyLocked)
			val = -val; // transition into locked state
		else
			--val; // increment the number of waiting threads (which has the sign reversed during locked state)
	} while (__strex(lock, val));

	// While the lock is held by a different thread:
	while (bAlreadyLocked)
	{
		// Wait for the lock holder thread to wake us up
		syncArbitrateAddress(lock, ARBITRATION_WAIT_IF_LESS_THAN, 0);

		// Try to lock again
		do
		{
			// Read the current lock state
			val = __ldrex(lock);
			bAlreadyLocked = val < 0;

			// Calculate the desired next state of the lock
			if (!bAlreadyLocked)
				val = -(val-1); // decrement the number of waiting threads *and* transition into locked state
			else
			{
				// Since the lock is still held, we need to cancel the atomic update and wait again
				__clrex();
				break;
			}
		} while (__strex(lock, val));
	}

	__dmb();
}

int LightLock_TryLock(LightLock* lock)
{
	s32 val;
	do
	{
		val = __ldrex(lock);
		if (val == 0) val = 1; // 0 is an invalid state - treat it as 1 (unlocked)
		if (val < 0)
		{
			__clrex();
			return 1; // Failure
		}
	} while (__strex(lock, -val));

	__dmb();
	return 0; // Success
}

void LightLock_Unlock(LightLock* lock)
{
	__dmb();

	s32 val;
	do
		val = -__ldrex(lock);
	while (__strex(lock, val));

	if (val > 1)
		// Wake up exactly one thread
		syncArbitrateAddress(lock, ARBITRATION_SIGNAL, 1);
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

static inline void CondVar_BeginWait(CondVar* cv, LightLock* lock)
{
	s32 val;
	do
		val = __ldrex(cv) - 1;
	while (__strex(cv, val));
	LightLock_Unlock(lock);
}

static inline bool CondVar_EndWait(CondVar* cv, s32 num_threads)
{
	bool hasWaiters;
	s32 val;

	do {
		val = __ldrex(cv);
		hasWaiters = val < 0;
		if (hasWaiters)
		{
			if (num_threads < 0)
				val = 0;
			else if (val <= -num_threads)
				val += num_threads;
			else
				val = 0;
		}
	} while (__strex(cv, val));

	return hasWaiters;
}

void CondVar_Init(CondVar* cv)
{
	*cv = 0;
}

void CondVar_Wait(CondVar* cv, LightLock* lock)
{
	CondVar_BeginWait(cv, lock);
	syncArbitrateAddress(cv, ARBITRATION_WAIT_IF_LESS_THAN, 0);
	LightLock_Lock(lock);
}

int CondVar_WaitTimeout(CondVar* cv, LightLock* lock, s64 timeout_ns)
{
	CondVar_BeginWait(cv, lock);

	bool timedOut = false;
	Result rc = syncArbitrateAddressWithTimeout(cv, ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT, 0, timeout_ns);
	if (R_DESCRIPTION(rc) == RD_TIMEOUT)
	{
		timedOut = CondVar_EndWait(cv, 1);
		__dmb();
	}

	LightLock_Lock(lock);
	return timedOut;
}

void CondVar_WakeUp(CondVar* cv, s32 num_threads)
{
	__dmb();
	if (CondVar_EndWait(cv, num_threads))
		syncArbitrateAddress(cv, ARBITRATION_SIGNAL, num_threads);
	else
		__dmb();
}

// LightEvent state
enum
{
	CLEARED_STICKY = -2,
	CLEARED_ONESHOT = -1,
	SIGNALED_ONESHOT = 0,
	SIGNALED_STICKY = 1
};

static inline void LightEvent_SetState(LightEvent* event, int state)
{
	do
		__ldrex(&event->state);
	while (__strex(&event->state, state));
}

static inline int LightEvent_TryReset(LightEvent* event)
{
	__dmb();
	do
	{
		if (__ldrex(&event->state))
		{
			__clrex();
			return 0;
		}
	} while (__strex(&event->state, CLEARED_ONESHOT));
	__dmb();
	return 1;
}

void LightEvent_Init(LightEvent* event, ResetType reset_type)
{
	LightLock_Init(&event->lock);
	LightEvent_SetState(event, reset_type == RESET_STICKY ? CLEARED_STICKY : CLEARED_ONESHOT);
}

void LightEvent_Clear(LightEvent* event)
{
	if (event->state == SIGNALED_STICKY)
	{
		LightLock_Lock(&event->lock);
		LightEvent_SetState(event, CLEARED_STICKY);
		LightLock_Unlock(&event->lock);
	} else if (event->state == SIGNALED_ONESHOT)
	{
		__dmb();
		LightEvent_SetState(event, CLEARED_ONESHOT);
		__dmb();
	}
}

void LightEvent_Pulse(LightEvent* event)
{
	if (event->state == CLEARED_STICKY)
		syncArbitrateAddress(&event->state, ARBITRATION_SIGNAL, -1);
	else if (event->state == CLEARED_ONESHOT)
		syncArbitrateAddress(&event->state, ARBITRATION_SIGNAL, 1);
	else
		LightEvent_Clear(event);
}

void LightEvent_Signal(LightEvent* event)
{
	if (event->state == CLEARED_ONESHOT)
	{
		__dmb();
		LightEvent_SetState(event, SIGNALED_ONESHOT);
		syncArbitrateAddress(&event->state, ARBITRATION_SIGNAL, 1);
	} else if (event->state == CLEARED_STICKY)
	{
		LightLock_Lock(&event->lock);
		LightEvent_SetState(event, SIGNALED_STICKY);
		syncArbitrateAddress(&event->state, ARBITRATION_SIGNAL, -1);
		LightLock_Unlock(&event->lock);
	}
}

int LightEvent_TryWait(LightEvent* event)
{
	if (event->state == SIGNALED_STICKY)
		return 1;
	return LightEvent_TryReset(event);
}

void LightEvent_Wait(LightEvent* event)
{
	for (;;)
	{
		if (event->state == CLEARED_STICKY)
		{
			syncArbitrateAddress(&event->state, ARBITRATION_WAIT_IF_LESS_THAN, SIGNALED_ONESHOT);
			return;
		}
		if (event->state != CLEARED_ONESHOT)
		{
			if (event->state == SIGNALED_STICKY)
				return;
			if (event->state == SIGNALED_ONESHOT && LightEvent_TryReset(event))
				return;
		}
		syncArbitrateAddress(&event->state, ARBITRATION_WAIT_IF_LESS_THAN, SIGNALED_ONESHOT);
	}
}

int LightEvent_WaitTimeout(LightEvent* event, s64 timeout_ns)
{
	Result  timeoutRes = 0x09401BFE;
	Result  res = 0;

	while (res != timeoutRes)
	{
		if (event->state == CLEARED_STICKY)
		{
			res = syncArbitrateAddressWithTimeout(&event->state, ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT, SIGNALED_ONESHOT, timeout_ns);
			return res == timeoutRes;
		}

		if (event->state != CLEARED_ONESHOT)
		{
			if (event->state == SIGNALED_STICKY)
				return 0;

			if (event->state == SIGNALED_ONESHOT && LightEvent_TryReset(event))
				return 0;
		}

		res = syncArbitrateAddressWithTimeout(&event->state, ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT, SIGNALED_ONESHOT, timeout_ns);
	}

	return res == timeoutRes;
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
			if (old_count >= count)
				break;
			__clrex();

			do
				num_threads_acq = (s16)__ldrexh((u16 *)&semaphore->num_threads_acq);
			while (__strexh((u16 *)&semaphore->num_threads_acq, num_threads_acq + 1));

			syncArbitrateAddress(&semaphore->current_count, ARBITRATION_WAIT_IF_LESS_THAN, count);

			do
				num_threads_acq = (s16)__ldrexh((u16 *)&semaphore->num_threads_acq);
			while (__strexh((u16 *)&semaphore->num_threads_acq, num_threads_acq - 1));
		}
	} while (__strex(&semaphore->current_count, old_count - count));

	__dmb();
}

int LightSemaphore_TryAcquire(LightSemaphore* semaphore, s32 count)
{
	s32 old_count;
	do
	{
		old_count = __ldrex(&semaphore->current_count);
		if (old_count < count)
		{
			__clrex();
			return 1; // failure
		}
	} while (__strex(&semaphore->current_count, old_count - count));

	__dmb();
	return 0; // success
}

void LightSemaphore_Release(LightSemaphore* semaphore, s32 count)
{
	__dmb();

	s32 old_count, new_count;
	do
	{
		old_count = __ldrex(&semaphore->current_count);
		new_count = old_count + count;
		if (new_count >= semaphore->max_count)
			new_count = semaphore->max_count;
	} while (__strex(&semaphore->current_count, new_count));

	if(old_count <= 0 || semaphore->num_threads_acq > 0)
		syncArbitrateAddress(&semaphore->current_count, ARBITRATION_SIGNAL, count);
}
