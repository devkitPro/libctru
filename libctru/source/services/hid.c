/*
  _hid.c - Touch screen, buttons, gyroscope, accelerometer etc.
*/
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

Handle hidHandle;
Handle hidMemHandle;

Handle hidEvents[5];

vu32* hidSharedMem;

static u32 kOld, kHeld, kDown, kUp;
static touchPosition tPos;
static circlePosition cPos;


Result hidInit(u32* sharedMem)
{
	if(!sharedMem)sharedMem=(u32*)HID_SHAREDMEM_DEFAULT;
	Result ret=0;

	// Request service.
	if((ret=srvGetServiceHandle(&hidHandle, "hid:USER")))return ret;

	// Get sharedmem handle.
	if((ret=HIDUSER_GetHandles(&hidMemHandle, &hidEvents[HIDEVENT_PAD0], &hidEvents[HIDEVENT_PAD1], &hidEvents[HIDEVENT_Accel], &hidEvents[HIDEVENT_Gyro], &hidEvents[HIDEVENT_DebugPad]))) goto cleanup1;

	// Map HID shared memory at addr "sharedMem".
	hidSharedMem=sharedMem;
	if((ret=svcMapMemoryBlock(hidMemHandle, (u32)hidSharedMem, MEMPERM_READ, 0x10000000)))goto cleanup2;

	// Reset internal state.
	kOld = kHeld = kDown = kUp = 0;
	return 0;

cleanup2:
	svcCloseHandle(hidMemHandle);
cleanup1:
	svcCloseHandle(hidHandle);
	return ret;
}

void hidExit()
{
	// Unmap HID sharedmem and close handles.
	svcUnmapMemoryBlock(hidMemHandle, (u32)hidSharedMem);
	svcCloseHandle(hidMemHandle);
	svcCloseHandle(hidHandle);
}

void hidWaitForEvent(HID_Event id, bool nextEvent)
{
	if(id>=HIDEVENT_MAX)return;

	if (nextEvent)
		svcClearEvent(hidEvents[id]);
	svcWaitSynchronization(hidEvents[id], U64_MAX);
	if (!nextEvent)
		svcClearEvent(hidEvents[id]);
}

void hidScanInput()
{
	kOld = kHeld;

	u32 padId = hidSharedMem[4];
	if(padId>7)padId=7;
	kHeld = hidSharedMem[10 + padId*4];
	cPos = *(circlePosition*)&hidSharedMem[10 + padId*4 + 3];

	u32 touchId = hidSharedMem[42 + 4];
	if(touchId>7)touchId=7;
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

Result HIDUSER_GetHandles(Handle* outMemHandle, Handle *eventpad0, Handle *eventpad1, Handle *eventaccel, Handle *eventgyro, Handle *eventdebugpad)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xa0000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(hidHandle)))return ret;

	if(outMemHandle)*outMemHandle=cmdbuf[3];

	if(eventpad0)*eventpad0=cmdbuf[4];
	if(eventpad1)*eventpad1=cmdbuf[5];
	if(eventaccel)*eventaccel=cmdbuf[6];
	if(eventgyro)*eventgyro=cmdbuf[7];
	if(eventdebugpad)*eventdebugpad=cmdbuf[8];

	return cmdbuf[1];
}

Result HIDUSER_EnableAccelerometer()
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x110000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(hidHandle)))return ret;

	return cmdbuf[1];
}

Result HIDUSER_DisableAccelerometer()
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x120000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(hidHandle)))return ret;

	return cmdbuf[1];
}

Result HIDUSER_EnableGyroscope()
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x130000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(hidHandle)))return ret;

	return cmdbuf[1];
}

Result HIDUSER_DisableGyroscope()
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x140000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(hidHandle)))return ret;

	return cmdbuf[1];
}
