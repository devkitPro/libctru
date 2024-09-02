/**
 * @file qtm.h
 * @brief QTM services.
 *
 * QTM is responsible for the following:
 *      - tracking and predicting the position of the user's eyes. This is done by using the inner
 *        camera sampling at 320x240px at 30 FPS, and by using the gyroscope to predict the position
 *        of the eyes between two camera samples (effectively doubling the tracking rate).
 *        The API reporting eye tracking data is actually *console-agnostic*. This concept is most
 *        likely covered by patent US9098112B2
 *      - automatically managing the IR LED emitter used for eye tracking in low-light conditions
 *      - managing the state of the parallax barrier by controlling the positions of the barrier's
 *        mask (opaque/transparent state by repeating pattern of 12 units, plus polarity). This is
 *        done via a TI TCA6416A I2C->Parallel expander (highlighted in yellow in this teardown photo:
 *        https://guide-images.cdn.ifixit.com/igi/IKlW6WTZKKmapYkt.full, with the expected 12 traces
 *        being clearly visible near the ribbon cable slot)
 *      - updating the barrier position according to eye tracking data relative to an optimal setting
 *        stored in calibration data: this is what Nintendo calls "Super Stable 3D" (SS3D); not done
 *        when in 2D mode
 *
 * Head-tracking can be used even if SS3D is disabled.
 *
 * SS3D works as follows:
 *      - compute the raw X and Y pixel coordinates for the eye midpoint:
 *            `rawX = (leftEyeRawPxCoords.x + rightEyeRawPxCoords.x) / 2`
 *            `rawY = (leftEyeRawPxCoords.y + rightEyeRawPxCoords.y) / 2`
 *      - rotate the value around the optical Z-axis using the optimal eye-to-camera angle from
 *        calibration data, with a rotation matrix
 *      - normalize the X value:
 *            `xC = (rawX / 320) - 0.5`
 *      - transform into world space coordinate, using fovX from calibration (convert to radians first).
 *        Note that this fovX doesn't take lens distortion into account and is slightly different from
 *        the hardcoded angle used in \ref QTMU_GetTrackingData
 *            `x = xC * tan(fovX/2)`
 *      - multiply by length of adjacent side (eye-to-camera distance) to get the length of the opposite
 *        side. This is the eye horizontal deviation to camera lens in mm, which is then converted to
 *        number of iod/12 units:
 *            `delta = x * optimalDistance / (iod/12)`
 *      - we then obtain the new target position of the parallax barrier (expressed in iod/12 units,
 *        mod iod/12):
 *            `pos = centerPos + delta`
 *       - the value is then rounded to nearest integer. To avoid artifacts, if the rounded value is
 *         going to increase, then 0.01 is subtracted, and if it is going to decrease, then 0.01 is added.
 *         The value is rounded again to compute the final discrete value written to the expander (barrier
 *         position).
 *       - note: all calculation in QTM and otherwise assume interocular distance to be 62mm (the average).
 *         Assumedly, if the user's IOD is different, then "optimal distance to screen" effectively changes
 *         for that user.
 *
 * QTM services are not present on O3DS, thus caller must call \ref qtmCheckServicesRegistered to check
 * if the services are registered. Moreover, since QTM functionality might not always be available (due
 * to blacklist or console being N2DSXL), `qtm:u` users should check Result return values, and `qtm:s`
 * users can call \ref QTMS_SetQtmStatus to check the actual availability status.
 *
 * Considering that the eye tracking data reporting API is hardware-agnostic, it is advisable to
 * hardcode neither camera aspect ratio (even if it is 4/3 on real hardware) and resolution nor
 * field-of-view angle.
 *
 * There is a separate QTM service, `qtm:c` ("hardware check"), that lets you manipulate parallax barrier
 * pattern directly. It is covered in \ref qtmc.h instead.
 */
#pragma once

#include <3ds/types.h>

/// ID of QTM status data (fully enabled/SS3D disabled) in `cfg`
#define QTM_STATUS_CFG_BLK_ID   0x180000u

/// ID of QTM calibration data in `cfg`
#define QTM_CAL_CFG_BLK_ID      0x180001u

/**
 * @brief QTM enablement status (when cameras not in use by user), set by `qtm:s`.
 * @note  Manual IR LED control, camera lux, and `qtm:c` commands remain available
 *        for use on N3DS and N3DSXL regardless.
 */
