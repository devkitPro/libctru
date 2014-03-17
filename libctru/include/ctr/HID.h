#ifndef HID_H
#define HID_H

#define HID_SHAREDMEM_DEFAULT (0x10000000)

#define CPAD_X(v) ((s16)((v)&0xFFFF))
#define CPAD_Y(v) ((s16)(((v>>16))&0xFFFF))

#define TOUCH_X(v) ((u16)((v)&0xFFFF))
#define TOUCH_Y(v) ((u16)(((v>>16))&0xFFFF))

typedef enum
{
	PAD_A = (1<<0),
	PAD_B = (1<<1),
	PAD_SELECT = (1<<2),
	PAD_START = (1<<3),
	PAD_RIGHT = (1<<4),
	PAD_LEFT = (1<<5),
	PAD_UP = (1<<6),
	PAD_DOWN = (1<<7),
	PAD_R = (1<<8),
	PAD_L = (1<<9),
	PAD_X = (1<<10),
	PAD_Y = (1<<11)
}PAD_KEY;

extern Handle hidMemHandle;
extern vu32* hidSharedMem;

Result hidInit(u32* sharedMem);
void hidExit();

Result HIDUSER_GetInfo(Handle* handle, Handle* outMemHandle);
Result HIDUSER_EnableAccelerometer(Handle* handle);

#endif
