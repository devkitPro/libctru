/**
 * @file mcuhwc.h
 * @brief MCU Hardware Control service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuHwc.
Result mcuHwcInit(void);

/// Exits mcuHwc.
void mcuHwcExit(void);

/**
 * @brief Gets the current mcuHwc session handle.
 * @return A pointer to the current mcuHwc session handle.
 */
Handle *mcuHwcGetSessionHandle(void);

/**
 * @brief Reads data from an i2c device3 register
 * @param reg Register number. See https://www.3dbrew.org/wiki/I2C_Registers#Device_3 for more info
 * @param data Pointer to write the data to.
 * @param size Size of data to be read
 */
Result MCUHWC_ReadRegister(u8 reg, void *data, u32 size);

/**
 * @brief Writes data to a i2c device3 register
 * @param reg Register number. See https://www.3dbrew.org/wiki/I2C_Registers#Device_3 for more info
 * @param data Pointer to write the data to.
 * @param size Size of data to be written
 */
Result MCUHWC_WriteRegister(u8 reg, const void *data, u32 size);

/**
 * @brief Reads the info registers of the MCU.
 * @param data Pointer to write the info to.
 * @param size Size of the info. To read info data at a given offset, at least (offset + wanted data size) bytes must be read.
 */
Result MCUHWC_ReadInfoRegister(void *data, u8 size);

/**
 * @brief Gets the battery voltage in 20 mV increments.
 * @param voltage Pointer to write the battery voltage to.
 */
Result MCUHWC_GetBatteryVoltage(u8 *voltage);

/**
 * @brief Gets the battery level as a percentage.
 * @param level Pointer to write the current battery level to.
 */
Result MCUHWC_GetBatteryLevel(u8 *level);

/**
 * @brief Sets the Power LED state.
 * @param state powerLedState State of power LED.
 */
Result MCUHWC_SetPowerLedState(MCU_PowerLedState state);

/**
 * @brief Sets the WiFi LED state.
 * @param state State of Wifi LED. (True/False)
 */
Result MCUHWC_SetWifiLedState(bool state);

/**
 * @brief Sets the Camera LED state.
 * @param state State of Camera LED. (True/False)
 */
Result MCUHWC_SetCameraLedState(bool state);

/**
 * @brief Sets the 3D LED state.
 * @param state State of 3D LED. (True/False)
 */
Result MCUHWC_Set3dLedState(bool state);

/**
 * @brief Sets the info (notification) LED pattern.
 * @param pattern Pattern for the info LED.
 */
Result MCUHWC_SetInfoLedPattern(const MCU_InfoLedPattern *pattern);

/**
 * @brief Gets the volume slider level.
 * @param level Pointer to write the slider level to.
 */
Result MCUHWC_GetVolumeSliderLevel(u8 *level);

/**
 * @brief Sets the flicker (VCOM) value for the top screen.
 * @param flicker The new value to use. Default value: 0x5C
 */
Result MCUHWC_SetTopLcdFlicker(u8 flicker);

/**
 * @brief Sets the flicker (VCOM) value for the bottom screen.
 * @param flicker The new value to use. Default value: 0x5F
 */
Result MCUHWC_SetBottomLcdFlicker(u8 flicker);

/**
 * @brief Gets the battery temperature using a sensor on the console's PCB.
 * @param out_value Pointer to output the temperature (in degrees Celsius) to.
 */
Result MCUHWC_GetBatteryPcbTemperature(s8 *out_value);

/**
 * @brief Reads the current RTC time.
 * @param out_time Pointer to output the RTC time to.
 */
Result MCUHWC_GetRtcTime(MCU_RtcTime *out_time);

/**
 * @brief Gets the major MCU firmware version.
 * @param out Pointer to write the major firmware version to.
 */
Result MCUHWC_GetFwVerHigh(u8 *out);

/**
 * @brief Gets the minor MCU firmware version.
 * @param out Pointer to write the minor firmware version to.
 */
Result MCUHWC_GetFwVerLow(u8 *out);
