/**
 * @file ptmsysm.h
 * @brief PTMSYSM service.
 */
#pragma once

#include <3ds/types.h>

/// PDN wake events and MCU interrupts to select, combined with those of other processes
typedef struct PtmWakeEvents {
	u32 pdn_wake_events;    ///< Written to PDN_WAKE_EVENTS. Don't select bit26 (MCU), PTM will do it automatically.
	u32 mcu_interupt_mask;  ///< MCU interrupts to check when a MCU wake event happens.
} PtmWakeEvents;

typedef struct {
	PtmWakeEvents exit_sleep_events;       ///< Wake events for which the system should fully wake up.
	PtmWakeEvents continue_sleep_events;   ///< Wake events for which the system should return to sleep.
} PtmSleepConfig;

enum {
	// Sleep FSM notification IDs
	PTMNOTIFID_SLEEP_REQUESTED  = 0x101, ///< @ref PTMSYSM_RequestSleep has been called (ack = 3)
	PTMNOTIFID_SLEEP_DENIED     = 0x102, ///< The sleep request has been denied by @ref PTMSYSM_ReplyToSleepQuery(true) (no ack required).
	PTMNOTIFID_SLEEP_ALLOWED    = 0x103, ///< The sleep request has been allowed by @ref PTMSYSM_ReplyToSleepQuery(false) (ack = 1).
	PTMNOTIFID_GOING_TO_SLEEP   = 0x104, ///< All processes not having "RunnableOnSleep" have been paused & the system is about to go to sleep (ack = 0).
	PTMNOTIFID_FULLY_WAKING_UP  = 0x105, ///< The system has been woken up, and the paused processes are about to be unpaused (ack = 1).
	PTMNOTIFID_FULLY_AWAKE      = 0x106, ///< The system is fully awake (no ack required).
	PTMNOTIFID_HALF_AWAKE       = 0x107, ///< The system has been woken up but is about to go to sleep again (ack = 2).

	PTMNOTIFID_SHUTDOWN         = 0x108, ///< The system is about to power off or reboot.

	PTMNOTIFID_BATTERY_VERY_LOW = 0x211, ///< The battery level has reached 5% or below.
	PTMNOTIFID_BATTERY_LOW      = 0x212, ///< The battery level has reached 10% or below.
};

/// See @ref PTMSYSM_NotifySleepPreparationComplete. Corresponds to the number of potentially remaning notifs. until sleep/wakeup.
static inline s32 ptmSysmGetNotificationAckValue(u32 id)
{
	static const s32 values[] = { 3, -1, 1, 0, 0, -1, 2 };
	if (id < PTMNOTIFID_SLEEP_REQUESTED || id > PTMNOTIFID_HALF_AWAKE)
		return -1;
	return values[id - PTMNOTIFID_SLEEP_REQUESTED];
}

/// Initializes ptm:sysm.
Result ptmSysmInit(void);

/// Exits ptm:sysm.
void ptmSysmExit(void);

/// Requests to enter sleep mode.
Result PTMSYSM_RequestSleep(void);

/**
 * @brief Accepts or denies the incoming sleep mode request.
 * @param deny Whether or not to deny the sleep request.
 * @note If deny = false, this is equivalent to calling @ref PTMSYSM_NotifySleepPreparationComplete(3)
 */
Result PTMSYSM_ReplyToSleepQuery(bool deny);

/**
 * @brief Acknowledges the current sleep notification and advance the internal sleep mode FSM. All subscribers must reply.
 * @param ackValue Use @ref ptmSysmGetNotificationAckValue
 * @note @ref PTMNOTIFID_SLEEP_DENIED and @ref PTMNOTIFID_FULLY_AWAKE don't require this.
 */
Result PTMSYSM_NotifySleepPreparationComplete(s32 ackValue);

/**
 * @brief Sets the wake events (two sets: when to fully wake up and when to return to sleep).
 * @param sleepConfig Pointer to the two sets of wake events.
 * @note Can only be called just before acknowledging @ref PTMNOTIFID_GOING_TO_SLEEP or @ref PTMNOTIFID_HALF_AWAKE.
 */
Result PTMSYSM_SetWakeEvents(const PtmSleepConfig *sleepConfig);

/**
 * @brief Gets the wake reason (only the first applicable wake event is taken into account).
 * @param sleepConfig Pointer to the two sets of wake events. Only the relevant set will be filled.
 */
Result PTMSYSM_GetWakeReason(PtmSleepConfig *outSleepConfig);

/// Cancels the "half-awake" state and fully wakes up the 3DS after some delay.
Result PTMSYSM_Awaken(void);

/// Invalidates the "system time" (cfg block 0x30002)
Result PTMSYSM_InvalidateSystemTime(void);

/**
 * @brief Reads the time and date coming from the RTC and converts the result.
 * @returns The number of milliseconds since 01/01/2000.
 */
Result PTMSYSM_GetRtcTime(s64 *outMsY2k);

/**
 * @brief Writes the time and date coming to the RTC, after conversion.
 * @param msY2k The number of milliseconds since 01/01/2000.
 */
Result PTMSYSM_SetRtcTime(s64 msY2k);

/**
 * @brief Returns 1 if it's a New 3DS, otherwise 0.
 */
Result PTMSYSM_CheckNew3DS(void);

/**
 * @brief Configures the New 3DS' CPU clock speed and L2 cache.
 * @param value Bit0: enable higher clock, Bit1: enable L2 cache.
 */
Result PTMSYSM_ConfigureNew3DSCPU(u8 value);

/**
 * @brief Trigger a hardware system shutdown via the MCU.
 * @param timeout: timeout passed to PMApp:ShutdownAsync (PrepareForReboot).
 */
Result PTMSYSM_ShutdownAsync(u64 timeout);

/**
 * @brief Trigger a hardware system reboot via the MCU.
 * @param timeout: timeout passed to PMApp:ShutdownAsync (PrepareForReboot).
 */
Result PTMSYSM_RebootAsync(u64 timeout);
