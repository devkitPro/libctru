/**
 * @file mcuHWC.h
 * @brief mcuHWC service.
 */
#pragma once

/// Initializes mcuHWC.
Result  mcuHWCInit(void);

/// Exits mcuHWC.
void mcuHWCExit(void);

/**
 * @brief Reads data from a MCU Register
 * @param reg Register number. See https://www.3dbrew.org/wiki/I2C_Registers#Device_3 for more info
 * @param size Size of data to be read
 * @param data Pointer to write the data to.
 */
Result mcuReadRegister(u8 reg, u32 size, u8* data);

/**
 * @brief Writes data to a MCU Register
 * @param reg Register number. See https://www.3dbrew.org/wiki/I2C_Registers#Device_3 for more info
 * @param size Size of data to be written
 * @param data Pointer to write the data from.
 */
Result mcuWriteRegister(u8 reg, u32 size, u8* data);

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