typedef enum QtmStatus {
	QTM_STATUS_ENABLED          = 0, ///< QTM is fully enabled.
	QTM_STATUS_SS3D_DISABLED    = 1, ///< QTM "super stable 3D" feature is disabled. Parallax barrier hardware state is configured to match O3DS.

	/**
	 * @brief QTM is unavailable: either "blacklisted" (usually by NS) for the current title, **or console is a N2DSXL**.
	 *
	 *        In this state, all QTM functionality is disabled. This includes "super-stable 3D"
	 *        (ie. auto barrier adjustment) including `qtm:s` manual barrier position setting functions,
	 *        head tracking, IR LED control and camera luminance reporting (400.0 is returned instead).
	 *
	 * @note  `qtm:c` barrier hardware state setting function (\ref blah) bypasses this state.
	 * @note  Due to an oversight, \ref QTMS_SetQtmStatus allows changing QTM state on N2DSXL. This is not intended
	 *        to be done, and is in fact never done by official software.
	 */
	QTM_STATUS_UNAVAILABLE      = 2,
} QtmStatus;

/// QTM status data (fully enabled/SS3D disabled) in `cfg`. Usually all-zero on N2DSXL.
typedef struct QtmStatusCfgData {
	QtmStatus defaultStats; ///< QTM status at boot (fully enabled or SS3D disabled).
	/**
	 * @brief "Global variable" (.data) section load mode? Unused.
	 *        From CTRAging:
	 *            - 0: "normal"
	 *            - 1: "single reacq"
	 *            - 2: "double reacq"
	              - 3/4/5: "w2w copy 1/10/100"
	 */
	u8 gvLoadMode;
	u8 _padding[2];         ///< Padding.
} QtmStatusCfgData;

/// QTM calibration data (fully enabled/SS3D disabled) in `cfg`. Usually all-zero on N2DSXL.
typedef struct QtmCalibrationData {
	/**
	 * @brief Neutral (center) barrier position/offset (with slit width of 6 units), when the user is
	 *        facing directly facing the camera, that is to say, their eye midpoint normalized X coord
	 *        in the camera's plane is 0, assuming the user's head is located at the expected viewing distance
	 *        and at the expected eye-to-camera angle (as per the rest of this structure).
	 *        This is expressed in terms of iod/12 units modulo iod/12 (thus, range is 0 to 11 included),
	 *        with IOD (interocular distance) assumed to be 62mm.
	 * @note  This field is floating-point for QTM auto-adjustment purposes, however the actual barrier
	 *        position in hardware is an integer.
	 * @note  This is the field that System Settings lets you add -1.0 to +1.0 to.
	 * @note  Moreover, this field can be directly changed through \ref QTMS_SetCenterBarrierPosition.
	 */
	float centerBarrierPosition;

	float translationX;     ///< Lens X coord in inner camera space? Very low value and seems to be unused.
	float translationY;     ///< Lens Y coord in inner camera space? Very low value and seems to be unused.
	float rotationZ;        ///< Optimal eye-to-camera angle, in radians, without accounting for lens distortion.
	float fovX;             ///< Camera's horizontal FoV in degrees, without accounting for lens distortion.
	float viewingDistance;  ///< Optimal viewing distance between user and top screen, assuming iod to be 62mm.
} QtmCalibrationData;

/// Left eye or right eye, for \ref QtmTrackingData and \ref QtmRawTrackingData
typedef enum QtmEyeSide {
	QTM_EYE_LEFT    = 0,    ///< Left eye.
	QTM_EYE_RIGHT   = 1,    ///< Right eye.

	QTM_EYE_NUM,            ///< Number of eyes.
} QtmEyeSide;

