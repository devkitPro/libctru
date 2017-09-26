/**
 * @file mcuhwc.h
 * @brief mcuHwc service.
 */
#pragma once

typedef enum
{
	LED_NORMAL = 1,	///< The normal mode of the led
	LED_SLEEP_MODE,	///< The led pulses slowly as it does in the sleep mode
	LED_OFF, 	///< Switch off power led
	LED_RED,	///< Red state of the led
	LED_BLUE,	///< Blue state of the led
	LED_BLINK_RED,	///< Blinking red state of power led and notification led
}powerLedState;

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

/**
 * @brief Sets Wifi LED state
 * @param state State of Wifi LED. (True/False)
 */
Result mcuHwcSetWifiLedState(bool state);

/**
 * @brief Sets Power LED state
 * @param state powerLedState State of power LED.
 */
Result mcuHwcSetPowerLedState(powerLedState state);

/**
 * @brief Gets 3d slider level
 * @param level Pointer to write 3D slider level to.
 */
 Result mcuHwcGet3dSliderLevel(u8 *level);