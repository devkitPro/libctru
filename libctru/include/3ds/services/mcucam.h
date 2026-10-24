/**
 * @file mcucam.h
 * @brief MCU Camera service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuCam.
Result mcuCamInit(void);

/// Exits mcuCam.
void mcuCamExit(void);

/**
 * @brief Gets the current mcuCam session handle.
 * @return A pointer to the current mcuCam session handle.
 */
Handle *mcuCamGetSessionHandle(void);

/**
  * @brief Sets the camera LED state.
  * @param on Whether or not the camera LED should be turned on.
  */
Result MCUCAM_SetCameraLedState(bool on);

/**
  * @brief Returns whether or not the camera LED is on.
  * @param out_on Pointer to output whether or not the camera LED is on.
  */
Result MCUCAM_GetCameraLedState(bool *out_on);