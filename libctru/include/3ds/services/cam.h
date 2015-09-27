/**
 * @file cam.h
 * @brief CAM service for using the 3DS's front and back cameras.
 */
#pragma once
#include <3ds/services/y2r.h>
#include <3ds/types.h>

/**
 * @brief Camera connection target ports.
 */
typedef enum {
	PORT_NONE = 0x0,
	PORT_CAM1 = BIT(0),
	PORT_CAM2 = BIT(1),

	// Port combinations.
	PORT_BOTH = PORT_CAM1 | PORT_CAM2,
} CAMU_Port;

/**
 * @brief Camera combinations.
 */
typedef enum {
	SELECT_NONE = 0x0,
	SELECT_OUT1 = BIT(0),
	SELECT_IN1  = BIT(1),
	SELECT_OUT2 = BIT(2),

	// Camera combinations.
	SELECT_IN1_OUT1  = SELECT_OUT1 | SELECT_IN1,
	SELECT_OUT1_OUT2 = SELECT_OUT1 | SELECT_OUT2,
	SELECT_IN1_OUT2  = SELECT_IN1 | SELECT_OUT2,
	SELECT_ALL       = SELECT_OUT1 | SELECT_IN1 | SELECT_OUT2,
} CAMU_CameraSelect;

/**
 * @brief Camera contexts.
 */
typedef enum {
	CONTEXT_NONE = 0x0,
	CONTEXT_A    = BIT(0),
	CONTEXT_B    = BIT(1),

	// Context combinations.
	CONTEXT_BOTH = CONTEXT_A | CONTEXT_B,
} CAMU_Context;

/**
 * @brief Ways to flip the camera image.
 */
typedef enum {
	FLIP_NONE       = 0x0,
	FLIP_HORIZONTAL = 0x1,
	FLIP_VERTICAL   = 0x2,
	FLIP_REVERSE    = 0x3,
} CAMU_Flip;

/**
 * @brief Camera image resolutions.
 */
typedef enum {
	SIZE_VGA         = 0x0,
	SIZE_QVGA        = 0x1,
	SIZE_QQVGA       = 0x2,
	SIZE_CIF         = 0x3,
	SIZE_QCIF        = 0x4,
	SIZE_DS_LCD      = 0x5,
	SIZE_DS_LCDx4    = 0x6,
	SIZE_CTR_TOP_LCD = 0x7,

	// Alias for bottom screen to match top screen naming.
	SIZE_CTR_BOTTOM_LCD = SIZE_QVGA,
} CAMU_Size;

/**
 * @brief Camera capture frame rates.
 */
typedef enum {
	FRAME_RATE_15       = 0x0,
	FRAME_RATE_15_TO_5  = 0x1,
	FRAME_RATE_15_TO_2  = 0x2,
	FRAME_RATE_10       = 0x3,
	FRAME_RATE_8_5      = 0x4,
	FRAME_RATE_5        = 0x5,
	FRAME_RATE_20       = 0x6,
	FRAME_RATE_20_TO_5  = 0x7,
	FRAME_RATE_30       = 0x8,
	FRAME_RATE_30_TO_5  = 0x9,
	FRAME_RATE_15_TO_10 = 0xA,
	FRAME_RATE_20_TO_10 = 0xB,
	FRAME_RATE_30_TO_10 = 0xC,
} CAMU_FrameRate;

/**
 * @brief Camera white balance modes.
 */
typedef enum {
	WHITE_BALANCE_AUTO  = 0x0,
	WHITE_BALANCE_3200K = 0x1,
	WHITE_BALANCE_4150K = 0x2,
	WHITE_BALANCE_5200K = 0x3,
	WHITE_BALANCE_6000K = 0x4,
	WHITE_BALANCE_7000K = 0x5,
	WHITE_BALANCE_MAX   = 0x6,

	// White balance aliases.
	WHITE_BALANCE_NORMAL                  = WHITE_BALANCE_AUTO,
	WHITE_BALANCE_TUNGSTEN                = WHITE_BALANCE_3200K,
	WHITE_BALANCE_WHITE_FLUORESCENT_LIGHT = WHITE_BALANCE_4150K,
	WHITE_BALANCE_DAYLIGHT                = WHITE_BALANCE_5200K,
	WHITE_BALANCE_CLOUDY                  = WHITE_BALANCE_6000K,
	WHITE_BALANCE_HORIZON                 = WHITE_BALANCE_6000K,
	WHITE_BALANCE_SHADE                   = WHITE_BALANCE_7000K,
} CAMU_WhiteBalance;

/**
 * @brief Camera photo modes.
 */
