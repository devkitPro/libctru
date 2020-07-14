/**
 * @file synchronization.h
 * @brief Provides synchronization locks.
 */
#pragma once
#include <sys/lock.h>
#include <3ds/svc.h>

/// A light lock.
typedef _LOCK_T LightLock;

/// A recursive lock.
typedef _LOCK_RECURSIVE_T RecursiveLock;

/// A condition variable.
typedef s32 CondVar;

/// A light event.
typedef struct
{
	s32 state;      ///< State of the event: -2=cleared sticky, -1=cleared oneshot, 0=signaled oneshot, 1=signaled sticky
	LightLock lock; ///< Lock used for sticky timer operation
} LightEvent;

/// A light semaphore.
typedef struct
{
	s32 current_count;      ///< The current release count of the semaphore
	s16 num_threads_acq;    ///< Number of threads concurrently acquiring the semaphore
	s16 max_count;          ///< The maximum release count of the semaphore
} LightSemaphore;

/// Performs a Data Synchronization Barrier operation.
static inline void __dsb(void)
{
	__asm__ __volatile__("mcr p15, 0, %[val], c7, c10, 4" :: [val] "r" (0) : "memory");
}

/// Performs a Data Memory Barrier operation.
static inline void __dmb(void)
{
	__asm__ __volatile__("mcr p15, 0, %[val], c7, c10, 5" :: [val] "r" (0) : "memory");
}

/// Performs a clrex operation.
static inline void __clrex(void)
{
	__asm__ __volatile__("clrex" ::: "memory");
}

/**
 * @brief Performs a ldrex operation.
 * @param addr Address to perform the operation on.
 * @return The resulting value.
 */
static inline s32 __ldrex(s32* addr)
{
	s32 val;
	__asm__ __volatile__("ldrex %[val], %[addr]" : [val] "=r" (val) : [addr] "Q" (*addr));
	return val;
}

/**
 * @brief Performs a strex operation.
 * @param addr Address to perform the operation on.
 * @param val Value to store.
 * @return Whether the operation was successful.
 */
static inline bool __strex(s32* addr, s32 val)
{
	bool res;
	__asm__ __volatile__("strex %[res], %[val], %[addr]" : [res] "=&r" (res) : [val] "r" (val), [addr] "Q" (*addr));
	return res;
}

/**
 * @brief Performs a ldrexh operation.
 * @param addr Address to perform the operation on.
 * @return The resulting value.
 */
static inline u16 __ldrexh(u16* addr)
{
	u16 val;
	__asm__ __volatile__("ldrexh %[val], %[addr]" : [val] "=r" (val) : [addr] "Q" (*addr));
	return val;
}

/**
 * @brief Performs a strexh operation.
 * @param addr Address to perform the operation on.
 * @param val Value to store.
 * @return Whether the operation was successful.
 */
static inline bool __strexh(u16* addr, u16 val)
{
	bool res;
	__asm__ __volatile__("strexh %[res], %[val], %[addr]" : [res] "=&r" (res) : [val] "r" (val), [addr] "Q" (*addr));
	return res;
}

/**
 * @brief Performs a ldrexb operation.
 * @param addr Address to perform the operation on.
 * @return The resulting value.
 */
static inline u8 __ldrexb(u8* addr)
{
	u8 val;
	__asm__ __volatile__("ldrexb %[val], %[addr]" : [val] "=r" (val) : [addr] "Q" (*addr));
	return val;
}

/**
 * @brief Performs a strexb operation.
 * @param addr Address to perform the operation on.
 * @param val Value to store.
 * @return Whether the operation was successful.
 */
static inline bool __strexb(u8* addr, u8 val)
{
	bool res;
	__asm__ __volatile__("strexb %[res], %[val], %[addr]" : [res] "=&r" (res) : [val] "r" (val), [addr] "Q" (*addr));
	return res;
}

