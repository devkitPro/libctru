/*
  gsp.c _ Gpu/lcd stuff.
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/gsp.h>

#define GSP_EVENT_STACK_SIZE 0x1000

Handle gspGpuHandle=0;
Handle gspLcdHandle=0;
Handle gspEvents[GSPEVENT_MAX];
vu32 gspEventCounts[GSPEVENT_MAX];
u64 gspEventStack[GSP_EVENT_STACK_SIZE/sizeof(u64)]; //u64 so that it's 8-byte aligned
volatile bool gspRunEvents;
Handle gspEventThread;

static Handle gspEvent;
static vu8* gspEventData;

static void gspEventThreadMain(void *arg);


Result gspInit()
{
	return srvGetServiceHandle(&gspGpuHandle, "gsp::Gpu");
}

void gspExit()
{
	if(gspGpuHandle)svcCloseHandle(gspGpuHandle);
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

void gspExitEventHandler()
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
	u32 strexFailed;
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

		u32* gsp_header_ptr = (u32*)(gspEventData + 0);

		// Do a load on all header fields as an atomic unit
		__asm__ volatile (
				"ldrex %[result], %[addr]" :
				[result]"=r"(header.as_u32) :
				[addr]"Q"(*gsp_header_ptr));

		if (__builtin_expect(header.count == 0, 0)) {
			__asm__ volatile ("clrex");
			return -1;
		}

		curEvt = gspEventData[0xC + header.cur];

		header.cur += 1;
		if (header.cur >= 0x34) header.cur -= 0x34;
		header.count -= 1;
		header.err = 0; // Should this really be set?

		__asm__ volatile (
				"strex %[result], %[val], %[addr]" :
				[result]"=&r"(strexFailed), [addr]"=Q"(*gsp_header_ptr) :
				[val]"r"(header.as_u32));
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

Result GSPGPU_WriteHWRegs(Handle* handle, u32 regAddr, u32* data, u8 size)
{
	if(!handle)handle=&gspGpuHandle;

	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00010082; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[3]=(size<<14)|2;
	cmdbuf[4]=(u32)data;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_WriteHWRegsWithMask(Handle* handle, u32 regAddr, u32* data, u8 datasize, u32* maskdata, u8 masksize)
{
	if(!handle)handle=&gspGpuHandle;

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
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReadHWRegs(Handle* handle, u32 regAddr, u32* data, u8 size)
{
	if(!handle)handle=&gspGpuHandle;

	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00040080; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[0x40]=(size<<14)|2;
	cmdbuf[0x40+1]=(u32)data;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetBufferSwap(Handle* handle, u32 screenid, GSP_FramebufferInfo *framebufinfo)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	if(!handle)handle=&gspGpuHandle;

	cmdbuf[0] = 0x00050200;
	cmdbuf[1] = screenid;
	memcpy(&cmdbuf[2], framebufinfo, sizeof(GSP_FramebufferInfo));

	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_FlushDataCache(Handle* handle, u8* adr, u32 size)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00080082; //request header code
	cmdbuf[1]=(u32)adr;
	cmdbuf[2]=size;
	cmdbuf[3]=0x0;
	cmdbuf[4]=0xffff8001;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_InvalidateDataCache(Handle* handle, u8* adr, u32 size)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	if(!handle)handle=&gspGpuHandle;

	cmdbuf[0] = 0x00090082;
	cmdbuf[1] = (u32)adr;
	cmdbuf[2] = size;
	cmdbuf[3] = 0;
	cmdbuf[4] = 0xFFFF8001;

	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetLcdForceBlack(Handle* handle, u8 flags)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x000B0040; //request header code
	cmdbuf[1]=flags;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_TriggerCmdReqQueue(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x000C0000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RegisterInterruptRelayQueue(Handle* handle, Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00130042; //request header code
	cmdbuf[1]=flags;
	cmdbuf[2]=0x0;
	cmdbuf[3]=eventHandle;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(threadID)*threadID=cmdbuf[2];
	if(outMemHandle)*outMemHandle=cmdbuf[4];

	return cmdbuf[1];
}

Result GSPGPU_UnregisterInterruptRelayQueue(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00140000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_AcquireRight(Handle* handle, u8 flags)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x160042; //request header code
	cmdbuf[1]=flags;
	cmdbuf[2]=0x0;
	cmdbuf[3]=0xffff8001;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReleaseRight(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x170000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ImportDisplayCaptureInfo(Handle* handle, GSP_CaptureInfo *captureinfo)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00180000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	ret = cmdbuf[1];

	if(ret==0)
	{
		memcpy(captureinfo, &cmdbuf[2], 0x20);
	}

	return ret;
}

Result GSPGPU_SaveVramSysArea(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00190000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RestoreVramSysArea(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x001A0000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

//essentially : get commandIndex and totalCommands, calculate offset of new command, copy command and update totalCommands
//use LDREX/STREX because this data may also be accessed by the GSP module and we don't want to break stuff
//(mostly, we could overwrite the buffer header with wrong data and make the GSP module reexecute old commands)
Result GSPGPU_SubmitGxCommand(u32* sharedGspCmdBuf, u32 gxCommand[0x8], Handle* handle)
{
	if(!sharedGspCmdBuf || !gxCommand)return -1;

	u32 cmdBufHeader;
	__asm__ __volatile__ ("ldrex %[result], [%[adr]]" : [result] "=r" (cmdBufHeader) : [adr] "r" (sharedGspCmdBuf));

	u8 commandIndex=cmdBufHeader&0xFF;
	u8 totalCommands=(cmdBufHeader>>8)&0xFF;

	if(totalCommands>=15)return -2;

	u8 nextCmd=(commandIndex+totalCommands)%15; //there are 15 command slots
	u32* dst=&sharedGspCmdBuf[8*(1+nextCmd)];
	memcpy(dst, gxCommand, 0x20);

	u32 mcrVal=0x0;
	__asm__ __volatile__ ("mcr p15, 0, %[val], c7, c10, 4" :: [val] "r" (mcrVal)); //Data Synchronization Barrier Register
	totalCommands++;
	cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|(((u32)totalCommands)<<8);

	while(1)
	{
		u32 strexResult;
		__asm__ __volatile__ ("strex %[result], %[val], [%[adr]]" : [result] "=&r" (strexResult) : [adr] "r" (sharedGspCmdBuf), [val] "r" (cmdBufHeader));
		if(!strexResult)break;

		__asm__ __volatile__ ("ldrex %[result], [%[adr]]" : [result] "=r" (cmdBufHeader) : [adr] "r" (sharedGspCmdBuf));
		totalCommands=((cmdBufHeader&0xFF00)>>8)+1;
		cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|((totalCommands<<8)&0xFF00);
	}

	if(totalCommands==1)return GSPGPU_TriggerCmdReqQueue(handle);
	return 0;
}

Result gspLcdInit()
{
	return srvGetServiceHandle(&gspLcdHandle, "gsp::Lcd");
}

void gspLcdExit()
{
	if(gspLcdHandle)svcCloseHandle(gspLcdHandle);
}

Result GSPLCD_PowerOffBacklight(GSPLCD_Screens screen)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00120040;
	cmdbuf[1] = screen;

	Result ret=0;
	if ((ret = svcSendSyncRequest(gspLcdHandle)))return ret;

	return cmdbuf[1];
}

Result GSPLCD_PowerOnBacklight(GSPLCD_Screens screen)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00110040;
	cmdbuf[1] = screen;

	Result ret=0;
	if ((ret = svcSendSyncRequest(gspLcdHandle)))return ret;

	return cmdbuf[1];
}
