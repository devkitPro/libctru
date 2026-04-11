/**
 * @file mcu_common.h
 * @brief Common MCU-related definitions.
 */

#pragma once

#include <3ds/types.h>

/// SRV notifications published by the MCU system module.
enum MCU_InterruptNotification
{
	MCUNOTIF_SHELL_STATE_CHANGE    = 0x200, ///< The shell state has changed.
	MCUNOTIF_POWER_BUTTON_PRESSED  = 0x202, ///< The power button was pressed.
	MCUNOTIF_POWER_BUTTON_HELD     = 0x203, ///< The power button was held.
	MCUNOTIF_HOME_BUTTON_PRESSED   = 0x204, ///< The HOME button was pressed.
	MCUNOTIF_HOME_BUTTON_RELEASED  = 0x205, ///< The HOME button was released.
	MCUNOTIF_WLAN_SWITCH_TRIGGERED = 0x206, ///< The WiFi switch was triggered.
	MCUNOTIF_FATAL_HW_ERROR        = 0x20C, ///< MCU watchdog reset occurred.
	MCUNOTIF_AC_ADAPTER_REMOVED    = 0x20D, ///< The AC adapter was disconnected.
	MCUNOTIF_AC_ADAPTER_CONNECTED  = 0x20E, ///< The AC adapter was connected.
	MCUNOTIF_STARTED_CHARGING      = 0x20F, ///< The battery started charging.
	MCUNOTIF_STOPPED_CHARGING      = 0x210, ///< The battery stopped charging.
};

/// MCU interrupts.
enum MCU_Interrupt
{
	MCUINT_POWER_BUTTON_PRESS          = BIT(0),  ///< The power button was pressed.
	MCUINT_POWER_BUTTON_HELD           = BIT(1),  ///< The power button was held.
	MCUINT_HOME_BUTTON_PRESS           = BIT(2),  ///< The HOME button was pressed.
	MCUINT_HOME_BUTTON_RELEASE         = BIT(3),  ///< The HOME button was released.
	MCUINT_WLAN_SWITCH_TRIGGER         = BIT(4),  ///< The WiFi switch was triggered.
	MCUINT_SHELL_CLOSE                 = BIT(5),  ///< The shell was closed (or sleep switch turned off).
	MCUINT_SHELL_OPEN                  = BIT(6),  ///< The shell was opened (or sleep switch turned on).
	MCUINT_FATAL_HW_ERROR              = BIT(7),  ///< MCU watchdog reset occurred.
	MCUINT_AC_ADAPTER_REMOVED          = BIT(8),  ///< The AC adapter was disconnected.
	MCUINT_AC_ADAPTER_PLUGGED_IN       = BIT(9),  ///< The AC adapter was connected.
	MCUINT_RTC_ALARM                   = BIT(10), ///< The RTC alarm time has been reached.
	MCUINT_ACCELEROMETER_I2C_MANUAL_IO = BIT(11), ///< The manual accelerometer I²C read/write operation has completed.
	MCUINT_ACCELEROMETER_NEW_SAMPLE    = BIT(12), ///< A new accelerometer sample is available.
	MCUINT_CRITICAL_BATTERY            = BIT(13), ///< The battery is critically low.
	MCUINT_CHARGING_STOP               = BIT(14), ///< The battery stopped charging.
	MCUINT_CHARGING_START              = BIT(15), ///< The battery started charging.
	
	MCUINT_VOL_SLIDER                  = BIT(22), ///< The position of the volume slider changed.
	
	MCUINT_LCD_OFF                     = BIT(24), ///< The LCDs turned off.
	MCUINT_LCD_ON                      = BIT(25), ///< The LCDs turned on.
	MCUINT_BOT_BACKLIGHT_OFF           = BIT(26), ///< The bottom screen backlight turned off.
	MCUINT_BOT_BACKLIGHT_ON            = BIT(27), ///< The bottom screen backlight turned on.
	MCUINT_TOP_BACKLIGHT_OFF           = BIT(28), ///< The top screen backlight turned off.
	MCUINT_TOP_BACKLIGHT_ON            = BIT(29), ///< The top screen backlight turned on.
};

