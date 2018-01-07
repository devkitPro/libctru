/**
 * @file ndm.h
 * @brief NDMU service. https://3dbrew.org/wiki/NDM_Services
 */
#pragma once

/// Exclusive states.
typedef enum {
	EXCLUSIVE_STATE_NONE = 0,
	EXCLUSIVE_STATE_INFRASTRUCTURE = 1,
	EXCLUSIVE_STATE_LOCAL_COMMUNICATIONS = 2,
	EXCLUSIVE_STATE_STREETPASS = 3,
	EXCLUSIVE_STATE_STREETPASS_DATA = 4
} NDM_ExclusiveState;

/// Current states.
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
	STATE_CEC_SUSPENDING = 11
} NDM_State;

// Daemons.
typedef enum {
	DAEMON_CEC = 0,
	DAEMON_BOSS = 1,
	DAEMON_NIM = 2,
	DAEMON_FRIENDS = 3
} NDM_Daemon;

/// Used to specify multiple daemons.
typedef enum {
	DAEMON_MASK_CEC = 1 << DAEMON_CEC,
	DAEMON_MASK_BOSS = 1 << DAEMON_BOSS,
	DAEMON_MASK_NIM = 1 << DAEMON_NIM,
	DAEMON_MASK_FRIENDS = 1 << DAEMON_FRIENDS
} NDM_Daemon_Mask;

// Daemon status.
typedef enum {
	DAEMON_STATUS_BUSY = 0,
	DAEMON_STATUS_IDLE = 1,
	DAEMON_STATUS_SUSPENDING = 2,
	DAEMON_STATUS_SUSPENDED = 3
} NDM_Daemon_Status;

/// Initializes ndmu.
Result ndmuInit(void);

/// Exits ndmu.
void ndmuExit(void);

/**
 * @brief Sets the network daemon to an exclusive state.
 * @param state State specified in the NDM_ExclusiveState enumerator.
 */
Result NDMU_EnterExclusiveState(NDM_ExclusiveState state);

///  Cancels an exclusive state for the network daemon.
Result NDMU_LeaveExclusiveState(void);

/**
 * @brief Returns the exclusive state for the network daemon.
 * @param state Pointer to write the exclsuive state to.
 */
Result NDMU_GetExclusiveState(NDM_ExclusiveState *state);

///  Locks the exclusive state.
Result NDMU_LockState(void);

///  Unlocks the exclusive state.
Result NDMU_UnlockState(void);

/**
 * @brief Suspends scheduling for all network daemons.
 * @param flag 0 = Wait for completion, 1 = Perform in background.
 */
Result NDMU_SuspendScheduler(u32 flag);

/// Resumes daemon scheduling.
Result NDMU_ResumeScheduler(void);

/**
 * @brief Returns the current state for the network daemon.
 * @param state Pointer to write the current state to.
 */
Result NDMU_GetCurrentState(NDM_State *state);

/**
 * @brief Returns the daemon state.
 * @param state Pointer to write the daemons state to.
 */
Result NDMU_QueryStatus(NDM_Daemon_Status *state);

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

/// Reverts network daemon to defaults.
Result NDMU_ResetDaemons(void);

///  Clears half awake mac filter.
Result NDMU_ClearMacFilter(void);

