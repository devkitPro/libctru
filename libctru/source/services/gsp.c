/*
  gsp.c _ Gpu/lcd stuff.
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/gsp.h>

#define GSP_EVENT_STACK_SIZE 0x1000

Handle gspGpuHandle;
Handle gspLcdHandle;
Handle gspEvents[GSPEVENT_MAX];
vu32 gspEventCounts[GSPEVENT_MAX];
u64 gspEventStack[GSP_EVENT_STACK_SIZE/sizeof(u64)]; //u64 so that it's 8-byte aligned
volatile bool gspRunEvents;
Handle gspEventThread;

static Handle gspEvent;
static int gspRefCount, gspLcdRefCount;
static vu8* gspEventData;

static void gspEventThreadMain(void *arg);

Result gspInit(void)
{
	Result res=0;
	if (AtomicPostIncrement(&gspRefCount)) return 0;
	res = srvGetServiceHandle(&gspGpuHandle, "gsp::Gpu");
	if (R_FAILED(res)) AtomicDecrement(&gspRefCount);
	return res;
}

void gspExit(void)
{
	if (AtomicDecrement(&gspRefCount)) return;
	svcCloseHandle(gspGpuHandle);
}

Result gspInitEventHandler(Handle _gspEvent, vu8* _gspSharedMem, u8 gspThreadId)
{
	// Create events
	int i;
	for (i = 0; i < GSPEVENT_MAX; i ++)
	{
		Result rc = svcCreateEvent(&gspEvents[i], 0);
		if (rc != 0)
		{
			// Destroy already created events due to failure
			int j;
			for (j = 0; j < i; j ++)
				svcCloseHandle(gspEvents[j]);
			return rc;
		}
	}

	// Start event thread
	gspEvent = _gspEvent;
	gspEventData = _gspSharedMem + gspThreadId*0x40;
	gspRunEvents = true;
	return svcCreateThread(&gspEventThread, gspEventThreadMain, 0x0, (u32*)((char*)gspEventStack + sizeof(gspEventStack)), 0x31, 0xfffffffe);
}

void gspExitEventHandler(void)
{
	// Stop event thread
	gspRunEvents = false;
	svcWaitSynchronization(gspEventThread, 1000000000);
	svcCloseHandle(gspEventThread);

	// Free events
	int i;
	for (i = 0; i < GSPEVENT_MAX; i ++)
		svcCloseHandle(gspEvents[i]);
}

void gspWaitForEvent(GSP_Event id, bool nextEvent)
{
	if(id>=GSPEVENT_MAX)return;

	if (nextEvent)
		svcClearEvent(gspEvents[id]);
	svcWaitSynchronization(gspEvents[id], U64_MAX);
	if (!nextEvent)
		svcClearEvent(gspEvents[id]);
}

static int popInterrupt()
{
	int curEvt;
	bool strexFailed;
	do {
		union {
			struct {
				u8 cur;
				u8 count;
				u8 err;
				u8 unused;
			};
			u32 as_u32;
		} header;

		// Do a load on all header fields as an atomic unit
		header.as_u32 = __ldrex((s32*)gspEventData);

		if (__builtin_expect(header.count == 0, 0)) {
			__clrex();
			return -1;
		}

		curEvt = gspEventData[0xC + header.cur];

		header.cur += 1;
		if (header.cur >= 0x34) header.cur -= 0x34;
		header.count -= 1;
		header.err = 0; // Should this really be set?

		strexFailed = __strex((s32*)gspEventData, header.as_u32);
	} while (__builtin_expect(strexFailed, 0));

	return curEvt;
}

void gspEventThreadMain(void *arg)
{
	while (gspRunEvents)
	{
		svcWaitSynchronization(gspEvent, U64_MAX);
		svcClearEvent(gspEvent);

		while (true)
		{
			int curEvt = popInterrupt();

			if (curEvt == -1)
				break;

			if (curEvt < GSPEVENT_MAX) {
				svcSignalEvent(gspEvents[curEvt]);
				gspEventCounts[curEvt]++;
			}
		}
	}
	svcExitThread();
}

Result GSPGPU_WriteHWRegs(u32 regAddr, u32* data, u8 size)
{
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00010082; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[3]=(size<<14)|2;
	cmdbuf[4]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_WriteHWRegsWithMask(u32 regAddr, u32* data, u8 datasize, u32* maskdata, u8 masksize)
{
	if(datasize>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00020084; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=datasize;
	cmdbuf[3]=(datasize<<14)|2;
	cmdbuf[4]=(u32)data;
	cmdbuf[5]=(masksize<<14)|0x402;
	cmdbuf[6]=(u32)maskdata;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReadHWRegs(u32 regAddr, u32* data, u8 size)
{
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00040080; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[0x40]=(size<<14)|2;
	cmdbuf[0x40+1]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetBufferSwap(u32 screenid, GSP_FramebufferInfo *framebufinfo)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00050200;
	cmdbuf[1] = screenid;
	memcpy(&cmdbuf[2], framebufinfo, sizeof(GSP_FramebufferInfo));

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_FlushDataCache(const void* adr, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00080082; //request header code
	cmdbuf[1]=(u32)adr;
	cmdbuf[2]=size;
	cmdbuf[3]=0x0;
	cmdbuf[4]=CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_InvalidateDataCache(const void* adr, u32 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00090082;
	cmdbuf[1] = (u32)adr;
	cmdbuf[2] = size;
	cmdbuf[3] = 0;
	cmdbuf[4] = CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetLcdForceBlack(u8 flags)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x000B0040; //request header code
	cmdbuf[1]=flags;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_TriggerCmdReqQueue(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x000C0000; //request header code

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RegisterInterruptRelayQueue(Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00130042; //request header code
	cmdbuf[1]=flags;
	cmdbuf[2]=0x0;
	cmdbuf[3]=eventHandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	if(threadID)*threadID=cmdbuf[2];
	if(outMemHandle)*outMemHandle=cmdbuf[4];

	return cmdbuf[1];
}

Result GSPGPU_UnregisterInterruptRelayQueue(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00140000; //request header code

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_AcquireRight(u8 flags)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x160042; //request header code
	cmdbuf[1]=flags;
	cmdbuf[2]=0x0;
	cmdbuf[3]=CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReleaseRight(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x170000; //request header code

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ImportDisplayCaptureInfo(GSP_CaptureInfo *captureinfo)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00180000; //request header code

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
		memcpy(captureinfo, &cmdbuf[2], 0x20);

	return ret;
}

Result GSPGPU_SaveVramSysArea(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00190000; //request header code

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RestoreVramSysArea(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x001A0000; //request header code

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

//essentially : get commandIndex and totalCommands, calculate offset of new command, copy command and update totalCommands
//use LDREX/STREX because this data may also be accessed by the GSP module and we don't want to break stuff
//(mostly, we could overwrite the buffer header with wrong data and make the GSP module reexecute old commands)
Result GSPGPU_SubmitGxCommand(u32* sharedGspCmdBuf, u32 gxCommand[0x8])
{
	if(!sharedGspCmdBuf || !gxCommand)return -1;

	u32 cmdBufHeader = __ldrex((s32*)sharedGspCmdBuf);

	u8 commandIndex=cmdBufHeader&0xFF;
	u8 totalCommands=(cmdBufHeader>>8)&0xFF;

	if(totalCommands>=15)return -2;

	u8 nextCmd=(commandIndex+totalCommands)%15; //there are 15 command slots
	u32* dst=&sharedGspCmdBuf[8*(1+nextCmd)];
	memcpy(dst, gxCommand, 0x20);

	__dsb();
	totalCommands++;
	cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|(((u32)totalCommands)<<8);

	while(1)
	{
		if (!__strex((s32*)sharedGspCmdBuf, cmdBufHeader)) break;

		cmdBufHeader = __ldrex((s32*)sharedGspCmdBuf);
		totalCommands=((cmdBufHeader&0xFF00)>>8)+1;
		cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|((totalCommands<<8)&0xFF00);
	}

	if(totalCommands==1)return GSPGPU_TriggerCmdReqQueue();
	return 0;
}

Result gspLcdInit(void)
{
	Result res=0;
	if (AtomicPostIncrement(&gspLcdRefCount)) return 0;
	res = srvGetServiceHandle(&gspLcdHandle, "gsp::Lcd");
	if (R_FAILED(res)) AtomicDecrement(&gspLcdRefCount);
	return res;
}

void gspLcdExit(void)
{
	if (AtomicDecrement(&gspLcdRefCount)) return;
	svcCloseHandle(gspLcdHandle);
}

Result GSPLCD_PowerOffBacklight(GSPLCD_Screens screen)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00120040;
	cmdbuf[1] = screen;

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle)))return ret;

	return cmdbuf[1];
}

Result GSPLCD_PowerOnBacklight(GSPLCD_Screens screen)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00110040;
	cmdbuf[1] = screen;

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle)))return ret;

	return cmdbuf[1];
}
