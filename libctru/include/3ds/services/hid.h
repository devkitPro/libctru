#pragma once

//See also: http://3dbrew.org/wiki/HID_Services http://3dbrew.org/wiki/HID_Shared_Memory

#define HID_SHAREDMEM_DEFAULT (0x10000000)

typedef enum
{
	KEY_A       = BIT(0),
	KEY_B       = BIT(1),
	KEY_SELECT  = BIT(2),
	KEY_START   = BIT(3),
	KEY_DRIGHT  = BIT(4),
	KEY_DLEFT   = BIT(5),
	KEY_DUP     = BIT(6),
	KEY_DDOWN   = BIT(7),
	KEY_R       = BIT(8),
	KEY_L       = BIT(9),
	KEY_X       = BIT(10),
	KEY_Y       = BIT(11),
	KEY_ZL      = BIT(14), // (new 3DS only)
	KEY_ZR      = BIT(15), // (new 3DS only)
	KEY_TOUCH   = BIT(20), // Not actually provided by HID
	KEY_CSTICK_RIGHT = BIT(24), // c-stick (new 3DS only)
	KEY_CSTICK_LEFT  = BIT(25), // c-stick (new 3DS only)
	KEY_CSTICK_UP    = BIT(26), // c-stick (new 3DS only)
	KEY_CSTICK_DOWN  = BIT(27), // c-stick (new 3DS only)
	KEY_CPAD_RIGHT = BIT(28), // circle pad
	KEY_CPAD_LEFT  = BIT(29), // circle pad
	KEY_CPAD_UP    = BIT(30), // circle pad
	KEY_CPAD_DOWN  = BIT(31), // circle pad

	// Generic catch-all directions
	KEY_UP    = KEY_DUP    | KEY_CPAD_UP,
	KEY_DOWN  = KEY_DDOWN  | KEY_CPAD_DOWN,
	KEY_LEFT  = KEY_DLEFT  | KEY_CPAD_LEFT,
	KEY_RIGHT = KEY_DRIGHT | KEY_CPAD_RIGHT,
} PAD_KEY;

typedef struct
{
	u16 px, py;
} touchPosition;

typedef struct
{
	s16 dx, dy;
} circlePosition;

typedef struct
{
	s16 x;
	s16 y;
	s16 z;
} accelVector;

typedef struct
{
	s16 x;//roll
	s16 z;//yaw
	s16 y;//pitch
} angularRate;

typedef enum
{
	HIDEVENT_PAD0 = 0, //"Event signaled by HID-module, when the sharedmem+0(PAD/circle-pad)/+0xA8(touch-screen) region was updated."
	HIDEVENT_PAD1, //"Event signaled by HID-module, when the sharedmem+0(PAD/circle-pad)/+0xA8(touch-screen) region was updated."
	HIDEVENT_Accel, //"Event signaled by HID-module, when the sharedmem accelerometer state was updated."
	HIDEVENT_Gyro, //"Event signaled by HID-module, when the sharedmem gyroscope state was updated."
	HIDEVENT_DebugPad, //"Event signaled by HID-module, when the sharedmem DebugPad state was updated."

	HIDEVENT_MAX, // used to know how many events there are
} HID_Event;

extern Handle hidMemHandle;
extern vu32* hidSharedMem;

Result hidInit(u32* sharedMem);
void hidExit();

void hidScanInput();
u32 hidKeysHeld();
u32 hidKeysDown();
u32 hidKeysUp();
void hidTouchRead(touchPosition* pos);
void hidCircleRead(circlePosition* pos);
void hidAccelRead(accelVector* vector);
void hidGyroRead(angularRate* rate);

void hidWaitForEvent(HID_Event id, bool nextEvent);

// libnds compatibility defines
#define scanKeys   hidScanInput
#define keysHeld   hidKeysHeld
#define keysDown   hidKeysDown
#define keysUp     hidKeysUp
#define touchRead  hidTouchRead
#define circleRead hidCircleRead

Result HIDUSER_GetHandles(Handle* outMemHandle, Handle *eventpad0, Handle *eventpad1, Handle *eventaccel, Handle *eventgyro, Handle *eventdebugpad);
Result HIDUSER_EnableAccelerometer();
Result HIDUSER_DisableAccelerometer();
Result HIDUSER_EnableGyroscope();
Result HIDUSER_DisableGyroscope();
Result HIDUSER_GetGyroscopeRawToDpsCoefficient(float *coeff);
Result HIDUSER_GetSoundVolume(u8 *volume); //Return the volume slider value (0-63)
