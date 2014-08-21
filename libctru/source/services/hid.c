#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/HID.h>
#include <3ds/srv.h>
#include <3ds/svc.h>

Handle hidHandle;
Handle hidMemHandle;

vu32* hidSharedMem;

static u32 kOld, kHeld, kDown, kUp;
static touchPosition tPos;
static circlePosition cPos;

Result hidInit(u32* sharedMem)
{
	if(!sharedMem)sharedMem=(u32*)HID_SHAREDMEM_DEFAULT;
	Result ret=0;

	if((ret=srvGetServiceHandle(&hidHandle, "hid:USER")))return ret;
	
	if((ret=HIDUSER_GetInfo(NULL, &hidMemHandle)))return ret;
	hidSharedMem=sharedMem;
	svcMapMemoryBlock(hidMemHandle, (u32)hidSharedMem, MEMPERM_READ, 0x10000000);

	if((ret=HIDUSER_EnableAccelerometer(NULL)))return ret;

	kOld = kHeld = kDown = kUp = 0;

	return 0;
}

void hidExit()
{
	svcUnmapMemoryBlock(hidMemHandle, (u32)hidSharedMem);
	svcCloseHandle(hidMemHandle);
	svcCloseHandle(hidHandle);
}

void hidScanInput()
{
	kOld = kHeld;

	int padId = hidSharedMem[4];
	kHeld = hidSharedMem[10 + padId*4];
	cPos = *(circlePosition*)&hidSharedMem[10 + padId*4 + 3];

	int touchId = hidSharedMem[42 + 4];
	tPos = *(touchPosition*)&hidSharedMem[42 + 8 + touchId*2];
	if (hidSharedMem[42 + 8 + touchId*2 + 1])
		kHeld |= KEY_TOUCH;

	kDown = (~kOld) & kHeld;
	kUp = kOld & (~kHeld);
}

u32 hidKeysHeld()
{
	return kHeld;
}

u32 hidKeysDown()
{
	return kDown;
}

u32 hidKeysUp()
{
	return kUp;
}

void hidTouchRead(touchPosition* pos)
{
	if (pos) *pos = tPos;
}

void hidCircleRead(circlePosition* pos)
{
	if (pos) *pos = cPos;
}

Result HIDUSER_GetInfo(Handle* handle, Handle* outMemHandle)
{
	if(!handle)handle=&hidHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xa0000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(outMemHandle)*outMemHandle=cmdbuf[3];

	return cmdbuf[1];
}

Result HIDUSER_EnableAccelerometer(Handle* handle)
{
	if(!handle)handle=&hidHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x110000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}
