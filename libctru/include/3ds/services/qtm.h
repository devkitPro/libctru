/**
 * @file qtm.h
 * @brief QTM service.
 */
#pragma once

//See also: http://3dbrew.org/wiki/QTM_Services

/**
 * @brief Head tracking coordinate pair.
 */
typedef struct {
	float x; ///< X coordinate.
	float y; ///< Y coordinate.
} qtmHeadtrackingInfoCoord;

/**
 * @brief Head tracking info.
 */
typedef struct {
	u8 flags[5];                         ///< Flags.
	u8 padding[3];                       ///< Padding.
	float floatdata_x08;                 ///< Unknown. Not used by System_Settings.
	qtmHeadtrackingInfoCoord coords0[4]; ///< Head coordinates.
	u32 unk_x2c[5];                      ///< Unknown. Not used by System_Settings.
} qtmHeadtrackingInfo;

/**
 * @brief Initializes QTM.
 */
Result qtmInit(void);

/**
 * @brief Exits QTM.
 */
void qtmExit(void);

/**
 * @brief Checks whether QTM is initialized.
 * @return Whether QTM is initialized.
 */
bool qtmCheckInitialized(void);

/**
 * @brief Gets the current head tracking info.
 * @param val Normally 0.
 * @param out Pointer to write head tracking info to.
 */
Result qtmGetHeadtrackingInfo(u64 val, qtmHeadtrackingInfo *out);

/**
 * @brief Checks whether a head is fully detected.
 * @param info Tracking info to check.
 */
bool qtmCheckHeadFullyDetected(qtmHeadtrackingInfo *info);

/**
 * @brief Converts QTM coordinates to screen coordinates.
 * @param coord Coordinates to convert.
 * @param screen_width Width of the screen. Can be NULL to use the default value for the top screen.
 * @param screen_height Height of the screen. Can be NULL to use the default value for the top screen.
 * @param x Pointer to output the screen X coordinate to.
 * @param y Pointer to output the screen Y coordinate to.
 */
Result qtmConvertCoordToScreen(qtmHeadtrackingInfoCoord *coord, float *screen_width, float *screen_height, u32 *x, u32 *y);

