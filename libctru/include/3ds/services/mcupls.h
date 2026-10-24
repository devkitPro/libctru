/**
 * @file mcupls.h
 * @brief MCU PLS (Platform Services) service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuPls.
Result mcuPlsInit(void);

/// Exits mcuPls.
void mcuPlsExit(void);

/**
 * @brief Gets the current mcuPls session handle.
 * @return A pointer to the current mcuPls session handle.
 */
Handle *mcuPlsGetSessionHandle(void);

/**
  * @brief Gets the RTC time.
  * @param out_time Pointer to output the RTC time to.
  */
Result MCUPLS_GetRtcTime(MCU_RtcTime *out_time);

/**
  * @brief Gets the seconds part of the RTC time.
  * @param out_value Pointer to output the value to.
  */
Result MCUPLS_GetRtcTimeSeconds(u8 *out_value);

/**
  * @brief Gets the minute part of the RTC time.
  * @param out_value Pointer to output the value to.
  */
Result MCUPLS_GetRtcTimeMinute(u8 *out_value);

/**
  * @brief Gets the hour part of the RTC time.
  * @param out_value Pointer to output the value to.
  */
Result MCUPLS_GetRtcTimeHour(u8 *out_value);

/**
  * @brief Gets the weekday part of the RTC time.
  * @param out_value Pointer to output the value to.
  */
Result MCUPLS_GetRtcTimeWeekday(u8 *out_value);

/**
  * @brief Gets the day part of the RTC time.
  * @param out_value Pointer to output the value to.
  */
Result MCUPLS_GetRtcTimeDay(u8 *out_value);

/**
  * @brief Gets the month part of the RTC time.
  * @param out_value Pointer to output the value to.
  */
Result MCUPLS_GetRtcTimeMonth(u8 *out_value);

/**
  * @brief Gets the year since 2000 part of the RTC time.
  * @param out_value Pointer to output the value to.
  */
Result MCUPLS_GetRtcTimeYear(u8 *out_value);

/**
  * @brief Gets MCU's system tick.
  * @param out_value Pointer to output the value to.
  */
Result MCUPLS_GetTickCounter(u16 *out_value);
