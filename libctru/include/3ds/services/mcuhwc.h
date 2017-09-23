/**
 * @file mcuhwc.h
 * @brief mcuHwc service.
 */
#pragma once

/// Initializes mcuHwc.
Result mcuHwcInit(void);

/// Exits mcuHwc.
void mcuHwcExit(void);

/**
 * @brief Reads data from a mcuHwc Register
 * @param reg Register number. See https://www.3dbrew.org/wiki/I2C_Registers#Device_3 for more info
 * @param data Pointer to write the data to.
 * @param size Size of data to be read
 */
Result mcuHwcReadRegister(u8 reg, void *data, u32 size);

/**
 * @brief Writes data to a mcuHwc Register
 * @param reg Register number. See https://www.3dbrew.org/wiki/I2C_Registers#Device_3 for more info
 * @param data Pointer to write the data to.
 * @param size Size of data to be written
 */
Result mcuHwcWriteRegister(u8 reg, const void *data, u32 size);

/**
 * @brief Gets the battery voltage
 * @param voltage Pointer to write the battery voltage to.
 */
Result mcuHwcGetBatteryVoltage(u8 *voltage);

/**
 * @brief Gets the battery level
 * @param level Pointer to write the current battery level to.
 */
Result mcuHwcGetBatteryLevel(u8 *level);

/**
 * @brief Gets the sound slider level
 * @param level Pointer to write the slider level to.
 */
Result mcuHwcGetSoundSliderLevel(u8 *level);