typedef enum {
	PHOTO_MODE_NORMAL    = 0x0,
	PHOTO_MODE_PORTRAIT  = 0x1,
	PHOTO_MODE_LANDSCAPE = 0x2,
	PHOTO_MODE_NIGHTVIEW = 0x3,
	PHOTO_MODE_LETTER    = 0x4,
} CAMU_PhotoMode;

/**
 * @brief Camera special effects.
 */
typedef enum {
	EFFECT_NONE     = 0x0,
	EFFECT_MONO     = 0x1,
	EFFECT_SEPIA    = 0x2,
	EFFECT_NEGATIVE = 0x3,
	EFFECT_NEGAFILM = 0x4,
	EFFECT_SEPIA01  = 0x5,
} CAMU_Effect;

/**
 * @brief Camera contrast patterns.
 */
typedef enum {
	CONTRAST_PATTERN_01 = 0x0,
	CONTRAST_PATTERN_02 = 0x1,
	CONTRAST_PATTERN_03 = 0x2,
	CONTRAST_PATTERN_04 = 0x3,
	CONTRAST_PATTERN_05 = 0x4,
	CONTRAST_PATTERN_06 = 0x5,
	CONTRAST_PATTERN_07 = 0x6,
	CONTRAST_PATTERN_08 = 0x7,
	CONTRAST_PATTERN_09 = 0x8,
	CONTRAST_PATTERN_10 = 0x9,
	CONTRAST_PATTERN_11 = 0xA,

	// Contrast aliases.
	CONTRAST_LOW    = CONTRAST_PATTERN_05,
	CONTRAST_NORMAL = CONTRAST_PATTERN_06,
	CONTRAST_HIGH   = CONTRAST_PATTERN_07,
} CAMU_Contrast;

/**
 * @brief Camera lens correction modes.
 */
typedef enum {
	LENS_CORRECTION_OFF   = 0x0,
	LENS_CORRECTION_ON_70 = 0x1,
	LENS_CORRECTION_ON_90 = 0x2,

	// Lens correction aliases.
	LENS_CORRECTION_DARK   = LENS_CORRECTION_OFF,
	LENS_CORRECTION_NORMAL = LENS_CORRECTION_ON_70,
	LENS_CORRECTION_BRIGHT = LENS_CORRECTION_ON_90,
} CAMU_LensCorrection;

/**
 * @brief Camera image output formats.
 */
typedef enum {
	OUTPUT_YUV_422 = 0x0,
	OUTPUT_RGB_565 = 0x1,
} CAMU_OutputFormat;

/**
 * @brief Camera shutter sounds.
 */
typedef enum {
	SHUTTER_SOUND_TYPE_NORMAL    = 0x0,
	SHUTTER_SOUND_TYPE_MOVIE     = 0x1,
	SHUTTER_SOUND_TYPE_MOVIE_END = 0x2,
} CAMU_ShutterSoundType;

/**
 * @brief Image quality calibration data.
 */
typedef struct {
	s16 aeBaseTarget;   ///< Auto exposure base target brightness.
	s16 kRL;            ///< Left color correction matrix red normalization coefficient.
	s16 kGL;            ///< Left color correction matrix green normalization coefficient.
	s16 kBL;            ///< Left color correction matrix blue normalization coefficient.
	s16 ccmPosition;    ///< Color correction matrix position.
	u16 awbCcmL9Right;  ///< Right camera, left color correction matrix red/green gain.
	u16 awbCcmL9Left;   ///< Left camera, left color correction matrix red/green gain.
	u16 awbCcmL10Right; ///< Right camera, left color correction matrix blue/green gain.
	u16 awbCcmL10Left;  ///< Left camera, left color correction matrix blue/green gain.
	u16 awbX0Right;     ///< Right camera, color correction matrix position threshold.
	u16 awbX0Left;      ///< Left camera, color correction matrix position threshold.
} CAMU_ImageQualityCalibrationData;

/**
 * @brief Stereo camera calibration data.
 */
typedef struct {
	u8 isValidRotationXY;   ///< #bool Whether the X and Y rotation data is valid.
	u8 padding[3];          ///< Padding. (Aligns isValidRotationXY to 4 bytes)
	float scale;            ///< Scale to match the left camera image with the right.
	float rotationZ;        ///< Z axis rotation to match the left camera image with the right.
	float translationX;     ///< X axis translation to match the left camera image with the right.
	float translationY;     ///< Y axis translation to match the left camera image with the right.
	float rotationX;        ///< X axis rotation to match the left camera image with the right.
	float rotationY;        ///< Y axis rotation to match the left camera image with the right.
	float angleOfViewRight; ///< Right camera angle of view.
	float angleOfViewLeft;  ///< Left camera angle of view.
	float distanceToChart;  ///< Distance between cameras and measurement chart.
	float distanceCameras;  ///< Distance between left and right cameras.
	s16 imageWidth;         ///< Image width.
	s16 imageHeight;        ///< Image height.
	u8 reserved[16];        ///< Reserved for future use. (unused)
} CAMU_StereoCameraCalibrationData;