/// MCU register IDs.
enum MCU_RegisterId
{
	MCUREG_VERSION_HIGH                 = 0x0,  ///< Major version of the MCU firmware.
	MCUREG_VERSION_LOW                  = 0x1,  ///< Minor version of the MCU firmware.
	
	MCUREG_RESET_EVENTS                 = 0x2,  ///< @ref MCU_ResetEventFlags
	
	MCUREG_VCOM_TOP                     = 0x3,  ///< Flicker/VCOM value for the top screen.
	MCUREG_VCOM_BOTTOM                  = 0x4,  ///< Flicker/VCOM value for the bottom screen.
	
	MCUREG_FIRMWARE_UPLOAD_0            = 0x5,  ///< Firmware upload register. 
	MCUREG_FIRMWARE_UPLOAD_1            = 0x6,  ///< Firmware upload register.
	MCUREG_FIRMWARE_UPLOAD_2            = 0x7,  ///< Firmware upload register.
	
	MCUREG_3D_SLIDER_POSITION           = 0x8,  ///< Position of the 3D slider.
	MCUREG_VOLUME_SLIDER_POSITION       = 0x9,  ///< Position of the volume slider.
	
	MCUREG_BATTERY_PCB_TEMPERATURE      = 0xA,  ///< Temperature of the battery, measured on a sensor on the PCB.
	MCUREG_BATTERY_PERCENTAGE_INT       = 0xB,  ///< Integer part of the battery percentage.
	MCUREG_BATTERY_PERCENTAGE_FRAC      = 0xC,  ///< Fractional part of the battery percentage.
	MCUREG_BATTERY_VOLTAGE              = 0xD,  ///< Voltage of the battery, in units of 20 mV.
	
	MCUREG_POWER_STATUS                 = 0xF,  ///< @ref MCU_PowerStatusFlags
	
	MCUREG_LEGACY_VERSION_HIGH          = 0xF,  ///< (Old MCU_FIRM only) Major firmware version.
	MCUREG_LEGACY_VERSION_LOW           = 0x10, ///< (Old MCU_FIRM only) Minor firmware version.
	MCUREG_LEGACY_FIRM_UPLOAD           = 0x3B, ///< (Old MCU_FIRM only) Firmware upload register.
	
	MCUREG_RECEIVED_IRQS                = 0x10, ///< Bitmask of received IRQs. @ref MCU_Interrupt
	MCUREG_IRQ_MASK                     = 0x18, ///< Bitmask of enabled IRQs.  @ref MCU_Interrupt
	
	MCUREG_PWR_CTL                      = 0x20, ///< @ref MCU_PowerTrigger
	MCUREG_LCD_PWR_CTL                  = 0x22, ///< @ref MCU_PowerStatusFlags
	MCUREG_MCU_RESET_CTL                = 0x23, ///< Writing 'r' to this register resets the MCU. Stubbed on retail.
	MCUREG_FORCE_SHUTDOWN_DELAY         = 0x24, ///< The amount of time, in units of 0.125s, for which the power button needs to be held to trigger a hard shutdown.
	
	MCUREG_VOLUME_UNK_25                = 0x25, ///< Unknown register. Used in mcu::SND
	MCUREG_UNK_26                       = 0x26, ///< Unknown register. Used in mcu::CDC
	
	MCUREG_LED_BRIGHTNESS_STATE         = 0x28, ///< Brightness of the status LEDs.
	MCUREG_POWER_LED_STATE              = 0x29, ///< @ref MCU_PowerLedState
	MCUREG_WLAN_LED_STATE               = 0x2A, ///< Controls the WiFi LED.
	MCUREG_CAMERA_LED_STATE             = 0x2B, ///< Controls the camera LED (on models that have one).
	MCUREG_3D_LED_STATE                 = 0x2C, ///< Controls the 3D LED (on models that have one).
	MCUREG_NOTIFICATION_LED_STATE       = 0x2D, ///< @ref MCU_InfoLedPattern
	MCUREG_NOTIFICATION_LED_CYCLE_STATE = 0x2E, ///< Bit 0 is set if the info (notification) LED has started a new cycle of its pattern.
	