/// QTM raw eye tracking data
typedef struct QtmRawTrackingData {
	/**
	 * @brief Eye position detected or predicted, equals (confidenceLevel > 0).
	 *        If false, QTM will attempt to make a guess based on gyro data.
	 *        If the console isn't moving either, then QTM will assume the user's eyes are progressively
	 *        moving back to face the screen.
	 */
	bool eyesTracked;       ///< Eye position detected or predicted, equals (confidenceLevel > 0).
	u8 _padding[3];         ///< Padding.
	u32 singletonQtmPtr;    ///< Pointer to eye-tracking singleton pointer, in QTM's .bss, located in N3DS extra memory.
	float confidenceLevel;  ///< Eye tracking confidence level (0 to 1).

	/**
	 * @brief Raw predicted or detected eye coordinates. Each eye is represented as one point.
	 *        Fractional part is *not* necessarily zero.
	 * @note  X coord is within 0 to 320.
	 * @note  Y coord is within 0 to 240.
	 */
	float rawEyeCameraCoordinates[QTM_EYE_NUM][2];

	float dPitch;           ///< Difference in gyro pitch from position at console boot.
	float dYaw;             ///< Difference in gyro yaw from position at console boot.
	float dRoll;            ///< Difference in gyro roll from position at console boot.

	s64   samplingTick;     ///< Time point the current measurements were made.
} QtmRawTrackingData;

/// QTM processed eye tracking data, suitable for 3D programming
typedef struct QtmTrackingData {
	bool eyesTracked;       ///< Eye position detected or tracked with some confidence, equals (confidenceLevel > 0). Even if false, QTM may make a guess
	bool faceDetected;      ///< Whether or not the entirety of the user's face has been detected with good confidence.
	bool eyesDetected;      ///< Whether or not the user's eyes have actually been detected with full confidence.
	u8 _unused;             ///< Unused.
	bool clamped;           ///< Whether or not the normalized eye coordinates have been clamped after accounting for lens distortion.
	u8 _padding[3];         ///< Padding.

	float confidenceLevel;  ///< Eye tracking confidence level (0 to 1).

	/**
	 * @brief Normalized eye coordinates, for each eye, after accounting for lens distortion, centered around camera.
	 *        X coord is in the -1 to 1 range, and Y coord range depends on inverse aspect ratio (-0.75 to 0.75 on real hardware).
	 * @note  On real hardware, X coord equals `((rawX / 160.0) - 1.00) * 1.0639` before clamping.
	 * @note  On real hardware, Y coord equals `((rawY / 160.0) - 0.75) * 1.0637` before clamping.
	 */
	float eyeCameraCoordinates[QTM_EYE_NUM][2];

	/**
	 * @brief Normalized eye coordinates, for each eye, in world space.
	 *        Corresponds to \ref eyeCameraCoordinates multiplied by tangent of field of view.
	 * @note  On real hardware, X coord equals `eyeCameraCoordinates.x * tan(64.9 deg / 2)`.
	 * @note  On real hardware, Y coord equals `eyeCameraCoordinates.x * tan(51.0 deg / 2)`.
	 */
	float eyeWorldCoordinates[QTM_EYE_NUM][2];

	float dPitch;           ///< Difference in gyro pitch from position at console boot.
	float dYaw;             ///< Difference in gyro yaw from position at console boot.
	float dRoll;            ///< Difference in gyro roll from position at console boot.


	s64   samplingTick;     ///< Time point the current measurements were made.
} QtmTrackingData;

/// QTM service name enum, excluding `qtm:c`
typedef enum QtmServiceName {
	/**
	 * @brief `qtm:u`: has eye-tracking commands and IR LED control commands, but for some
	 *        reason cannot fetch ambiant lux data from the camera's luminosity sensor.
	 */
	QTM_SERVICE_USER            = 0,

	/**
	 * @brief `qtm:s`: has access to all `qtm:u` commands, plus luminosity sensor, plus
	 *        manual barrier position setting and calibration adjustment commands.
	 *        Automatic barrier control is reenabled on session exit.
	 */
	QTM_SERVICE_SYSTEM          = 1,

	/**
	 * @brief `qtm:sp`: has access to all `qtm:s` (and `qtm:u`) commands, and merely has a
	 *         few more commands that GSP uses to notify QTM of 2D<>3D mode switches and
	 *         power events. Automatic barrier control is reenabled on session exit.
	 *         GSP always keeps a `qtm:sp` sessions open (at least on latest system version),
	 *         whereas NS opens then immediately closes a `qtm:sp` sessions only when dealing
	 *         with a "blacklisted" application (that is, almost never).
	 */
	QTM_SERVICE_SYSTEM_PROCESS  = 2,
} QtmServiceName;

/**
 * @brief  Check whether or not QTM services are registered.
 * @return True on O3DS systems, false on N3DS systems.
 */
bool qtmCheckServicesRegistered(void);

