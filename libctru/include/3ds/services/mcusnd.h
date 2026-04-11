/**
 * @file mcusnd.h
 * @brief MCU Sound service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuSnd.
Result mcuSndInit(void);

/// Exits mcuSnd.
void mcuSndExit(void);

/**
 * @brief Gets the current mcuSnd session handle.
 * @return A pointer to the current mcuSnd session handle.
 */
Handle *mcuSndGetSessionHandle(void);

/**
 * @brief Gets the volume slider level.
 * @param level Pointer to write the slider level to.
 */
Result MCUSND_GetVolumeSliderLevel(u8 *level);

/**
  * @brief Writes to MCU register 25h. The value is clamped to 6 bits (max value 63).
  * @param value The value to write.
  */
Result MCUSND_WriteReg25h(u8 value);

/**
  * @brief Reads from MCU register 25h.
  * @param out_value Pointer to output the value to.
  */
Result MCUSND_ReadReg25h(u8 *out_value);