	MCUREG_RTC_TIME_SECOND              = 0x30, ///< RTC second.
	MCUREG_RTC_TIME_MINUTE              = 0x31, ///< RTC minute.
	MCUREG_RTC_TIME_HOUR                = 0x32, ///< RTC hour.
	MCUREG_RTC_TIME_WEEKDAY             = 0x33, ///< RTC day of the week.
	MCUREG_RTC_TIME_DAY                 = 0x34, ///< RTC day of the month.
	MCUREG_RTC_TIME_MONTH               = 0x35, ///< RTC month.
	MCUREG_RTC_TIME_YEAR                = 0x36, ///< RTC year.
	MCUREG_RTC_TIME_CORRECTION          = 0x37, ///< RTC subsecond (RSUBC) correction value.
	
	MCUREG_RTC_ALARM_MINUTE             = 0x38, ///< RTC alarm minute.
	MCUREG_RTC_ALARM_HOUR               = 0x39, ///< RTC alarm hour.
	MCUREG_RTC_ALARM_DAY                = 0x3A, ///< RTC alarm day.
	MCUREG_RTC_ALARM_MONTH              = 0x3B, ///< RTC alarm month.
	MCUREG_RTC_ALARM_YEAR               = 0x3C, ///< RTC alarm year.
	
	MCUREG_TICK_COUNTER_LSB             = 0x3D, ///< MCU tick counter value (low byte).
	MCUREG_TICK_COUNTER_MSB             = 0x3E, ///< MCU tick counter value (high byte).
	
	MCUREG_SENSOR_CONFIG                = 0x40, ///< @ref MCU_SensorConfig

	MCUREG_ACCELEROMETER_MANUAL_REGID_R = 0x41, ///< Hardware register ID for use in manual accelerometer I²C reads.
	MCUREG_ACCELEROMETER_MANUAL_REGID_W = 0x43, ///< Hardware reigster ID for use in manual accelerometer I²C writes.
	MCUREG_ACCELEROMETER_MANUAL_IO      = 0x44, ///< Data register for manual accelerometer reads/writes.
	MCUREG_ACCELEROMETER_OUTPUT_X_LSB   = 0x45, ///< Accelerometer X coordinate (low byte).
	MCUREG_ACCELEROMETER_OUTPUT_X_MSB   = 0x46, ///< Accelerometer X coordinate (high byte).
	MCUREG_ACCELEROMETER_OUTPUT_Y_LSB   = 0x47, ///< Accelerometer Y coordinate (low byte).
	MCUREG_ACCELEROMETER_OUTPUT_Y_MSB   = 0x48, ///< Accelerometer Y coordinate (high byte).
	MCUREG_ACCELEROMETER_OUTPUT_Z_LSB   = 0x49, ///< Accelerometer Z coordinate (low byte).
	MCUREG_ACCELEROMETER_OUTPUT_Z_MSB   = 0x4A, ///< Accelerometer Z coordinate (high byte).

	MCUREG_PEDOMETER_STEPS_LOWBYTE      = 0x4B, ///< Pedometer step count (low byte).
	MCUREG_PEDOMETER_STEPS_MIDDLEBYTE   = 0x4C, ///< Pedometer step count (middle byte).
	MCUREG_PEDOMETER_STEPS_HIGHBYTE     = 0x4D, ///< Pedometer step count (high byte).
	MCUREG_PEDOMETER_CNT                = 0x4E, ///< @ref MCU_PedometerControl
	MCUREG_PEDOMETER_STEP_DATA          = 0x4F, ///< @ref MCU_PedometerStepData
	MCUREG_PEDOMETER_WRAP_MINUTE        = 0x50, ///< The minute within each RTC hour at which the step history should roll into the next hour.
	MCUREG_PEDOMETER_WRAP_SECOND        = 0x51, ///< The second within each RTC hour at which the step history should roll into the next hour.
	