/// Performs an atomic pre-increment operation.
#define AtomicIncrement(ptr) __atomic_add_fetch((u32*)(ptr), 1, __ATOMIC_SEQ_CST)
/// Performs an atomic pre-decrement operation.
#define AtomicDecrement(ptr) __atomic_sub_fetch((u32*)(ptr), 1, __ATOMIC_SEQ_CST)
/// Performs an atomic post-increment operation.
#define AtomicPostIncrement(ptr) __atomic_fetch_add((u32*)(ptr), 1, __ATOMIC_SEQ_CST)
/// Performs an atomic post-decrement operation.
#define AtomicPostDecrement(ptr) __atomic_fetch_sub((u32*)(ptr), 1, __ATOMIC_SEQ_CST)
/// Performs an atomic swap operation.
#define AtomicSwap(ptr, value) __atomic_exchange_n((u32*)(ptr), (value), __ATOMIC_SEQ_CST)

/**
 * @brief Function used to implement user-mode synchronization primitives.
 * @param addr Pointer to a signed 32-bit value whose address will be used to identify waiting threads.
 * @param type Type of action to be performed by the arbiter
 * @param value Number of threads to signal if using @ref ARBITRATION_SIGNAL, or the value used for comparison.
 *
 * This will perform an arbitration based on #type. The comparisons are done between #value and the value at the address #addr.
 *
 * @code
 * s32 val=0;
 * // Does *nothing* since val >= 0
 * syncArbitrateAddress(&val,ARBITRATION_WAIT_IF_LESS_THAN,0);
 * @endcode
 *
 * @note Usage of this function entails an implicit Data Memory Barrier (dmb).
 */
Result syncArbitrateAddress(s32* addr, ArbitrationType type, s32 value);

/**
 * @brief Function used to implement user-mode synchronization primitives (with timeout).
 * @param addr Pointer to a signed 32-bit value whose address will be used to identify waiting threads.
 * @param type Type of action to be performed by the arbiter (must use \ref ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT or \ref ARBITRATION_DECREMENT_AND_WAIT_IF_LESS_THAN_TIMEOUT)
 * @param value Number of threads to signal if using @ref ARBITRATION_SIGNAL, or the value used for comparison.
 *
 * This will perform an arbitration based on #type. The comparisons are done between #value and the value at the address #addr.
 *
 * @code
 * s32 val=0;
 * // Thread will wait for a signal or wake up after 10000000 nanoseconds because val < 1.
 * syncArbitrateAddressWithTimeout(&val,ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT,1,10000000LL);
 * @endcode
 *
 * @note Usage of this function entails an implicit Data Memory Barrier (dmb).
 */
Result syncArbitrateAddressWithTimeout(s32* addr, ArbitrationType type, s32 value, s64 timeout_ns);

/**
 * @brief Initializes a light lock.
 * @param lock Pointer to the lock.
 */
void LightLock_Init(LightLock* lock);

/**
 * @brief Locks a light lock.
 * @param lock Pointer to the lock.
 */
void LightLock_Lock(LightLock* lock);

/**
 * @brief Attempts to lock a light lock.
 * @param lock Pointer to the lock.
 * @return Zero on success, non-zero on failure.
 */
int LightLock_TryLock(LightLock* lock);

/**
 * @brief Unlocks a light lock.
 * @param lock Pointer to the lock.
 */
void LightLock_Unlock(LightLock* lock);

/**
 * @brief Initializes a recursive lock.
 * @param lock Pointer to the lock.
 */
void RecursiveLock_Init(RecursiveLock* lock);

/**
 * @brief Locks a recursive lock.
 * @param lock Pointer to the lock.
 */
void RecursiveLock_Lock(RecursiveLock* lock);

/**
 * @brief Attempts to lock a recursive lock.
 * @param lock Pointer to the lock.
 * @return Zero on success, non-zero on failure.
 */
int RecursiveLock_TryLock(RecursiveLock* lock);

/**
 * @brief Unlocks a recursive lock.
 * @param lock Pointer to the lock.
 */
void RecursiveLock_Unlock(RecursiveLock* lock);

/**
 * @brief Initializes a condition variable.
 * @param cv Pointer to the condition variable.
 */
