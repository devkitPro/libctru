#pragma once

#define HID_SHAREDMEM_DEFAULT (0x10000000)

typedef enum
{
	KEY_A      = BIT(0),
	KEY_B      = BIT(1),
	KEY_SELECT = BIT(2),
	KEY_START  = BIT(3),
	KEY_DRIGHT = BIT(4),
	KEY_DLEFT  = BIT(5),
	KEY_DUP    = BIT(6),
	KEY_DDOWN  = BIT(7),
	KEY_R      = BIT(8),
	KEY_L      = BIT(9),
	KEY_X      = BIT(10),
	KEY_Y      = BIT(11),
	KEY_TOUCH  = BIT(20), // Not actually provided by HID
	KEY_CRIGHT = BIT(28),
	KEY_CLEFT  = BIT(29),
	KEY_CUP    = BIT(30),
	KEY_CDOWN  = BIT(31),

	// Generic catch-all directions
	KEY_UP    = KEY_DUP    | KEY_CUP,
	KEY_DOWN  = KEY_DDOWN  | KEY_CDOWN,
	KEY_LEFT  = KEY_DLEFT  | KEY_CLEFT,
	KEY_RIGHT = KEY_DRIGHT | KEY_CRIGHT,
} PAD_KEY;

typedef struct
{
	u16 px, py;
} touchPosition;

typedef struct
{
	s16 dx, dy;
} circlePosition;

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

// libnds compatibility defines
#define scanKeys   hidScanInput
#define keysHeld   hidKeysHeld
#define keysDown   hidKeysDown
#define keysUp     hidKeysUp
#define touchRead  hidTouchRead
#define circleRead hidCircleRead

Result HIDUSER_GetSharedMem(Handle* outMemHandle);
Result HIDUSER_EnableAccelerometer();
Result HIDUSER_DisableAccelerometer();
Result HIDUSER_EnableGyroscope();
Result HIDUSER_DisableGyroscope();
