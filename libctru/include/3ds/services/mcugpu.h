/**
 * @file mcugpu.h
 * @brief MCU GPU service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuGpu.
Result mcuGpuInit(void);

/// Exits mcuGpu.
void mcuGpuExit(void);

/**
 * @brief Gets the current mcuGpu session handle.
 * @return A pointer to the current mcuGpu session handle.
 */
Handle *mcuGpuGetSessionHandle(void);

/**
 * @brief Returns whether the screen backlights are on.
 * @param out_top_on Pointer to output whether or not the top screen backlight is on.
 * @param out_bot_on Pointer to output whether or not the bottom screen backlight is on.
 */
Result MCUGPU_GetBacklightPower(bool *out_top_on, bool *out_bot_on);

/**
 * @brief Turns the top/bottom screen backlights on/off.
 * @param top_on Whether or not the top screen backlight should be turned on.
 * @param bot_on Whether or not the bottom screen backlight should be turned on.
 */
Result MCUGPU_SetBacklightPower(bool top_on, bool bot_on);

/**
 * @brief Returns whether the LCDs are on.
 * @param out_on Pointer to output whether or not the LCDs are on.
 */
Result MCUGPU_GetLcdPower(bool *out_on);

/**
 * @brief Turns the LCDs on/off.
 * @param on Whether or not the LCDs should be turned on.
 */
Result MCUGPU_SetLcdPower(bool on);

/**
 * @brief Sets the flicker (VCOM) value for the top screen.
 * @param flicker The new value to use. Default value: 0x5C
 */
Result MCUGPU_SetTopLcdFlicker(u8 flicker);

/**
 * @brief Gets the flicker (VCOM) value for the top screen.
 * @param out_flicker Pointer to output the flicker value to.
 */
Result MCUGPU_GetTopLcdFlicker(u8 *out_flicker);

/**
 * @brief Sets the flicker (VCOM) value for the bottom screen.
 * @param flicker The new value to use. Default value: 0x5F
 */
Result MCUGPU_SetBottomLcdFlicker(u8 flicker);

/**
 * @brief Gets the flicker (VCOM) value for the bottom screen.
 * @param out_flicker Pointer to output the flicker value to.
 */
Result MCUGPU_GetBottomLcdFlicker(u8 *out_flicker);

/**
 * @brief Gets the major MCU firmware version
 * @param out Pointer to write the major firmware version to.
 */
Result MCUGPU_GetFwVerHigh(u8 *out);

/**
 * @brief Gets the minor MCU firmware version
 * @param out Pointer to write the minor firmware version to.
 */
Result MCUGPU_GetFwVerLow(u8 *out);

/**
 * @brief Sets the 3D LED state.
 * @param state State of 3D LED. (True/False)
 */
Result MCUGPU_Set3dLedState(bool state);

/**
 * @brief Gets the 3D LED state.
 * @param out_state Pointer to output whether or not the 3D LED is on.
 */
Result MCUGPU_Get3dLedState(bool *out_state);

/**
 * @brief Gets the event handle for MCUGPU related events.
 * @param out_event Pointer to output the event handle to.
 */
Result MCUGPU_GetEventHandle(Handle *out_event);

/**
  * @brief Gets the GPU events that have recently been received by the MCU.
  * @param out_events Pointer to output the received events to.
  */
Result MCUGPU_GetReceivedEvents(u32 *out_events);