/**
 * @brief Initializes QTM (except `qtm:c`).
 *        Excluding `qtm:c`, QTM has three main services.
 *        Only 3 sessions (2 until 9.3.0 sysupdate) for ALL services COMBINED, including `qtm:c`,
 *        can be open at a time.
 *        Refer to \ref QtmServiceName enum value descriptions to see which service to choose.
 *
 * @param serviceName QTM service name enum value (corresponding to `qtm:u`, `qtm:s` and `qtm:sp`
 *                    respectively).
 * @note  Result of \ref qtmCheckServicesRegistered should be checked before calling this function.
 */
Result qtmInit(QtmServiceName serviceName);

/// Exits QTM.
void qtmExit(void);

/// Checks whether or not a `qtm:u`, `qtm:s` or `qtm:sp` session is active.
bool qtmIsInitialized(void);

/// Returns a pointer to the current `qtm:u` / `qtm:s` / `qtm:sp` session handle.
Handle *qtmGetSessionHandle(void);

/**
 * @brief  Gets the current raw eye tracking data, with an optional prediction made for predictionTimePointOrZero = t+dt,
 *         or for the current time point (QTM makes predictions based on gyro data since inner camera runs at 30 FPS).
 *
 * @param[out] outData Where to write the raw tracking data to. Cleared to all-zero on failure (instead of being left uninitialized).
 * @param predictionTimePointOrZero Either zero, or the time point (in system ticks) for which to make a prediction for.
 *                                  Maximum 1 frame (at 30 FPS) in the past, and up to 5 frames in the future.
 * @return `0xC8A18008` if camera is in use by user, or `0xC8A183EF` if QTM is unavailable (in particular, QTM is always
 *         unavailable on N2DSXL), Otherwise, 0 (success). Return value should be checked by caller.
 * @note   Consider using \ref QTMU_GetTrackingDataEx instead.
 */
Result QTMU_GetRawTrackingDataEx(QtmRawTrackingData *outData, s64 predictionTimePointOrZero);

/**
 * @brief  Gets the current raw eye tracking data.
 *
 * @param[out] outData Where to write the raw tracking data to. Cleared to all-zero on failure (instead of being left uninitialized).
 * @return `0xC8A18008` if camera is in use by user, or `0xC8A183EF` if QTM is unavailable (in particular, QTM is always
 *         unavailable on N2DSXL), Otherwise, 0 (success). Return value should be checked by caller.
 * @note   Consider using \ref QTMU_GetTrackingData instead.
 */
static inline Result QTMU_GetRawTrackingData(QtmRawTrackingData *outData)
{
	return QTMU_GetRawTrackingDataEx(outData, 0LL);
}

/**
 * @brief  Gets the current normalized eye tracking data, made suitable for 3D programming with an optional prediction made
 *         for predictionTimePointOrZero = t+dt, or for the current time point (QTM makes predictions based on gyro data since
 *         inner camera runs at 30 FPS).
 *
 * @param[out] outData Where to write the raw tracking data to. Cleared to all-zero on failure (instead of being left uninitialized).
 * @param predictionTimePointOrZero Either zero, or the time point (in system ticks) for which to make a prediction for.
 *                                  Maximum 1 frame (at 30 FPS) in the past, and up to 5 frames in the future.
 * @return `0xC8A18008` if camera is in use by user, or `0xC8A183EF` if QTM is unavailable (in particular, QTM is always
 *         unavailable on N2DSXL). Otherwise, 0 (success). Return value should be checked by caller.
 * @note   This can, for example, be used in games to allow the user to control the scene's camera with their own face.
 */
Result QTMU_GetTrackingDataEx(QtmTrackingData *outData, s64 predictionTimePointOrZero);

/**
 * @brief  Gets the current normalized eye tracking data, made suitable for 3D programming.
 *
 * @param[out] outData Where to write the raw tracking data to. Cleared to all-zero on failure (instead of being left uninitialized).
 * @return `0xC8A18008` if camera is in use by user, or `0xC8A183EF` if QTM is unavailable (in particular, QTM is always
 *         unavailable on N2DSXL). Otherwise, 0 (success). Return value should be checked by caller.
 * @note   This can, for example, be used in games to allow the user to control the scene's camera with their own face.
 */
static inline Result QTMU_GetTrackingData(QtmTrackingData *outData)
{
	return QTMU_GetTrackingDataEx(outData, 0LL);
}

