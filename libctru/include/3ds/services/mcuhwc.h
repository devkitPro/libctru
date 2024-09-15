/**
 * @file mcuhwc.h
 * @brief mcuHwc service.
 */
#pragma once

typedef enum {
	LED_NORMAL = 1,         ///< The normal mode of the led
	LED_SLEEP_MODE,         ///< The led pulses slowly as it does in the sleep mode
	LED_OFF,                ///< Switch off power led
	LED_RED,                ///< Red state of the led
	LED_BLUE,               ///< Blue state of the led
	LED_BLINK_RED,          ///< Blinking red state of power led and notification led
} powerLedState;

typedef struct InfoLedPattern
{
	u8 delay;               ///< Delay between pattern values, 1/16th of a second (1 second = 0x10)
	u8 smoothing;           ///< Smoothing between pattern values (higher = smoother)
	u8 loopDelay;           ///< Delay between pattern loops, 1/16th of a second (1 second = 0x10, 0xFF = pattern is played only once)
	u8 blinkSpeed;          ///< Blink speed, when smoothing == 0x00
	u8 redPattern[32];      ///< Pattern for red component
	u8 greenPattern[32];    ///< Pattern for green component
	u8 bluePattern[32];     ///< Pattern for blue component
} InfoLedPattern;

/// Initializes mcuHwc.
Result mcuHwcInit(void);

/// Exits mcuHwc.
void mcuHwcExit(void);

/**
 * @brief Gets the current mcuHwc session handle.
 * @return A pointer to the current mcuHwc session handle.
 */
Handle* mcuHwcGetSessionHandle(void);

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
 * @brief Gets the battery voltage
 * @param voltage Pointer to write the battery voltage to.
 */
Result MCUHWC_GetBatteryVoltage(u8 *voltage);

/**
 * @brief Gets the battery level
 * @param level Pointer to write the current battery level to.
 */
Result MCUHWC_GetBatteryLevel(u8 *level);

/**
 * @brief Gets the sound slider level
 * @param level Pointer to write the slider level to.
 */
Result MCUHWC_GetSoundSliderLevel(u8 *level);

/**
 * @brief Sets Wifi LED state
 * @param state State of Wifi LED. (True/False)
 */
Result MCUHWC_SetWifiLedState(bool state);

/**
 * @brief Sets the notification LED pattern
 * @param pattern Pattern for the notification LED.
 */
Result MCUHWC_SetInfoLedPattern(const InfoLedPattern* pattern);

/**
 * @brief Sets Power LED state
 * @param state powerLedState State of power LED.
 */
Result MCUHWC_SetPowerLedState(powerLedState state);

/**
 * @brief Gets 3d slider level
 * @param level Pointer to write 3D slider level to.
 */
Result MCUHWC_Get3dSliderLevel(u8 *level);

/**
 * @brief Gets the major MCU firmware version
 * @param out Pointer to write the major firmware version to.
 */
Result MCUHWC_GetFwVerHigh(u8 *out);

/**
 * @brief Gets the minor MCU firmware version
 * @param out Pointer to write the minor firmware version to.
 */
Result MCUHWC_GetFwVerLow(u8 *out);
