/**
 * @file mcuHwc.h
 * @brief mcuHWC service.
 */
#pragma once

/// Initializes mcuHWC.
Result mcuHWCInit(void);

/// Exits mcuHWC.
void mcuHWCExit(void);

/**
 * @brief Reads data from a MCU Register
 * @param reg Register number. See https://www.3dbrew.org/wiki/I2C_Registers#Device_3 for more info
 * @param data Pointer to write the data to.
 * @param size Size of data to be read
 */
Result mcuReadRegister(u8 reg, void *data, u32 size);

/**
 * @brief Writes data to a MCU Register
 * @param reg Register number. See https://www.3dbrew.org/wiki/I2C_Registers#Device_3 for more info
 * @param data Pointer to write the data to.
 * @param size Size of data to be written
 */
Result mcuWriteRegister(u8 reg, const void *data, u32 size);

/**
 * @brief Gets the battery voltage
 * @param voltage Pointer to write the battery voltage to.
 */
Result mcuGetBatteryVoltage(u8 *voltage);

/**
 * @brief Gets the battery level
 * @param level Pointer to write the current battery level to.
 */
Result mcuGetBatteryLevel(u8 *level);

/**
 * @brief Gets the sound slider level
 * @param level Pointer to write the slider level to.
 */
Result mcuGetSoundSliderLevel(u8 *level);