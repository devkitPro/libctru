/**
 * @file mcurtc.h
 * @brief MCU RTC service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuRtc.
Result mcuRtcInit(void);

/// Exits mcuRtc.
void mcuRtcExit(void);

/**
 * @brief Gets the current mcuRtc session handle.
 * @return A pointer to the current mcuRtc session handle.
 */
Handle *mcuRtcGetSessionHandle(void);

/**
  * @brief Sets the RTC time.
  * @param time The RTC time to set.
  */
Result MCURTC_SetRtcTime(const MCU_RtcTime *time);

/**
  * @brief Gets the RTC time.
  * @param out_time Pointer to output the RTC time to.
  * @param out_tick Pointer to output the system ticks at the time of reading the RTC to.
  */
Result MCURTC_GetRtcTime(MCU_RtcTime *out_time, s64 *out_tick);

/**
  * @brief Sets the seconds part of the RTC time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcTimeSeconds(u8 value);

/**
  * @brief Gets the seconds part of the RTC time.
  * @param out_value Pointer to output the value to.
  * @param out_tick Pointer to output the system ticks at the time of reading the RTC to.
  */
Result MCURTC_GetRtcTimeSeconds(u8 *out_value, s64 *out_tick);

/**
  * @brief Sets the minute part of the RTC time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcTimeMinute(u8 value);

/**
  * @brief Gets the minute part of the RTC time.
  * @param out_value Pointer to output the value to.
  * @param out_tick Pointer to output the system ticks at the time of reading the RTC to.
  */
Result MCURTC_GetRtcTimeMinute(u8 *out_value, s64 *out_tick);

/**
  * @brief Sets the hour part of the RTC time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcTimeHour(u8 value);

/**
  * @brief Gets the hour part of the RTC time.
  * @param out_value Pointer to output the value to.
  * @param out_tick Pointer to output the system ticks at the time of reading the RTC to.
  */
Result MCURTC_GetRtcTimeHour(u8 *out_value, s64 *out_tick);

/**
  * @brief Sets the weekday part of the RTC time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcTimeWeekday(u8 value);

/**
  * @brief Gets the weekday part of the RTC time.
  * @param out_value Pointer to output the value to.
  * @param out_tick Pointer to output the system ticks at the time of reading the RTC to.
  */
Result MCURTC_GetRtcTimeWeekday(u8 *out_value, s64 *out_tick);

/**
  * @brief Sets the day part of the RTC time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcTimeDay(u8 value);

/**
  * @brief Gets the day part of the RTC time.
  * @param out_value Pointer to output the value to.
  * @param out_tick Pointer to output the system ticks at the time of reading the RTC to.
  */
Result MCURTC_GetRtcTimeDay(u8 *out_value, s64 *out_tick);

/**
  * @brief Sets the month part of the RTC time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcTimeMonth(u8 value);

/**
  * @brief Gets the month part of the RTC time.
  * @param out_value Pointer to output the value to.
  * @param out_tick Pointer to output the system ticks at the time of reading the RTC to.
  */
Result MCURTC_GetRtcTimeMonth(u8 *out_value, s64 *out_tick);

/**
  * @brief Sets the year since 2000 part of the RTC time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcTimeYear(u8 value);

/**
  * @brief Gets the year since 2000 part of the RTC time.
  * @param out_value Pointer to output the value to.
  * @param out_tick Pointer to output the system ticks at the time of reading the RTC to.
  */
Result MCURTC_GetRtcTimeYear(u8 *out_value, s64 *out_tick);

/**
  * @brief Sets the correction part of the RTC time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcTimeCorrection(u8 value);

/**
  * @brief Gets the correction part of the RTC time.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetRtcTimeCorrection(u8 *out_value);

/**
  * @brief Sets the RTC alarm time.
  * @param time The RTC alarm time to set.
  */
Result MCURTC_SetRtcAlarmTime(const MCU_RtcAlarmTime *time);

/**
  * @brief Gets the RTC alarm time.
  * @param out_time Pointer to output the RTC alarm time to.
  */
Result MCURTC_GetRtcAlarmTime(MCU_RtcAlarmTime *out_time);

