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
static accelVector aVec;
static angularRate gRate;


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

u32 hidCheckSectionUpdateTime(vu32 *sharedmem_section, u32 id)
{
	s64 tick0=0, tick1=0;

	if(id==0)
	{
		tick0 = *((u64*)&sharedmem_section[0]);
		tick1 = *((u64*)&sharedmem_section[2]);

		if(tick0==tick1 || tick0<0 || tick1<0)return 1;
	}

	return 0;
}

void hidScanInput()
{
	u32 Id=0;

	kOld = kHeld;

	kHeld = 0;
	memset(&cPos, 0, sizeof(circlePosition));
	memset(&tPos, 0, sizeof(touchPosition));
	memset(&aVec, 0, sizeof(accelVector));
	memset(&gRate, 0, sizeof(angularRate));

	Id = hidSharedMem[4];//PAD / circle-pad
	if(Id>7)Id=7;
	if(hidCheckSectionUpdateTime(hidSharedMem, Id)==0)
	{
		kHeld = hidSharedMem[10 + Id*4];
		cPos = *(circlePosition*)&hidSharedMem[10 + Id*4 + 3];
	}

	Id = hidSharedMem[42 + 4];//Touch-screen
	if(Id>7)Id=7;
	if(hidCheckSectionUpdateTime(&hidSharedMem[42], Id)==0)
	{
		tPos = *(touchPosition*)&hidSharedMem[42 + 8 + Id*2];
		if (hidSharedMem[42 + 8 + Id*2 + 1])
			kHeld |= KEY_TOUCH;
	}

	kDown = (~kOld) & kHeld;
	kUp = kOld & (~kHeld);

	Id = hidSharedMem[66 + 4];//Accelerometer
	if(Id>7)Id=7;
	if(hidCheckSectionUpdateTime(&hidSharedMem[66], Id)==0)
	{
		aVec = *(accelVector*)&hidSharedMem[66 + 8 + Id*2];
	}

	Id = hidSharedMem[86 + 4];//Gyroscope
	if(Id>31)Id=31;
	if(hidCheckSectionUpdateTime(&hidSharedMem[86], Id)==0)
	{
		gRate = *(angularRate*)&hidSharedMem[86 + 8 + Id*2];
	}
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

void hidAccelRead(accelVector* vector)
{
	if (vector) *vector = aVec;
}

void hidGyroRead(angularRate* rate)
{
	if (rate) *rate = gRate;
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

