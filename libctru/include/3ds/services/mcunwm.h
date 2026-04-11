/**
 * @file mcunwm.h
 * @brief MCU NWM service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuNwm.
Result mcuNwmInit(void);

/// Exits mcuNwm.
void mcuNwmExit(void);

/**
 * @brief Gets the current mcuNwm session handle.
 * @return A pointer to the current mcuNwm session handle.
 */
Handle *mcuNwmGetSessionHandle(void);

/**
 * @brief Sets the WiFi LED state.
 * @param state State of Wifi LED.
 */
Result MCUNWM_SetWifiLedState(bool state);

/**
 * @brief Gets the WiFi LED state.
 * @param state State of Wifi LED.
 */
Result MCUNWM_GetWifiLedState(bool *out_state);

/**
  * @brief Configures the WiFi module to be in either 3DS mode (CTR mode) or DS[i] mode (MP mode).
  * @param mode The mode to set.
  */
Result MCUNWM_SetWifiMode(MCU_WifiMode mode);

/**
  * @brief Returns the current mode the WiFi module is in.
  * @param outIsMpMode Pointer to output the mode to.
  */
Result MCUNWM_GetWifiMode(MCU_WifiMode *out_mode);

/**
  * @brief Enables or disables WiFi.
  * @param enabled Whether or not the enable WiFi.
  */
Result MCUNWM_SetWifiEnabled(bool enabled);

/**
  * @brief Returns whether or not WiFi is currently enabled.
  * @param out_enabled Pointer to output the status to.
  */
Result MCUNWM_GetWifiEnabled(bool *out_enabled);

/**
  * @brief Sets the MCU flag that indicates whether or not WiFi was disabled previously (for persistence across reboots/FIRMs).
  * @param value The value to set.
  */
Result MCUNWM_SetWirelessDisabledFlag(bool value);

/**
  * @brief Gets the MCU flag that indicates whether or not WiFi was disabled previously (for persistence across reboots/FIRMs).
  * @param out_value Pointer to output the value to.
  */
Result MCUNWM_GetWirelessDisabledFlag(bool *out_value);