/**
 * @brief Batch camera configuration for use without a context.
 */
typedef struct {
	u8 camera;                        ///< #CAMU_CameraSelect Selected camera.
	s8 exposure;                      ///< Camera exposure.
	u8 whiteBalance;                  ///< #CAMU_WhiteBalance Camera white balance.
	s8 sharpness;                     ///< Camera sharpness.
	u8 autoExposureOn;                ///< #bool Whether to automatically determine the proper exposure.
	u8 autoWhiteBalanceOn;            ///< #bool Whether to automatically determine the white balance mode.
	u8 frameRate;                     ///< #CAMU_FrameRate Camera frame rate.
	u8 photoMode;                     ///< #CAMU_PhotoMode Camera photo mode.
	u8 contrast;                      ///< #CAMU_Contrast Camera contrast.
	u8 lensCorrection;                ///< #CAMU_LensCorrection Camera lens correction.
	u8 noiseFilterOn;                 ///< #bool Whether to enable the camera's noise filter.
	u8 padding;                       ///< Padding. (Aligns last 3 fields to 4 bytes)
	s16 autoExposureWindowX;          ///< X of the region to use for auto exposure.
	s16 autoExposureWindowY;          ///< Y of the region to use for auto exposure.
	s16 autoExposureWindowWidth;      ///< Width of the region to use for auto exposure.
	s16 autoExposureWindowHeight;     ///< Height of the region to use for auto exposure.
	s16 autoWhiteBalanceWindowX;      ///< X of the region to use for auto white balance.
	s16 autoWhiteBalanceWindowY;      ///< Y of the region to use for auto white balance.
	s16 autoWhiteBalanceWindowWidth;  ///< Width of the region to use for auto white balance.
	s16 autoWhiteBalanceWindowHeight; ///< Height of the region to use for auto white balance.
} CAMU_PackageParameterCameraSelect;

/**
 * @brief Batch camera configuration for use with a context.
 */
typedef struct {
	u8 camera;  ///< #CAMU_CameraSelect Selected camera.
	u8 context; ///< #CAMU_Context Selected context.
	u8 flip;    ///< #CAMU_Flip Camera image flip mode.
	u8 effect;  ///< #CAMU_Effect Camera image special effects.
	u8 size;    ///< #CAMU_Size Camera image resolution.
} CAMU_PackageParameterContext;

/**
 * @brief Batch camera configuration for use with a context and with detailed size information.
 */
typedef struct {
	u8 camera;  ///< #CAMU_CameraSelect Selected camera.
	u8 context; ///< #CAMU_Context Selected context.
	u8 flip;    ///< #CAMU_Flip Camera image flip mode.
	u8 effect;  ///< #CAMU_Effect Camera image special effects.
	s16 width;  ///< Image width.
	s16 height; ///< Image height.
	s16 cropX0; ///< First crop point X.
	s16 cropY0; ///< First crop point Y.
	s16 cropX1; ///< Second crop point X.
	s16 cropY1; ///< Second crop point Y.
} CAMU_PackageParameterContextDetail;

/**
 * @brief Initializes the cam service.
 *
 * This will internally get the handle of the service, and on success call CAMU_DriverInitialize.
 */
Result camInit();

/**
 * @brief Closes the cam service.
 *
 * This will internally call CAMU_DriverFinalize and close the handle of the service.
 */
Result camExit();

/// Begins capture on the specified camera port.
Result CAMU_StartCapture(CAMU_Port port);

///Terminates capture on the specified camera port.
Result CAMU_StopCapture(CAMU_Port port);

/**
 * @brief Gets whether the specified camera port is busy.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_IsBusy(bool* busy, CAMU_Port port);

///Clears the buffer and error flags of the specified camera port.
Result CAMU_ClearBuffer(CAMU_Port port);

/**
 * @brief Gets a handle to the event signaled on vsync interrupts.
 *
 * Writes the event handle to the provided output pointer.
 */
Result CAMU_GetVsyncInterruptEvent(Handle* event, CAMU_Port port);

/**
 * @brief Gets a handle to the event signaled on camera buffer errors.
 *
 * Writes the event handle to the provided output pointer.
 */
