/**
 * @file gsplcd.h
 * @brief GSPLCD service.
 */
#pragma once
#include <3ds/gfx.h> // For gfxScreen_t

/// LCD screens.
enum
{
	GSPLCD_SCREEN_TOP    = BIT(GFX_TOP),                             ///< Top screen.
	GSPLCD_SCREEN_BOTTOM = BIT(GFX_BOTTOM),                          ///< Bottom screen.
	GSPLCD_SCREEN_BOTH   = GSPLCD_SCREEN_TOP | GSPLCD_SCREEN_BOTTOM, ///< Both screens.
};

/// Initializes GSPLCD.
Result gspLcdInit(void);

/// Exits GSPLCD.
void gspLcdExit(void);

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
 * @brief Gets the LCD screens' vendors. Stubbed on old 3ds.
 * @param vendor Pointer to output the screen vendors to.
 */
Result GSPLCD_GetVendors(u8 *vendors);