/**
  * @brief Sets the minute part of the RTC alarm time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcAlarmTimeMinute(u8 value);

/**
  * @brief Gets the minute part of the RTC alarm time.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetRtcAlarmTimeMinute(u8 *out_value);

/**
  * @brief Sets the hour part of the RTC alarm time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcAlarmTimeHour(u8 value);

/**
  * @brief Gets the hour part of the RTC alarm time.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetRtcAlarmTimeHour(u8 *out_value);

/**
  * @brief Sets the day part of the RTC alarm time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcAlarmTimeDay(u8 value);

/**
  * @brief Gets the day part of the RTC alarm time.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetRtcAlarmTimeDay(u8 *out_value);

/**
  * @brief Sets the month part of the RTC alarm time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcAlarmTimeMonth(u8 value);

/**
  * @brief Gets the month part of the RTC alarm time.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetRtcAlarmTimeMonth(u8 *out_value);

/**
  * @brief Sets the year since 2000 part of the RTC alarm time.
  * @param value The value to set.
  */
Result MCURTC_SetRtcAlarmTimeYear(u8 value);

/**
  * @brief Gets the year since 2000 part of the RTC alarm time.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetRtcAlarmTimeYear(u8 *out_value);

/**
  * @brief Enables/disables the pedometer.
  * @param enabled Whether or not the pedometer should be enabled.
  */
Result MCURTC_SetPedometerEnabled(bool enabled);

/**
  * @brief Returns whether or not the pedometer is enabled.
  * @param out_enabled Pointer to output the status to.
  */
Result MCURTC_GetPedometerEnabled(bool *out_enabled);

/**
  * @brief Returns the total number of steps measured by the pedometer.
  * @param out_num_steps Pointer to output the number of steps to.
  */
Result MCURTC_ReadPedometerStepCount(u32 *out_num_steps);

/**
  * @brief Reads step data recorded by the pedometer.
  * @param out_data Pointer to output the step data to.
  */
Result MCURTC_ReadPedometerStepData(MCU_PedometerStepData *out_data);

/**
  * @brief Clears the current step data of the pedometer.
  */
Result MCURTC_ClearStepData();

/**
 * @brief Gets the interrupt event handle for power-related interrupts.
 * @param out_event Pointer to output the interrupt event handle to.
 */
Result MCURTC_GetInterruptEventHandle(Handle *out_handle);

/**
  * @brief Reads the recently received power-related interrupts.
  * @param out_irqs Pointer to output the received interrupts to.
  */
Result MCURTC_GetReceivedInterrupts(u32 *out_irqs);

/**
  * @brief Checks whether or not RTC time was lost. (This can happen due to the battery being removed, for example)
  * @param out_lost Pointer to output the status to.
  */
Result MCURTC_CheckRtcTimeLost(bool *out_lost);

/**
  * @brief Clears the "RTC time lost" flag.
  */
Result MCURTC_ClearRtcTimeLost();

/**
  * @brief Checks whether an MCU watchdog reset occurred.
  * @param out_occurred Pointer to output the status to.
  */
Result MCURTC_CheckWatchdogResetOccurred(bool *out_occurred);

/**
  * @brief Clears the "watchdog reset occurred" flag.
  */
Result MCURTC_ClearWatchdogResetOccurred();

/**
  * @brief Returns the shell open/closed (or sleep switch on/off on the original, non-XL 2DS) state.
  * @param out_open Pointer to output the state to.
  */
Result MCURTC_GetShellState(bool *out_open);

/**
  * @brief Returns whether or not the AC adapter is connected.
  * @param out_open Pointer to output the state to.
  */
Result MCURTC_GetAdapterState(bool *out_connected);

/**
  * @brief Returns whether or not the system is charging.
  * @param out_open Pointer to output the state to.
  */
Result MCURTC_GetChargingState(bool *out_connected);

/**
  * @brief Gets the battery level as a percentage.
  * @param out_level Pointer to output the value to.
  */
Result MCURTC_GetBatteryLevel(u8 *out_level);

/**
  * @brief Sets the power LED state.
  * @param state The state to set.
  */
Result MCURTC_SetPowerLedState(MCU_PowerLedState state);

/**
  * @brief Gets the power LED state.
  * @param out_state Pointer to output the state to.
  */
Result MCURTC_GetPowerLedState(MCU_PowerLedState *out_state);

/**
  * @brief Sets the brightness of the system LEDs (WiFi/Power/3D/etc).
  * @param value The brightness to set (0-255).
  */
Result MCURTC_SetLedBrightness(u8 value);

/**
  * @brief Gets the brightness of the system LEDs (WiFi/Power/3D/etc).
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetLedBrightness(u8 *out_value);

/**
  * @brief Performs a hardware shutdown.
  */
Result MCURTC_Poweroff();