/**
 * @brief Computes an approximation of the horizontal angular field of view of the camera based on eye tracking data.
 *
 * @param data Eye tracking data, obtained from \ref QTMU_GetTrackingData or \ref QTMU_GetTrackingDataEx.
 * @return Horizontal angular field of view in radians. Corresponds to 64.9 degrees on real hardware.
 */
float qtmComputeFovX(const QtmTrackingData *data);

/**
 * @brief Computes an approximation of the vertical angular field of view of the camera based on eye tracking data.
 *
 * @param data Eye tracking data, obtained from \ref QTMU_GetTrackingData or \ref QTMU_GetTrackingDataEx.
 * @return Vertical angular field of view in radians. Corresponds to 51.0 degrees on real hardware.
 */
float qtmComputeFovY(const QtmTrackingData *data);

/**
 * @brief Computes a rough approximation of the inverse of the aspect ration of the camera based on eye tracking data.
 *
 * @param data Eye tracking data, obtained from \ref QTMU_GetTrackingData or \ref QTMU_GetTrackingDataEx.
 * @return Rough approximation of the inverse of the aspect ratio of the camera. Aspect ratio is exactly 0.75 on real hardware.
 */
float qtmComputeInverseAspectRatio(const QtmTrackingData *data);

/**
 * @brief  Computes the user's head tilt angle, that is, the angle between the line through both eyes and the camera's
 *         horizontal axis in camera space.
 *
 * @param data Eye tracking data, obtained from \ref QTMU_GetTrackingData or \ref QTMU_GetTrackingDataEx.
 * @return Horizontal head angle relative to camera, in radians.
 */
float qtmComputeHeadTiltAngle(const QtmTrackingData *data);

/**
 * @brief  Estimates the distance between the user's eyes and the camera, based on
 *         eye tracking data. This may be a little bit inaccurate, as this assumes
 *         interocular distance of 62mm (like all 3DS software does), and that both
 *         eyes are at the same distance from the screen.
 *
 * @param data Eye tracking data, obtained from \ref QTMU_GetTrackingData or \ref QTMU_GetTrackingDataEx.
 * @return Eye-to-camera distance in millimeters.
 */
float qtmEstimateEyeToCameraDistance(const QtmTrackingData *data);

/**
 * @brief  Temporarily enables manual control of the IR LED by user, disabling its automatic control.
 *         If not already done, this also turns off the IR LED. This setting is cleared when user closes the console's shell.
 * @return Always 0 (success).
 */
Result QTMU_EnableManualIrLedControl(void);

/**
 * @brief  Temporarily disables manual control of the IR LED by user, re-enabling its automatic control.
 *         If not already done, this also turns off the IR LED.
 * @return Always 0 (success).
 */
Result QTMU_DisableManualIrLedControl(void);

/**
 * @brief  Turns the IR LED on or off during manual control. \ref QTMU_EnableManualIrLedControl must have been called.
 *
 * @param on Whether to turn the IR LED on or off.
 * @return `0xC8A18005` if manual control was not enabled or if the operation failed, `0xC8A18008` if camera is in use
 *         by user, or `0xC8A18009` if QTM is unavailable (in particular, QTM is always unavailable on N2DSXL).
 *         Otherwise, 0 (success).
 */
Result QTMU_SetIrLedStatus(bool on);

/**
 * @brief  Attempts to clear IR LED overrides from any of the relevant commands in `qtm:u`, `qtm:s` (and `qtm:c`) commands
 *         by calling \ref QTMU_EnableManualIrLedControl followed by \ref QTMU_DisableManualIrLedControl, so that auto IR LED
 *         management takes place again.
 * @return The value returned by \ref QTMU_DisableManualIrLedControl.
 */
Result qtmClearIrLedOverrides(void);

/**
 * @brief  Checks whether or not QTM has been blacklisted, ie. that it has been made unavailable.
 *         In detail, this means that the last call to \ref QTMS_SetQtmStatus was made with argument \ref QTM_STATUS_UNAVAILABLE,
 *         usually by NS. This feature seems to only be used for some internal test titles.
 *
 * @param[out] outBlacklisted Whether or not QTM is unavailable. Always true on N2DSXL.
 * @return Always 0 (success).
 * @note   On N2DSXL, even though status is always supposed to be \ref QTM_STATUS_UNAVAILABLE, this function often returns true
 *         (because NS doesn't change QTM's status if title isn't blacklisted). Do not rely on this for N2DSXL detection.
 * @note   Refer to https://www.3dbrew.org/wiki/NS_CFA for a list of title UIDs this is used for.
 */