Result CAMU_GetBufferErrorInterruptEvent(Handle* event, CAMU_Port port);

/**
 * @brief Initiates the process of receiving a camera frame.
 *
 * Writes a completion event handle to the provided pointer and writes image data to the provided buffer.
 */
Result CAMU_SetReceiving(Handle* event, void* dst, CAMU_Port port, u32 imageSize, s16 transferUnit);

/**
 * @brief Gets whether the specified camera port has finished receiving image data.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_IsFinishedReceiving(bool* finishedReceiving, CAMU_Port port);

///Sets the number of lines to transfer into an image buffer.
Result CAMU_SetTransferLines(CAMU_Port port, s16 lines, s16 width, s16 height);

/**
 * @brief Gets the maximum number of lines that can be saved to an image buffer.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_GetMaxLines(s16* maxLines, s16 width, s16 height);

///Sets the number of bytes to transfer into an image buffer.
Result CAMU_SetTransferBytes(CAMU_Port port, u32 bytes, s16 width, s16 height);

/**
 * @brief Gets the number of bytes to transfer into an image buffer.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_GetTransferBytes(u32* transferBytes, CAMU_Port port);

/**
 * @brief Gets the maximum number of bytes that can be saved to an image buffer.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_GetMaxBytes(u32* maxBytes, s16 width, s16 height);

///Sets whether image trimming is enabled.
Result CAMU_SetTrimming(CAMU_Port port, bool trimming);

/**
 * @brief Gets whether image trimming is enabled.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_IsTrimming(bool* trimming, CAMU_Port port);

///Sets the parameters used for trimming images.
Result CAMU_SetTrimmingParams(CAMU_Port port, s16 xStart, s16 yStart, s16 xEnd, s16 yEnd);

/**
 * @brief Gets the parameters used for trimming images.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_GetTrimmingParams(s16* xStart, s16* yStart, s16* xEnd, s16* yEnd, CAMU_Port port);

///Sets the parameters used for trimming images, relative to the center of the image.
Result CAMU_SetTrimmingParamsCenter(CAMU_Port port, s16 trimWidth, s16 trimHeight, s16 camWidth, s16 camHeight);

///Activates the specified camera.
Result CAMU_Activate(CAMU_CameraSelect select);

///Switches the specified camera's active context.
Result CAMU_SwitchContext(CAMU_CameraSelect select, CAMU_Context context);

///Sets the exposure value of the specified camera.
Result CAMU_SetExposure(CAMU_CameraSelect select, s8 exposure);

///Sets the white balance mode of the specified camera.
Result CAMU_SetWhiteBalance(CAMU_CameraSelect select, CAMU_WhiteBalance whiteBalance);

/**
 * @brief Sets the white balance mode of the specified camera.
 *
 * TODO: Explain "without base up"?
 */
Result CAMU_SetWhiteBalanceWithoutBaseUp(CAMU_CameraSelect select, CAMU_WhiteBalance whiteBalance);

///Sets the sharpness of the specified camera.
Result CAMU_SetSharpness(CAMU_CameraSelect select, s8 sharpness);

///Sets whether auto exposure is enabled on the specified camera.
Result CAMU_SetAutoExposure(CAMU_CameraSelect select, bool autoExposure);