/**
  * @brief Performs a hardware reboot.
  */
Result MCURTC_Reboot();

/**
  * @brief Performs a hardware reset.
  */
Result MCURTC_Reset();

/**
  * @brief Signals to the MCU that the system is about to enter sleep mode.
  */
Result MCURTC_SignalEnterSleepMode();

/**
  * @brief Sets the delay time for force shutdowns performed by holding the power button.
  * @param value The time in 8Hz units. Time in milliseconds = value * 125. Default value: 0x5D (11.625s)
  */
Result MCURTC_SetForceShutdownDelay(u8 value);

/**
  * @brief Gets the delay time for force shutdowns performed by holding the power button.
  * @param out_value Pointer to output the value to. The value in 8Hz units. Time in milliseconds = value * 125.
  */
Result MCURTC_GetForceShutdownDelay(u8 *out_value);

/**
 * @brief Reads the info registers of the MCU.
 * @param data Pointer to write the info to.
 * @param size Size of the info. To read info data at a given offset, at least (offset + wanted data size) bytes must be read.
 */
Result MCURTC_ReadInfoRegister(void *data, u8 size);

/**
  * @brief Writes to part of the battery-backed RAM storage area of the MCU.
  * @param offset Offset to write to. The offset is relative to the PTM area (0x8) (see @ref MCU_PlayTimeStorageArea). Writing to an offset less than 0x8 is not allowed.
  * @param size Amount of bytes to write, starting at the given offset. Writing past offset 0xC8 (relative offset 0xC0) is not allowed.
  */
Result MCURTC_WriteStorageArea(u8 offset, const void *buf, u8 size);

/**
  * @brief Reads from part of the battery-backed RAM storage area of the MCU.
  * @param offset Offset to read from. The offset is relative to the PTM area (0x8) (see @ref MCU_PlayTimeStorageArea). Reading from an offset less than 0x8 is not allowed.
  * @param size Amount of bytes to read, starting at the given offset. Reading past 0xC8 (relative offset 0xC0) is not allowed.
  */
Result MCURTC_ReadStorageArea(u8 offset, void *buf, u8 size);

/**
 * @brief Sets the info (notification) LED pattern.
 * @param pattern Pattern for the info LED.
 */
Result MCURTC_SetInfoLedPattern(const MCU_InfoLedPattern *pattern);

/**
  * @brief Sets the info (notification) LED animation, without changing the pattern.
  * @param animation The new animation to use.
  */
Result MCURTC_SetInfoLedAnimation(const MCU_InfoLedAnimation *animation);

/**
  * @brief Checks whether or not a new info (notification) LED pattern cycle has been started.
  * @param out_new_cycle_started Pointer to output the status to.
  */
Result MCURTC_GetInfoLedCycleStatus(bool *out_new_cycle_started);

/**
  * @brief Sets the minute within every hour of RTC time when a new hour worth of step data should be published.
  * @param value The value to set.
  */
Result MCURTC_SetPedometerWrapTimeMinute(u8 value);

/**
  * @brief Gets the minute within every hour of RTC time when a new hour worth of step data should be published.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetPedometerWrapTimeMinute(u8 *out_value);

/**
  * @brief Sets the second within every hour of RTC time when a new hour worth of step data should be published.
  * @param value The value to set.
  */
Result MCURTC_SetPedometerWrapTimeSecond(u8 value);

/**
  * @brief Gets the second within every hour of RTC time when a new hour worth of step data should be published.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetPedometerWrapTimeSecond(u8 *out_value);

/**
  * @brief Sets the blink pattern for when the battery is critically low.
  * @param pattern The new pattern to use. Default: 0x55555555. The pattern consists of 32 steps corresponding to the 32 bits of the pattern.
  */
Result MCURTC_SetPowerLedBlinkPattern(u32 pattern);

/**
 * @brief Sets the flicker (VCOM) value for the top screen.
 * @param flicker The new value to use. Default value: 0x5C
 */
Result MCURTC_SetTopLcdFlicker(u8 flicker);

/**
 * @brief Gets the flicker (VCOM) value for the top screen.
 * @param out_flicker Pointer to output the flicker value to.
 */
Result MCURTC_GetTopLcdFlicker(u8 *out_flicker);

/**
 * @brief Sets the flicker (VCOM) value for the bottom screen.
 * @param flicker The new value to use. Default value: 0x5F
 */
Result MCURTC_SetBottomLcdFlicker(u8 flicker);