	MCUREG_VOLUME_CALIBRATION_MIN       = 0x58, ///< Lower bound of sound volume.
	MCUREG_VOLUME_CALIBRATION_MAX       = 0x59, ///< Upper bound of sound volume.
	
	MCUREG_STORAGE_AREA_OFFSET          = 0x60, ///< Offset within the storage area to read/write to.
	MCUREG_STORAGE_AREA                 = 0x61, ///< Input/output byte for write/read operations in the storage area.
	
	MCUREG_INFO                         = 0x7F, ///< System information register.
};

enum MCU_ResetEventFlags
{
	MCU_RESETFLG_RTC_TIME_LOST  = BIT(0), ///< RTC time was lost (as is the case when the battery is removed).
	MCU_RESETFLG_WATCHDOG_RESET = BIT(1), ///< MCU Watchdog reset occurred.
};

enum MCU_PowerStatusFlags
{
	MCU_PWRSTAT_SHELL_OPEN        = BIT(1), ///< Set if the shell is open.
	MCU_PWRSTAT_ADAPTER_CONNECTED = BIT(3), ///< Set if the AC adapter is connected.
	MCU_PWRSTAT_CHARGING          = BIT(4), ///< Set if the battery is charging.
	MCU_PWRSTAT_BOTTOM_BL_ON      = BIT(5), ///< Set if the bottom backlight is on.
	MCU_PWRSTAT_TOP_BL_ON         = BIT(6), ///< Set if the top backlight is on.
	MCU_PWRSTAT_LCD_ON            = BIT(7)  ///< Set if the LCDs are on.
};

enum MCU_PedometerControl
{
	MCU_PEDOMETER_CLEAR      = BIT(0), ///< Clear the step history.
	MCU_PEDOMETER_STEPS_FULL = BIT(4)  ///< Set when the step history is full.
};

enum MCU_PowerTrigger
{
	MCU_PWR_SHUTDOWN     = BIT(0), ///< Turn off the system.
	MCU_PWR_RESET        = BIT(1), ///< Reset the MCU.
	MCU_PWR_REBOOT       = BIT(2), ///< Reboot the system.
	MCU_PWR_LGY_SHUTDOWN = BIT(3), ///< Turn off the system. (Used by LgyBg)
	MCU_PWR_SLEEP        = BIT(4), ///< Signal to enter sleep mode.
	OLDMCU_BL_OFF        = BIT(4), ///< (Old MCU_FIRM only) turn the backlights off.
	OLDMCU_BL_ON         = BIT(5), ///< (Old MCU_FIRM only) turn the backlights on.
	OLDMCU_LCD_OFF       = BIT(6), ///< (Old MCU_FIRM only) turn the LCDs off.
	OLDMCU_LCD_ON        = BIT(7), ///< (Old MCU_FIRM only) turn the LCDs on.
};

typedef enum MCU_PowerLedState
{
	LED_NORMAL     = 0, ///< Fade power LED to blue, while checking for battery percentage.
	LED_FADE_BLUE  = 1, ///< Fade power LED to blue.
	LED_SLEEP_MODE = 2, ///< The power LED pulses blue slowly, as it does in sleep mode.
	LED_OFF        = 3, ///< Power LED fades off.
	LED_RED        = 4, ///< Power LED instantaneously turns red.
	LED_BLUE       = 5, ///< Power LED instantaneously turns blue.
	LED_BLINK_RED  = 6, ///< Power LED and info (notification) LED blink red, as they do when the battery is critically low.
} MCU_PowerLedState;

