/**
 * @file hid.h
 * @brief HID service.
 */
#pragma once

//See also: http://3dbrew.org/wiki/HID_Services http://3dbrew.org/wiki/HID_Shared_Memory

/// Key values.
enum
{
	KEY_A       = BIT(0),       ///< A
	KEY_B       = BIT(1),       ///< B
	KEY_SELECT  = BIT(2),       ///< Select
	KEY_START   = BIT(3),       ///< Start
	KEY_DRIGHT  = BIT(4),       ///< D-Pad Right
	KEY_DLEFT   = BIT(5),       ///< D-Pad Left
	KEY_DUP     = BIT(6),       ///< D-Pad Up
	KEY_DDOWN   = BIT(7),       ///< D-Pad Down
	KEY_R       = BIT(8),       ///< R
	KEY_L       = BIT(9),       ///< L
	KEY_X       = BIT(10),      ///< X
	KEY_Y       = BIT(11),      ///< Y
	KEY_ZL      = BIT(14),      ///< ZL (New 3DS only)
	KEY_ZR      = BIT(15),      ///< ZR (New 3DS only)
	KEY_TOUCH   = BIT(20),      ///< Touch (Not actually provided by HID)
	KEY_CSTICK_RIGHT = BIT(24), ///< C-Stick Right (New 3DS only)
	KEY_CSTICK_LEFT  = BIT(25), ///< C-Stick Left (New 3DS only)
	KEY_CSTICK_UP    = BIT(26), ///< C-Stick Up (New 3DS only)
	KEY_CSTICK_DOWN  = BIT(27), ///< C-Stick Down (New 3DS only)
	KEY_CPAD_RIGHT = BIT(28),   ///< Circle Pad Right
	KEY_CPAD_LEFT  = BIT(29),   ///< Circle Pad Left
	KEY_CPAD_UP    = BIT(30),   ///< Circle Pad Up
	KEY_CPAD_DOWN  = BIT(31),   ///< Circle Pad Down

	// Generic catch-all directions
	KEY_UP    = KEY_DUP    | KEY_CPAD_UP,    ///< D-Pad Up or Circle Pad Up
	KEY_DOWN  = KEY_DDOWN  | KEY_CPAD_DOWN,  ///< D-Pad Down or Circle Pad Down
	KEY_LEFT  = KEY_DLEFT  | KEY_CPAD_LEFT,  ///< D-Pad Left or Circle Pad Left
	KEY_RIGHT = KEY_DRIGHT | KEY_CPAD_RIGHT, ///< D-Pad Right or Circle Pad Right
};

/// Touch position.
typedef struct
{
	u16 px; ///< Touch X
	u16 py; ///< Touch Y
} touchPosition;

/// Circle Pad position.
typedef struct
{
	s16 dx; ///< Pad X
	s16 dy; ///< Pad Y
} circlePosition;

/// Accelerometer vector.
typedef struct
{
	s16 x; ///< Accelerometer X
	s16 y; ///< Accelerometer Y
	s16 z; ///< Accelerometer Z
} accelVector;

/// Gyroscope angular rate.
typedef struct
{
	s16 x; ///< Roll
	s16 z; ///< Yaw
	s16 y; ///< Pitch
} angularRate;

/// HID events.
typedef enum
{
	HIDEVENT_PAD0 = 0, ///< Event signaled by HID-module, when the sharedmem+0(PAD/circle-pad)/+0xA8(touch-screen) region was updated.
	HIDEVENT_PAD1,     ///< Event signaled by HID-module, when the sharedmem+0(PAD/circle-pad)/+0xA8(touch-screen) region was updated.
	HIDEVENT_Accel,    ///< Event signaled by HID-module, when the sharedmem accelerometer state was updated.
	HIDEVENT_Gyro,     ///< Event signaled by HID-module, when the sharedmem gyroscope state was updated.
	HIDEVENT_DebugPad, ///< Event signaled by HID-module, when the sharedmem DebugPad state was updated.

	HIDEVENT_MAX,      ///< Used to know how many events there are.
} HID_Event;

extern Handle hidMemHandle; ///< HID shared memory handle.
extern vu32* hidSharedMem; ///< HID shared memory.

/// Initializes HID.
Result hidInit(void);

/// Exits HID.
void hidExit(void);

