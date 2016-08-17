/**
 * @file synchronization.h
 * @brief Provides synchronization locks.
 */
#pragma once
#include <sys/lock.h>

/// A light lock.
typedef _LOCK_T LightLock;

/// A recursive lock.
typedef _LOCK_RECURSIVE_T RecursiveLock;

/// A light event.
typedef struct
{
	s32 state;      ///< State of the event: -2=cleared sticky, -1=cleared oneshot, 0=signaled oneshot, 1=signaled sticky
	LightLock lock; ///< Lock used for sticky timer operation
} LightEvent;

/// Performs a Data Synchronization Barrier operation.
static inline void __dsb(void)
{
	__asm__ __volatile__("mcr p15, 0, %[val], c7, c10, 4" :: [val] "r" (0) : "memory");
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
 * @brief Retrieves the synchronization subsystem's address arbiter handle.
 * @return The synchronization subsystem's address arbiter handle.
 */
Handle __sync_get_arbiter(void);

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