typedef struct MCU_InfoLedAnimation
{
	u8 ticksPerFrame;        ///< The amount of time (in ticks), for which a given frame is active. Must be nonzero. (See @ref mcuTicksFromMs)
	u8 frameTransitionDelay; ///< Amount of time (in ticks) between pattern frames. (See @ref mcuTicksFromMs) Set to 0 to disable interpolation, or equal to ticksPerFrame for linear interpolation.
	u8 numLastFrameRepeats;  ///< The amount of times for which the last frame should be repeated. Treated as if the pattern contains numLastFrameRepeats more of the last frame than it actually does. Set to 0xFF to repeat forever.
	u8 _pad;
} MCU_InfoLedAnimation;

typedef struct MCU_InfoLedPattern
{
	MCU_InfoLedAnimation animation; ///< Animation.
	u8 redPattern[32];              ///< Pattern for red component.
	u8 greenPattern[32];            ///< Pattern for green component.
	u8 bluePattern[32];             ///< Pattern for blue component.
} MCU_InfoLedPattern;

typedef enum MCU_WifiMode
{
	WIFI_MODE_CTR = 0, ///< 3DS WiFi mode.
	WIFI_MODE_MP  = 1, ///< DS[i] WiFi mode ("MP").
} MCU_WifiMode;

typedef struct MCU_RtcTime
{
	u8 second;   ///< RTC second.
	u8 minute;   ///< RTC minute.
	u8 hour;     ///< RTC hour.
	u8 weekday;  ///< RTC day of the week.
	u8 monthday; ///< RTC day of the month.
	u8 month;    ///< RTC month.
	u8 year;     ///< RTC year (since 2000).
} MCU_RtcTime;

typedef struct MCU_RtcAlarmTime
{
	u8 minute; ///< RTC alarm minute.
	u8 hour;   ///< RTC alarm hour.
	u8 day;    ///< RTC alarm day.
	u8 month;  ///< RTC alarm day of the month.
	u8 year;   ///< RTC alarm year (since 2000).
} CTR_PACKED MCU_RtcAlarmTime;

typedef enum MCU_SensorConfig
{
	MCU_SENSOR_ACCELEROMETER_ENABLE = BIT(0), ///< If set, the accelerometer is enabled.
	MCU_SENSOR_PEDOMETER_ENABLE     = BIT(1), ///< If set, the pedometer is enabled.
} MCU_SensorConfig;

typedef struct MCU_AccelerometerData
{
	s16 x; ///< X coordinate of the current accelerometer sample.
	s16 y; ///< Y coordinate of the current accelerometer sample.
	s16 z; ///< Z coordinate of the current accelerometer sample.
} MCU_AccelerometerData;

typedef struct MCU_PedometerStepData
{
	struct MCU_PedometerTime
	{
		u8 hour;     ///< RTC hour at the time of the update.
		u8 monthday; ///< RTC day of the month at the time of the update.
		u8 month;    ///< RTC month at the time of the update.
		u8 year;     ///< RTC year (since 2000) at the time of the update.
		u8 minute;   ///< RTC minute at the time of the update.
		u8 second;   ///< RTC second at the time of the update.
	} pedometerTime;        ///< RTC timestamp for the time this data was last updated.
	u16 stepCounts[7 * 24]; ///< Step counts for every hour in the week, with the last element referring to the current hour.
} CTR_PACKED MCU_PedometerStepData;

typedef enum MCU_AccelerometerScale
{
	ACC_SCALE_2G = 0x0, ///< -2G to 2G
	ACC_SCALE_4G = 0x1, ///< -4G to 4G
	ACC_SCALE_8G = 0x3, ///< -8G to 8G
} MCU_AccelerometerScale;

typedef struct MCU_LgyLcdSettings
{
	u8 enableAblPowersave : 1; ///< Whether or not ABL (adaptive backlight) power save is enabled.
	u8 luminanceLevel : 3;     ///< The brightness level (1-5).
	u8 _unused : 4;
} MCU_LgyLcdSettings;

static inline u8 mcuTicksFromMs(u32 ms)
{
    // 512Hz
    u32 res = 512u * ms / 1000u;
    res = res < 2 ? 1 : res - 1; // res not allowed to be zero
    res = res > 255 ? 255 : res; // res can't exceed 255
    return (u8)res;
}