/**
 * @brief Sets the key repeat parameters for @ref hidKeysRepeat.
 * @param delay Initial delay.
 * @param interval Repeat interval.
 */
void hidSetRepeatParameters(u32 delay, u32 interval);

/// Scans HID for input data.
void hidScanInput(void);

/**
 * @brief Returns a bitmask of held buttons.
 * Individual buttons can be extracted using binary AND.
 * @return 32-bit bitmask of held buttons (1+ frames).
 */
u32 hidKeysHeld(void);

/**
 * @brief Returns a bitmask of newly pressed buttons, this frame.
 * Individual buttons can be extracted using binary AND.
 * @return 32-bit bitmask of newly pressed buttons.
 */
u32 hidKeysDown(void);

/**
 * @brief Returns a bitmask of newly pressed or repeated buttons, this frame.
 * Individual buttons can be extracted using binary AND.
 * @return 32-bit bitmask of newly pressed or repeated buttons.
 */
u32 hidKeysDownRepeat(void);

/**
 * @brief Returns a bitmask of newly released buttons, this frame.
 * Individual buttons can be extracted using binary AND.
 * @return 32-bit bitmask of newly released buttons.
 */
u32 hidKeysUp(void);

/**
 * @brief Reads the current touch position.
 * @param pos Pointer to output the touch position to.
 */
void hidTouchRead(touchPosition* pos);

/**
 * @brief Reads the current circle pad position.
 * @param pos Pointer to output the circle pad position to.
 */
void hidCircleRead(circlePosition* pos);

/**
 * @brief Reads the current accelerometer data.
 * @param vector Pointer to output the accelerometer data to.
 */
void hidAccelRead(accelVector* vector);

/**
 * @brief Reads the current gyroscope data.
 * @param rate Pointer to output the gyroscope data to.
 */
void hidGyroRead(angularRate* rate);

/**
 * @brief Waits for an HID event.
 * @param id ID of the event.
 * @param nextEvent Whether to discard the current event and wait for the next event.
 */
void hidWaitForEvent(HID_Event id, bool nextEvent);

/**
 * @brief Waits for any HID or IRRST event.
 * @param nextEvents Whether to discard the current events and wait for the next events.
 * @param cancelEvent Optional additional handle to wait on, otherwise 0.
 * @param timeout Timeout.
 */
Result hidWaitForAnyEvent(bool nextEvents, Handle cancelEvent, s64 timeout);

/// Compatibility macro for hidScanInput.
#define scanKeys   hidScanInput
/// Compatibility macro for hidKeysHeld.
#define keysHeld   hidKeysHeld
/// Compatibility macro for hidKeysDown.
#define keysDown   hidKeysDown
/// Compatibility macro for hidKeysUp.
#define keysUp     hidKeysUp
/// Compatibility macro for hidTouchRead.
#define touchRead  hidTouchRead
/// Compatibility macro for hidCircleRead.
#define circleRead hidCircleRead

/**
 * @brief Gets the handles for HID operation.
 * @param outMemHandle Pointer to output the shared memory handle to.
 * @param eventpad0 Pointer to output the pad 0 event handle to.
 * @param eventpad1 Pointer to output the pad 1 event handle to.
 * @param eventaccel Pointer to output the accelerometer event handle to.
 * @param eventgyro Pointer to output the gyroscope event handle to.
 * @param eventdebugpad Pointer to output the debug pad event handle to.
 */
Result HIDUSER_GetHandles(Handle* outMemHandle, Handle *eventpad0, Handle *eventpad1, Handle *eventaccel, Handle *eventgyro, Handle *eventdebugpad);

/// Enables the accelerometer.
Result HIDUSER_EnableAccelerometer(void);

/// Disables the accelerometer.
Result HIDUSER_DisableAccelerometer(void);

/// Enables the gyroscope.
Result HIDUSER_EnableGyroscope(void);

/// Disables the gyroscope.
Result HIDUSER_DisableGyroscope(void);

/**
 * @brief Gets the gyroscope raw to dps coefficient.
 * @param coeff Pointer to output the coefficient to.
 */
Result HIDUSER_GetGyroscopeRawToDpsCoefficient(float *coeff);

/**
 * @brief Gets the current volume slider value. (0-63)
 * @param volume Pointer to write the volume slider value to.
 */
Result HIDUSER_GetSoundVolume(u8 *volume);