Result QTMU_IsCurrentAppBlacklisted(bool *outBlacklisted);

/**
 * @brief  Sets the neutral (center) barrier position/offset in calibration, _without_ saving it to `cfg`.
 *         Takes effect immediately. SS3D works by calculating the position of the eye midpoint, rotated
 *         by the ideal eye-to-camera angle, expressed in (iod/12 units, iod assumed to be 62mm).
 *
 * @param position Center barrier position, in terms of iod/12 units modulo iod/12.
 * @note   This field is floating-point for QTM auto-adjustment purposes, however the actual barrier position
 *         in hardware is an integer.
 * @note   This is the field that System Settings lets you add -1.0 to +1.0 to.
 * @note   There is no "get" counterpart for this.
 * @return `0xC8A18009` if QTM is unavailable (in particular, QTM is always unavailable on N2DSXL), otherwise
           0 (success).
 */
Result QTMS_SetCenterBarrierPosition(float position);

/**
 * @brief  Gets the average ambient luminance as perceived by the inner camera (in lux).
 *         If QTM is unavailable (in particular, QTM is always unavailable on N2DSXL), returns 400.0 instead
 *         of the actual luminance.
 *
 * @param[out] outLuminanceLux Where to write the luminance to. Always 400.0 on N2DSXL.
 * @note   Camera exposure, and in particular auto-exposure affects the returned luminance value. This must be
 *         taken into consideration, because this value can thus surge when user covers the inner camera.
 * @return Always 0 (success).
 */
Result QTMS_GetCameraLuminance(float *outLuminanceLux);

/**
 * @brief  Enables automatic barrier control when in 3D mode with "super stable 3D" enabled.
 *
 * @note   This is automatically called upon `qtm:s` and `qtm:sp` session exit.
 * @return `0xC8A18009` if QTM is unavailable (in particular, QTM is always unavailable on N2DSXL), otherwise
           0 (success).
 * @note   Due to an oversight, \ref QTMS_SetQtmStatus allows changing QTM state on N2DSXL. This is not intended
 *         to be done, and is in fact never done by official software. If that is regardless the case,
 *         this function here does nothing.
 */
Result QTMS_EnableAutoBarrierControl(void);

/**
 * @brief  Temporarily disables automatic barrier control (when in 3D mode with "super stable 3D" enabled).
 *
 * @note   This is automatically called upon `qtm:s` and `qtm:sp` session exit.
 * @return `0xC8A18009` if QTM is unavailable (in particular, QTM is always unavailable on N2DSXL), otherwise
           0 (success).
 * @note   Due to an oversight, \ref QTMS_SetQtmStatus allows changing QTM state on N2DSXL. This is not intended
 *         to be done, and is in fact never done by official software. If that is regardless the case,
 *         this function here does nothing.
 */
Result QTMS_DisableAutoBarrierControl(void);

/**
 * @brief  Temporarily sets the parallax barrier's position (offset in iod/12 units, assuming slit width of 6 units).
 *         Does nothing in 2D mode and/or if "super stable 3D" is disabled.
 *
 * @param position Parallax barrier position (offset in units), must be between 0 and 11 (both included)
 * @return `0xC8A18009` if QTM is unavailable (in particular, QTM is always unavailable on N2DSXL), 0xE0E18002
 *         if \p position is not in range, otherwise 0 (success).
 * @note   Due to an oversight, \ref QTMS_SetQtmStatus allows changing QTM state on N2DSXL. This is not intended
 *         to be done, and is in fact never done by official software. If that is regardless the case,
 *         this function here does nothing.
 * @note   No effect when the screen is in 2D mode.
 * @see    QTMC_SetBarrierPattern
 */
Result QTMS_SetBarrierPosition(u8 position);

/**
 * @brief  Gets the current position of the parallax barrier (offset in iod/12 units, slit width of 6 units).
 *         When "super stable 3D" is disabled, returns 13 instead.
 *
 * @param[out] outPosition Where to write the barrier's position to.
 * @return `0xC8A18009` if QTM is unavailable (in particular, QTM is always unavailable on N2DSXL), otherwise
           0 (success).
 * @note   When SS3D is disabled, this returns 13 to \p outPosition . When in 2D mode, the returned position is not
           updated.
 * @note   Due to an oversight, \ref QTMS_SetQtmStatus allows changing QTM state on N2DSXL. This is not intended
 *         to be done, and is in fact never done by official software. If that is regardless the case,
 *         this function here returns 13 to \p outPosition .
 * @see    QTMC_SetBarrierPattern
 */
