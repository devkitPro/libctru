/**
 * @file ndm.h
 * @brief NDMU service. https://3dbrew.org/wiki/NDM_Services
 */
#pragma once

typedef enum {
	EXCLUSIVE_STATE_NONE = 0,
	EXCLUSIVE_STATE_INFRASTRUCTURE = 1,
	EXCLUSIVE_STATE_LOCAL_COMMUNICATIONS = 2,
	EXCLUSIVE_STATE_STREETPASS = 3,
	EXCLUSIVE_STATE_STREETPASS_DATA = 4,
} NDM_ExclusiveState;

typedef enum {
	STATE_INITIAL = 0,
	STATE_SUSPENDED = 1,
	STATE_INFRASTRUCTURE_CONNECTING = 2,
	STATE_INFRASTRUCTURE_CONNECTED = 3,
	STATE_INFRASTRUCTURE_WORKING = 4,
	STATE_INFRASTRUCTURE_SUSPENDING = 5,
	STATE_INFRASTRUCTURE_FORCE_SUSPENDING = 6,
	STATE_INFRASTRUCTURE_DISCONNECTING = 7,
	STATE_INFRASTRUCTURE_FORCE_DISCONNECTING = 8,
	STATE_CEC_WORKING = 9,
	STATE_CEC_FORCE_SUSPENDING = 10,
	STATE_CEC_SUSPENDING = 11,
} NDM_State;

/// Initializes ndmu.
Result ndmuInit(void);

/// Exits ndmu.
void ndmuExit(void);

/**
 * @brief Enter an exclusive ndm state.
 * @param state State specified in the NDM_ExclusiveState enumerator.
 */
Result NDMU_EnterExclusiveState(NDM_ExclusiveState state);

///  Leaves the ndm exclusive state.
Result NDMU_LeaveExclusiveState(void);

/**
 * @brief Returns the exclsuive ndm state.
 * @param state Pointer to write the exclsuive ndm state to.
 */
Result NDMU_GetExclusiveState(NDM_ExclusiveState *state);

///  Locks the ndm state.
Result NDMU_LockState(void);

///  Unlocks the ndm state.
Result NDMU_UnlockState(void);

/**
 * @brief Suspends the ndm scheduler.
 * @param flag 0 = Wait for completion, 1 = Perform in background.
 */
Result NDMU_SuspendScheduler(u32 flag);

///  Resumes the ndm scheduler.
Result NDMU_ResumeScheduler(void);

/**
 * @brief Returns the current ndm state.
 * @param state Pointer to write the current NDM state to.
 */
Result NDMU_GetCurrentState(NDM_State *state);

/**
 * @brief Sets the scan interval.
 * @param interval Value to set the scan interval to.
 */
Result NDMU_SetScanInterval(u32 interval);

/**
 * @brief Returns the scan interval.
 * @param interval Pointer to write the interval value to.
 */
Result NDMU_GetScanInterval(u32 *interval);

/**
 * @brief Returns the retry interval.
 * @param interval Pointer to write the interval value to.
 */
Result NDMU_GetRetryInterval(u32 *interval);

/// Reset daemons to default daemon bit mask values.
Result NDMU_ResetDaemons(void);

///  Clears half awake mac filter.
Result NDMU_ClearMacFilter(void);

