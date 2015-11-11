/**
 * @file qtm.h
 * @brief QTM service.
 */
#pragma once

//See also: http://3dbrew.org/wiki/QTM_Services

/// Head tracking coordinate pair.
typedef struct {
	float x; ///< X coordinate.
	float y; ///< Y coordinate.
} QTM_HeadTrackingInfoCoord;

/// Head tracking info.
typedef struct {
	u8 flags[5];                         ///< Flags.
	u8 padding[3];                       ///< Padding.
	float floatdata_x08;                 ///< Unknown. Not used by System_Settings.
	QTM_HeadTrackingInfoCoord coords0[4]; ///< Head coordinates.
	u32 unk_x2c[5];                      ///< Unknown. Not used by System_Settings.
} QTM_HeadTrackingInfo;

/// Initializes QTM.
Result qtmInit(void);

/// Exits QTM.
void qtmExit(void);

/**
 * @brief Checks whether QTM is initialized.
 * @return Whether QTM is initialized.
 */
bool qtmCheckInitialized(void);

/**
 * @brief Checks whether a head is fully detected.
 * @param info Tracking info to check.
 */
bool qtmCheckHeadFullyDetected(QTM_HeadTrackingInfo *info);

/**
 * @brief Converts QTM coordinates to screen coordinates.
 * @param coord Coordinates to convert.
 * @param screen_width Width of the screen. Can be NULL to use the default value for the top screen.
 * @param screen_height Height of the screen. Can be NULL to use the default value for the top screen.
 * @param x Pointer to output the screen X coordinate to.
 * @param y Pointer to output the screen Y coordinate to.
 */
Result qtmConvertCoordToScreen(QTM_HeadTrackingInfoCoord *coord, float *screen_width, float *screen_height, u32 *x, u32 *y);

/**
 * @brief Gets the current head tracking info.
 * @param val Normally 0.
 * @param out Pointer to write head tracking info to.
 */
Result QTM_GetHeadTrackingInfo(u64 val, QTM_HeadTrackingInfo* out);