Result QTMS_GetCurrentBarrierPosition(u8 *outPosition);

/**
 * @brief  Temporarily overrides IR LED state. Requires "manual control" from `qtm:u` to be disabled, and has
 *         lower priority than it.
 *
 * @param on Whether to turn the IR LED on or off.
 * @return `0xC8A18005` if manual control was enabled or if the operation failed, `0xC8A18008` if camera is in use
 *         by user (unless "hardware check" API enabled), or `0xC8A18009` if QTM is unavailable (in particular,
 *         QTM is always unavailable on N2DSXL). Otherwise, 0 (success).
 */
Result QTMS_SetIrLedStatusOverride(bool on);

/**
 * @brief  Sets calibration data, taking effect immediately, and optionally saves it to `cfg`.
 *
 * @param cal Pointer to calibration data.
 * @param saveCalToCfg Whether or not to persist the calibration data in `cfg`.
 * @return `0xC8A18009` if QTM is unavailable (in particular, QTM is always unavailable on N2DSXL), otherwise
           whatever `cfg:s` commands return (if used), or 0 (success).
 * @note   There is no "get" counterpart for this function, and there is no way to see the current calibration data
           in use unless it has been saved to `cfg`.
 * @note   Due to an oversight, \ref QTMS_SetQtmStatus allows changing QTM state on N2DSXL. This is not intended
 *         to be done, and is in fact never done by official software. If that is regardless the case,
 *         this function here doesn't apply calibrations parameters (they may still be saved, however,
 *         even though QTM calibration blocks are always normally 0 on N2DSXL).
 */
Result QTMS_SetCalibrationData(const QtmCalibrationData *cal, bool saveCalToCfg);

/**
 * @brief  Gets the current QTM status (enabled/ss3d disabled/unavailable).
 *
 * @param[out] outQtmStatus Where to write the QTM status to.
 * @return Always 0.
 */
Result QTMS_GetQtmStatus(QtmStatus *outQtmStatus);

/**
 * @brief   Gets the current QTM status (enabled/ss3d disabled/unavailable). Also sets or clear the
 *          "blacklisted" flag returned by \ref QTMU_IsCurrentAppBlacklisted.
 *
 * @param qtmStatus QTM status to set. If equal to \ref QTM_STATUS_UNAVAILABLE, sets the "blacklisted" flag,
 *                  otherwise clears it.
 * @return  `0xE0E18002` if enum value is invalid, otherwise 0 (success).
 * @note    System settings uses this to disable super-stable 3D, and NS to "blacklist" (make QTM unavailable)
 *          specific applications.
 */
Result QTMS_SetQtmStatus(QtmStatus qtmStatus);

/**
 * @brief  Called by GSP's LCD driver to signal 2D<>3D mode change
 * @param newMode 0 for 2D, 1 for 800px 2D (unused for this function, same as 0), 2 for 3D
 * @return Always 0 (success).
 */
Result QTMSP_NotifyTopLcdModeChange(u8 newMode);

/**
 * @brief  Called by GSP's LCD driver during top LCD power-on to signal to QTM that it may power on
 *         and/or reconfigure then use the TI TCA6416A expander. In the process, QTM re-creates its
 *         expander thread.
 * @return Always 0 (success).
 */
Result QTMSP_NotifyTopLcdPowerOn(void);

/**
 * @brief  Called by GSP's LCD driver to know whether or not QTM's expander thread is using
 *         the TI TCA6416A expander; it is waiting for this to become true/false during LCD
 *         power on/power off to proceed. Always false on N2DSXL.
 * @param[out] outActive Where to write the "in use" status to.
 * @return  Always 0 (success).
 */
Result QTMSP_IsExpanderInUse(bool *outActive);

/**
 * @brief  Called by GSP's LCD driver during top LCD power-on to signal to QTM that it needs to
 *         switch the parallax barrier state to a 2D state (all-transparent mask). Causes QTM's
 *         expander thread to exit, relinquishing its `i2c::QTM` session with it.
 * @return Always 0 (success).
 */
Result QTMSP_NotifyTopLcdPowerOff(void);