/**
 * @brief Gets whether auto exposure is enabled on the specified camera.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_IsAutoExposure(bool* autoExposure, CAMU_CameraSelect select);

///Sets whether auto white balance is enabled on the specified camera.
Result CAMU_SetAutoWhiteBalance(CAMU_CameraSelect select, bool autoWhiteBalance);

/**
 * @brief Gets whether auto white balance is enabled on the specified camera.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_IsAutoWhiteBalance(bool* autoWhiteBalance, CAMU_CameraSelect select);

///Flips the image of the specified camera in the specified context.
Result CAMU_FlipImage(CAMU_CameraSelect select, CAMU_Flip flip, CAMU_Context context);

///Sets the image resolution of the given camera in the given context, in detail.
Result CAMU_SetDetailSize(CAMU_CameraSelect select, s16 width, s16 height, s16 cropX0, s16 cropY0, s16 cropX1, s16 cropY1, CAMU_Context context);

///Sets the image resolution of the given camera in the given context.
Result CAMU_SetSize(CAMU_CameraSelect select, CAMU_Size size, CAMU_Context context);

///Sets the frame rate of the given camera.
Result CAMU_SetFrameRate(CAMU_CameraSelect select, CAMU_FrameRate frameRate);

///Sets the photo mode of the given camera.
Result CAMU_SetPhotoMode(CAMU_CameraSelect select, CAMU_PhotoMode photoMode);

///Sets the special effects of the given camera in the given context.
Result CAMU_SetEffect(CAMU_CameraSelect select, CAMU_Effect effect, CAMU_Context context);

///Sets the contrast mode of the given camera.
Result CAMU_SetContrast(CAMU_CameraSelect select, CAMU_Contrast contrast);

///Sets the lens correction mode of the given camera.
Result CAMU_SetLensCorrection(CAMU_CameraSelect select, CAMU_LensCorrection lensCorrection);

///Sets the output format of the given camera in the given context.
Result CAMU_SetOutputFormat(CAMU_CameraSelect select, CAMU_OutputFormat format, CAMU_Context context);

///Sets the region to base auto exposure off of for the specified camera.
Result CAMU_SetAutoExposureWindow(CAMU_CameraSelect select, s16 x, s16 y, s16 width, s16 height);

///Sets the region to base auto white balance off of for the specified camera.
Result CAMU_SetAutoWhiteBalanceWindow(CAMU_CameraSelect select, s16 x, s16 y, s16 width, s16 height);

///Sets whether the specified camera's noise filter is enabled.
Result CAMU_SetNoiseFilter(CAMU_CameraSelect select, bool noiseFilter);

///Synchronizes the specified cameras' vsync timing.
Result CAMU_SynchronizeVsyncTiming(CAMU_CameraSelect select1, CAMU_CameraSelect select2);

/**
 * @brief Gets the vsync timing record of the specified camera for the specified number of signals.
 *
 * Writes the result to the provided output pointer, which should be of size "past * sizeof(s64)".
 */
Result CAMU_GetLatestVsyncTiming(s64* timing, CAMU_Port port, u32 past);

/**
 * @brief Gets the specified camera's stereo camera calibration data.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_GetStereoCameraCalibrationData(CAMU_StereoCameraCalibrationData* data);

///Sets the specified camera's stereo camera calibration data.
Result CAMU_SetStereoCameraCalibrationData(CAMU_StereoCameraCalibrationData data);

/**
 * @brief Writes to the specified I2C register of the specified camera.
 *
 * Use with caution.
 */
Result CAMU_WriteRegisterI2c(CAMU_CameraSelect select, u16 addr, u16 data);

/**
 * @brief Writes to the specified MCU variable of the specified camera.
 *
 * Use with caution.
 */
Result CAMU_WriteMcuVariableI2c(CAMU_CameraSelect select, u16 addr, u16 data);

/**
 * @brief Reads the specified I2C register of the specified camera.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_ReadRegisterI2cExclusive(u16* data, CAMU_CameraSelect select, u16 addr);

/**
 * @brief Reads the specified MCU variable of the specified camera.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_ReadMcuVariableI2cExclusive(u16* data, CAMU_CameraSelect select, u16 addr);

/**
 * @brief Sets the specified camera's image quality calibration data.
 */
Result CAMU_SetImageQualityCalibrationData(CAMU_ImageQualityCalibrationData data);

/**
 * @brief Gets the specified camera's image quality calibration data.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_GetImageQualityCalibrationData(CAMU_ImageQualityCalibrationData* data);

///Configures a camera with pre-packaged configuration data without a context.
Result CAMU_SetPackageParameterWithoutContext(CAMU_PackageParameterCameraSelect param);

///Configures a camera with pre-packaged configuration data with a context.
Result CAMU_SetPackageParameterWithContext(CAMU_PackageParameterContext param);

///Configures a camera with pre-packaged configuration data without a context and extra resolution details.
Result CAMU_SetPackageParameterWithContextDetail(CAMU_PackageParameterContextDetail param);

///Gets the Y2R coefficient applied to image data by the camera.
Result CAMU_GetSuitableY2rStandardCoefficient(Y2R_StandardCoefficient* coefficient);

///Plays the specified shutter sound.
Result CAMU_PlayShutterSound(CAMU_ShutterSoundType sound);

///Initializes the camera driver.
Result CAMU_DriverInitialize();

///Finalizes the camera driver.
Result CAMU_DriverFinalize();

/**
 * @brief Gets the current activated camera.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_GetActivatedCamera(CAMU_CameraSelect* select);

/**
 * @brief Gets the current sleep camera.
 *
 * Writes the result to the provided output pointer.
 */
Result CAMU_GetSleepCamera(CAMU_CameraSelect* select);

///Sets the current sleep camera.
Result CAMU_SetSleepCamera(CAMU_CameraSelect select);

///Sets whether to enable synchronization of left and right camera brightnesses.
Result CAMU_SetBrightnessSynchronization(bool brightnessSynchronization);