void CondVar_Init(CondVar* cv);

/**
 * @brief Waits on a condition variable.
 * @param cv Pointer to the condition variable.
 * @param lock Pointer to the lock to atomically unlock/relock during the wait.
 */
void CondVar_Wait(CondVar* cv, LightLock* lock);

/**
 * @brief Waits on a condition variable with a timeout.
 * @param cv Pointer to the condition variable.
 * @param lock Pointer to the lock to atomically unlock/relock during the wait.
 * @param timeout_ns Timeout in nanoseconds.
 * @return Zero on success, non-zero on failure.
 */
int CondVar_WaitTimeout(CondVar* cv, LightLock* lock, s64 timeout_ns);

/**
 * @brief Wakes up threads waiting on a condition variable.
 * @param cv Pointer to the condition variable.
 * @param num_threads Maximum number of threads to wake up (or \ref ARBITRATION_SIGNAL_ALL to wake them all).
 */
void CondVar_WakeUp(CondVar* cv, s32 num_threads);

/**
 * @brief Wakes up a single thread waiting on a condition variable.
 * @param cv Pointer to the condition variable.
 */
static inline void CondVar_Signal(CondVar* cv)
{
	CondVar_WakeUp(cv, 1);
}

/**
 * @brief Wakes up all threads waiting on a condition variable.
 * @param cv Pointer to the condition variable.
 */
static inline void CondVar_Broadcast(CondVar* cv)
{
	CondVar_WakeUp(cv, ARBITRATION_SIGNAL_ALL);
}

/**
 * @brief Initializes a light event.
 * @param event Pointer to the event.
 * @param reset_type Type of reset the event uses (RESET_ONESHOT/RESET_STICKY).
 */
void LightEvent_Init(LightEvent* event, ResetType reset_type);

/**
 * @brief Clears a light event.
 * @param event Pointer to the event.
 */
void LightEvent_Clear(LightEvent* event);

/**
 * @brief Wakes up threads waiting on a sticky light event without signaling it. If the event had been signaled before, it is cleared instead.
 * @param event Pointer to the event.
 */
void LightEvent_Pulse(LightEvent* event);

/**
 * @brief Signals a light event, waking up threads waiting on it.
 * @param event Pointer to the event.
 */
void LightEvent_Signal(LightEvent* event);

/**
 * @brief Attempts to wait on a light event.
 * @param event Pointer to the event.
 * @return Non-zero if the event was signaled, zero otherwise.
 */
int LightEvent_TryWait(LightEvent* event);

/**
 * @brief Waits on a light event.
 * @param event Pointer to the event.
 */
void LightEvent_Wait(LightEvent* event);

/**
 * @brief Waits on a light event until either the event is signaled or the timeout is reached.
 * @param event Pointer to the event.
 * @param timeout_ns Timeout in nanoseconds.
 * @return Non-zero on timeout, zero otherwise.
 */
int LightEvent_WaitTimeout(LightEvent* event, s64 timeout_ns);

/**
 * @brief Initializes a light semaphore.
 * @param event Pointer to the semaphore.
 * @param max_count Initial count of the semaphore.
 * @param max_count Maximum count of the semaphore.
 */
void LightSemaphore_Init(LightSemaphore* semaphore, s16 initial_count, s16 max_count);

/**
 * @brief Acquires a light semaphore.
 * @param semaphore Pointer to the semaphore.
 * @param count Acquire count
 */
void LightSemaphore_Acquire(LightSemaphore* semaphore, s32 count);

/**
 * @brief Attempts to acquire a light semaphore.
 * @param semaphore Pointer to the semaphore.
 * @param count Acquire count
 * @return Zero on success, non-zero on failure
 */
int LightSemaphore_TryAcquire(LightSemaphore* semaphore, s32 count);

/**
 * @brief Releases a light semaphore.
 * @param semaphore Pointer to the semaphore.
 * @param count Release count
 */
void LightSemaphore_Release(LightSemaphore* semaphore, s32 count);