/**
 * @brief Gets the flicker (VCOM) value for the bottom screen.
 * @param out_flicker Pointer to output the flicker value to.
 */
Result MCURTC_GetBottomLcdFlicker(u8 *out_flicker);

/**
  * @brief Sets the minimum and maximum levels for the volume slider.
  * @param min The minimum level. Default: 0x24 (+8 in PTM).
  * @param max The maximum level. Default: 0xDB (-8 in PTM).
  */
Result MCURTC_SetVolumeSliderBounds(u8 min, u8 max);

/**
  * @brief Gets the minimum and maximum levels for the volume slider.
  * @param out_min Pointer to output the minimum level to.
  * @param out_max Pointer to output the maximum level to.
  */
Result MCURTC_GetVolumeSliderBounds(u8 *out_min, u8 *out_max);

/**
  * @brief Sets the MCU interrupt mask.
  * @Param mask Bitmask for enabled interrupts.
  */
Result MCURTC_SetInterruptMask(u32 mask);

/**
  * @brief Gets the MCU interrupt mask.
  * @Param out_mask Pointer to output the interrupt mask to.
  */
Result MCURTC_GetInterruptMask(u32 *out_mask);

/**
  * @brief Exits exclusive interrupt mode and transfers control of IRQs to the MCU system module.
  */
Result MCURTC_LeaveExclusiveInterruptMode();

/**
  * @brief Enters exclusive interrupt mode, taking control of IRQs from the MCU system module.
  */
Result MCURTC_EnterExclusiveInterruptMode();

/**
  * @brief Reads received interrupts and acknowledges them.
  * @param out_interrupts Pointer to output the received interrupts to.
  */
Result MCURTC_ReadInterrupts(u32 *out_interrupts);

/**
  * @brief Forcefully triggers the MCU interrupt handler to handle the given set of IRQs, even if none of them happened.
  * @param interrupts Bitmask of the interrupts to force-trigger.
  */
Result MCURTC_TriggerInterrupts(u32 interrupts);

/**
  * @brief Sets the MCU flag indicating whether or not the MCU firmware was updated.
  * @param value The value to set.
  */
Result MCURTC_SetMcuFirmUpdatedFlag(bool value);

/**
  * @brief Gets the MCU flag indicating whether or not the MCU firmware was updated.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetMcuFirmUpdatedFlag(bool *out_value);

/**
  * @brief Sets the MCU flag indicating whether or not either a legacy title or System Settings was closed, as a result of pressing the power button to trigger a reboot.
  * @param value The value to set.
  */
Result MCURTC_SetSoftwareClosedFlag(bool value);

/**
  * @brief Gets the MCU flag indicating whether or not either a legacy title or System Settings was closed, as a result of pressing the power button to trigger a reboot.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetSoftwareClosedFlag(bool *out_value);

/**
  * @brief Sets LCD settings for legacy titles.
  * @param settings The settings data to set.
  */
Result MCURTC_SetLgyLcdSettings(MCU_LcdSettings settings);

/**
  * @brief Gets the LCD settings for legacy titles.
  * @param out_config Pointer to output the settings data to.
  */
Result MCURTC_GetLgyLcdConfig(MCU_LcdSettings *out_settings);

/**
  * @brief Sets the MCU flag that indicates whether or the legacy title should play in native resolution.
  * @param value The value to set.
  */
Result MCURTC_SetLgyNativeResolutionFlag(bool value);

/**
  * @brief Gets the MCU flag that indicates whether or the legacy title should play in native resolution.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetLgyNativeResolutionFlag(bool *out_value);

/**
  * @brief Sets the local friend code counter.
  * @param value The value to set.
  */
Result MCURTC_SetLocalFriendCodeCounter(u16 value);

/**
  * @brief Gets the local friend code counter.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetLocalFriendCodeCounter(u16 *out_value);

/**
  * @brief Sets the LegacyJumpProhibited MCU flag.
  * @param value The value to set.
  */
Result MCURTC_SetLegacyJumpProhibitedFlag(bool value);

/**
  * @brief Gets the LegacyJumpProhibited MCU flag.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetLegacyJumpProhibitedFlag(bool *out_value);

/**
  * @brief Sets the UUID clock sequence.
  * @param value The value to set.
  */
Result MCURTC_SetUuidClockSequence(u16 value);

/**
  * @brief Gets the UUID clock sequence.
  * @param out_value Pointer to output the value to.
  */
Result MCURTC_GetUuidClockSequence(u16 *out_value);
