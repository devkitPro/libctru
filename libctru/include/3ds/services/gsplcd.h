/**
 * @file gsplcd.h
 * @brief GSPLCD service.
 */
#pragma once
#include <3ds/types.h>
#include <3ds/services/gspgpu.h>

/// LCD screens.
enum
{
	GSPLCD_SCREEN_TOP    = BIT(GSP_SCREEN_TOP),                      ///< Top screen.
	GSPLCD_SCREEN_BOTTOM = BIT(GSP_SCREEN_BOTTOM),                   ///< Bottom screen.
	GSPLCD_SCREEN_BOTH   = GSPLCD_SCREEN_TOP | GSPLCD_SCREEN_BOTTOM, ///< Both screens.
};

/// Initializes GSPLCD.
Result gspLcdInit(void);

/// Exits GSPLCD.
void gspLcdExit(void);

/// Powers on both backlights.
Result GSPLCD_PowerOnAllBacklights(void);

/// Powers off both backlights.
Result GSPLCD_PowerOffAllBacklights(void);

/**
 * @brief Powers on the backlight.
 * @param screen Screen to power on.
 */
Result GSPLCD_PowerOnBacklight(u32 screen);

/**
 * @brief Powers off the backlight.
 * @param screen Screen to power off.
 */
Result GSPLCD_PowerOffBacklight(u32 screen);

/**
 * @brief Sets 3D_LEDSTATE to the input state value.
 * @param disable False = 3D LED enable, true = 3D LED disable.
 */
Result GSPLCD_SetLedForceOff(bool disable);

/**
 * @brief Gets the LCD screens' vendors. Stubbed on old 3ds.
 * @param vendor Pointer to output the screen vendors to.
 */
Result GSPLCD_GetVendors(u8 *vendors);

/**
 * @brief Gets the LCD screens' brightness. Stubbed on old 3ds.
 * @param screen Screen to get the brightness value of.
 * @param brightness Brightness value returned.
 */
Result GSPLCD_GetBrightness(u32 screen, u32 *brightness);

/**
 * @brief Sets the LCD screens' brightness.
 * @param screen Screen to set the brightness value of.
 * @param brightness Brightness value set.
 */
Result GSPLCD_SetBrightness(u32 screen, u32 brightness);

/**
 * @brief Sets the LCD screens' raw brightness.
 * @param screen Screen to set the brightness value of.
 * @param brightness Brightness value set.
 */
Result GSPLCD_SetBrightnessRaw(u32 screen, u32 brightness);