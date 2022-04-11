#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/allocator/mappable.h>
#include <3ds/services/gspgpu.h>
#include <3ds/ipc.h>
#include <3ds/thread.h>

#define GSP_EVENT_STACK_SIZE 0x1000

static Handle gspGpuHandle;
static int gspRefCount;

static Handle gspSharedMemHandle;
static void* gspSharedMem;
static u8 gspThreadId;

static bool gspGpuRight;

static Handle gspEvent;
static Thread gspEventThread;
static volatile bool gspRunEvents;

static s32 gspLastEvent;
static LightEvent gspEvents[GSPGPU_EVENT_MAX];
static ThreadFunc gspEventCb[GSPGPU_EVENT_MAX];
static void* gspEventCbData[GSPGPU_EVENT_MAX];
static bool gspEventCbOneShot[GSPGPU_EVENT_MAX];

static void gspEventThreadMain(void *arg);

static inline void gspWriteGxReg(u32 offset, u32 data)
{
	GSPGPU_WriteHWRegs(0x400000 + offset, &data, 4);
}

static inline void gspWriteGxRegMasked(u32 offset, u32 data, u32 mask)
{
	GSPGPU_WriteHWRegsWithMask(0x400000 + offset, &data, 4, &mask, 4);
}

// Hardware initialization for first-time GSP users (matching official software).
static void gspHardwareInit(void)
{
	// Some GPU-internal init registers
	gspWriteGxReg(0x1000, 0);
	gspWriteGxReg(0x1080, 0x12345678);
	gspWriteGxReg(0x10C0, 0xFFFFFFF0);
	gspWriteGxReg(0x10D0, 1);
	gspWriteGxReg(0x1914, 1); // homebrew addition: make sure GPUREG_START_DRAW_FUNC0 starts off in configuration mode

	// Top screen LCD configuration, see https://www.3dbrew.org/wiki/GPU/External_Registers#LCD_Source_Framebuffer_Setup

	// Top screen sync registers:
	gspWriteGxReg(0x0400, 0x1C2);
	gspWriteGxReg(0x0404, 0xD1);
	gspWriteGxReg(0x0408, 0x1C1);
	gspWriteGxReg(0x040C, 0x1C1);
	gspWriteGxReg(0x0410, 0);
	gspWriteGxReg(0x0414, 0xCF);
	gspWriteGxReg(0x0418, 0xD1);
	gspWriteGxReg(0x041C, (0x1C5 << 16) | 0x1C1);
	gspWriteGxReg(0x0420, 0x10000);
	gspWriteGxReg(0x0424, 0x19D);
	gspWriteGxReg(0x0428, 2);
	gspWriteGxReg(0x042C, 0x192);
	gspWriteGxReg(0x0430, 0x192);
	gspWriteGxReg(0x0434, 0x192);
	gspWriteGxReg(0x0438, 1);
	gspWriteGxReg(0x043C, 2);
	gspWriteGxReg(0x0440, (0x196 << 16) | 0x192);
	gspWriteGxReg(0x0444, 0);
	gspWriteGxReg(0x0448, 0);

	// Top screen fb geometry
	gspWriteGxReg(0x045C, (400 << 16) | 240); // dimensions
	gspWriteGxReg(0x0460, (0x1C1 << 16) | 0xD1);
	gspWriteGxReg(0x0464, (0x192 << 16) | 2);

	// Top screen framebuffer format (initial)
	gspWriteGxReg(0x0470, 0x80340);

	// Top screen unknown reg @ 0x9C
	gspWriteGxReg(0x049C, 0);

	// Bottom screen LCD configuration

	// Bottom screen sync registers:
	gspWriteGxReg(0x0500, 0x1C2);
	gspWriteGxReg(0x0504, 0xD1);
	gspWriteGxReg(0x0508, 0x1C1);
	gspWriteGxReg(0x050C, 0x1C1);
	gspWriteGxReg(0x0510, 0xCD);
	gspWriteGxReg(0x0514, 0xCF);
	gspWriteGxReg(0x0518, 0xD1);
	gspWriteGxReg(0x051C, (0x1C5 << 16) | 0x1C1);
	gspWriteGxReg(0x0520, 0x10000);
	gspWriteGxReg(0x0524, 0x19D);
	gspWriteGxReg(0x0528, 0x52);
	gspWriteGxReg(0x052C, 0x192);
	gspWriteGxReg(0x0530, 0x192);
	gspWriteGxReg(0x0534, 0x4F);
	gspWriteGxReg(0x0538, 0x50);
	gspWriteGxReg(0x053C, 0x52);
	gspWriteGxReg(0x0540, (0x198 << 16) | 0x194);
	gspWriteGxReg(0x0544, 0);
	gspWriteGxReg(0x0548, 0x11);

	// Bottom screen fb geometry
	gspWriteGxReg(0x055C, (320 << 16) | 240); // dimensions
	gspWriteGxReg(0x0560, (0x1C1 << 16) | 0xD1);
	gspWriteGxReg(0x0564, (0x192 << 16) | 0x52);

	// Bottom screen framebuffer format (initial)
	gspWriteGxReg(0x0570, 0x80300);

	// Bottom screen unknown reg @ 0x9C
	gspWriteGxReg(0x059C, 0);

	// Initial, blank framebuffer (top left A/B, bottom A/B, top right A/B)
	gspWriteGxReg(0x0468, 0x18300000);
	gspWriteGxReg(0x046C, 0x18300000);
	gspWriteGxReg(0x0568, 0x18300000);
	gspWriteGxReg(0x056C, 0x18300000);
	gspWriteGxReg(0x0494, 0x18300000);
	gspWriteGxReg(0x0498, 0x18300000);

	// Framebuffer select: A
	gspWriteGxReg(0x0478, 1);
	gspWriteGxReg(0x0578, 1);

	// Clear DMA transfer (PPF) "transfer finished" bit
	gspWriteGxRegMasked(0x0C18, 0, 0xFF00);

	// GX_GPU_CLK |= 0x70000 (value is 0x100 when gsp starts, enough to at least display framebuffers & have memory fill work)
	// This enables the clock to some GPU components
	gspWriteGxReg(0x0004, 0x70100);

	// Clear Memory Fill (PSC0 and PSC1) "busy" and "finished" bits
	gspWriteGxRegMasked(0x001C, 0, 0xFF);
	gspWriteGxRegMasked(0x002C, 0, 0xFF);

	// More init registers
	gspWriteGxReg(0x0050, 0x22221200);
	gspWriteGxRegMasked(0x0054, 0xFF2, 0xFFFF);

	// Enable some LCD clocks (?) (unsure)
	gspWriteGxReg(0x0474, 0x10501);
	gspWriteGxReg(0x0574, 0x10501);
}

Result gspInit(void)
{
	Result ret=0;
	if (AtomicPostIncrement(&gspRefCount)) return 0;

	// Initialize events
	for (int i = 0; i < GSPGPU_EVENT_MAX; i ++)
		LightEvent_Init(&gspEvents[i], RESET_STICKY);

	// Retrieve a GSP service session handle
	ret = srvGetServiceHandle(&gspGpuHandle, "gsp::Gpu");
	if (R_FAILED(ret)) goto _fail0;

	// Acquire GPU rights
	ret = GSPGPU_AcquireRight(0);
	if (R_FAILED(ret)) goto _fail1;

	// Register ourselves as a user of graphics hardware
	svcCreateEvent(&gspEvent, RESET_ONESHOT);
	ret = GSPGPU_RegisterInterruptRelayQueue(gspEvent, 0x1, &gspSharedMemHandle, &gspThreadId);
	if (R_FAILED(ret))
		goto _fail2;

	// Initialize the hardware if we are the first process to register
	if (ret == 0x2A07)
		gspHardwareInit();

	// Map GSP shared memory
	gspSharedMem = mappableAlloc(0x1000);
	svcMapMemoryBlock(gspSharedMemHandle, (u32)gspSharedMem, MEMPERM_READWRITE, MEMPERM_DONTCARE);

	// Start event handling thread
	gspRunEvents = true;
	gspLastEvent = -1;
	gspEventThread = threadCreate(gspEventThreadMain, 0x0, GSP_EVENT_STACK_SIZE, 0x1A, -2, true);
	return 0;

_fail2:
	GSPGPU_ReleaseRight();
_fail1:
	svcCloseHandle(gspGpuHandle);
_fail0:
	AtomicDecrement(&gspRefCount);
	return ret;
}

void gspExit(void)
{
	if (AtomicDecrement(&gspRefCount)) return;

	// Stop event handling thread
	gspRunEvents = false;
	svcSignalEvent(gspEvent);
	threadJoin(gspEventThread, U64_MAX);

	// Unmap and close GSP shared memory
	svcUnmapMemoryBlock(gspSharedMemHandle, (u32)gspSharedMem);
	svcCloseHandle(gspSharedMemHandle);
	mappableFree(gspSharedMem);

	// Unregister ourselves
	GSPGPU_UnregisterInterruptRelayQueue();

	// Release GPU rights and close the service handle
	GSPGPU_ReleaseRight();
	svcCloseHandle(gspGpuHandle);
}

Handle *gspGetSessionHandle(void)
{
	return &gspGpuHandle;
}

bool gspHasGpuRight(void)
{
	return gspGpuRight;
}

bool gspPresentBuffer(unsigned screen, unsigned swap, const void* fb_a, const void* fb_b, u32 stride, u32 mode)
{
	GSPGPU_FramebufferInfo info;
	info.active_framebuf = swap;
	info.framebuf0_vaddr = (u32*)fb_a;
	info.framebuf1_vaddr = (u32*)fb_b;
	info.framebuf_widthbytesize = stride;
	info.format = mode;
	info.framebuf_dispselect = swap;
	info.unk = 0;

	s32* fbInfoHeader = (s32*)((u8*)gspSharedMem + 0x200 + gspThreadId*0x80 + screen*0x40);
	GSPGPU_FramebufferInfo* fbInfos = (GSPGPU_FramebufferInfo*)&fbInfoHeader[1];
	unsigned pos = 1 - (*fbInfoHeader & 0xff);
	fbInfos[pos] = info;
	__dsb();

	union
	{
		s32 header;
		struct { u8 swap, update; };
	} u;

	bool ret;
	do
	{
		u.header = __ldrex(fbInfoHeader);
		ret = u.update != 0;

		u.swap = pos;
		u.update = 1;
	} while (__strex(fbInfoHeader, u.header));

	return ret;
}

bool gspIsPresentPending(unsigned screen)
{
	s32* fbInfoHeader = (s32*)((u8*)gspSharedMem + 0x200 + gspThreadId*0x80 + screen*0x40);
	return (*fbInfoHeader & 0xff00) != 0;
}

void gspSetEventCallback(GSPGPU_Event id, ThreadFunc cb, void* data, bool oneShot)
{
	if(id>= GSPGPU_EVENT_MAX)return;

	gspEventCb[id] = cb;
	gspEventCbData[id] = data;
	gspEventCbOneShot[id] = oneShot;
}

void gspWaitForEvent(GSPGPU_Event id, bool nextEvent)
{
	if(id>= GSPGPU_EVENT_MAX)return;

	if (nextEvent)
		LightEvent_Clear(&gspEvents[id]);
	LightEvent_Wait(&gspEvents[id]);
	if (!nextEvent)
		LightEvent_Clear(&gspEvents[id]);
}

GSPGPU_Event gspWaitForAnyEvent(void)
{
	s32 x;
	do
	{
		do
		{
			x = __ldrex(&gspLastEvent);
			if (x < 0)
			{
				__clrex();
				break;
			}
		} while (__strex(&gspLastEvent, -1));
		if (x < 0)
			syncArbitrateAddress(&gspLastEvent, ARBITRATION_WAIT_IF_LESS_THAN, 0);
	} while (x < 0);
	return (GSPGPU_Event)x;
}

static int popInterrupt(void)
{
	int curEvt;
	bool strexFailed;
	u8* gspEventData = (u8*)gspSharedMem + gspThreadId*0x40;
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

// Dummy version to avoid linking in gxqueue.c if not actually used
__attribute__((weak)) void gxCmdQueueInterrupt(GSPGPU_Event irq)
{
}

void gspEventThreadMain(void *arg)
{
	while (gspRunEvents)
	{
		svcWaitSynchronization(gspEvent, U64_MAX);
		svcClearEvent(gspEvent);

		if (!gspRunEvents)
			break;

		while (true)
		{
			int curEvt = popInterrupt();

			if (curEvt == -1)
				break;

			if (curEvt < GSPGPU_EVENT_MAX)
			{
				gxCmdQueueInterrupt((GSPGPU_Event)curEvt);
				if (gspEventCb[curEvt])
				{
					ThreadFunc func = gspEventCb[curEvt];
					if (gspEventCbOneShot[curEvt])
						gspEventCb[curEvt] = NULL;
					func(gspEventCbData[curEvt]);
				}
				LightEvent_Signal(&gspEvents[curEvt]);
				do
					__ldrex(&gspLastEvent);
				while (__strex(&gspLastEvent, curEvt));
				syncArbitrateAddress(&gspLastEvent, ARBITRATION_SIGNAL, 1);
			}
		}
	}
}

//essentially : get commandIndex and totalCommands, calculate offset of new command, copy command and update totalCommands
//use LDREX/STREX because this data may also be accessed by the GSP module and we don't want to break stuff
//(mostly, we could overwrite the buffer header with wrong data and make the GSP module reexecute old commands)
Result gspSubmitGxCommand(const u32 gxCommand[0x8])
{
	u32* sharedGspCmdBuf = (u32*)((u8*)gspSharedMem + 0x800 + gspThreadId*0x200);
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

Result GSPGPU_WriteHWRegs(u32 regAddr, const u32* data, u8 size)
{
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x1,2,2); // 0x10082
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[4]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_WriteHWRegsWithMask(u32 regAddr, const u32* data, u8 datasize, const u32* maskdata, u8 masksize)
{
	if(datasize>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x2,2,4); // 0x20084
	cmdbuf[1]=regAddr;
	cmdbuf[2]=datasize;
	cmdbuf[3]=IPC_Desc_StaticBuffer(datasize, 0);
	cmdbuf[4]=(u32)data;
	cmdbuf[5]=IPC_Desc_StaticBuffer(masksize, 1);
	cmdbuf[6]=(u32)maskdata;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReadHWRegs(u32 regAddr, u32* data, u8 size)
{
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x4,2,0); // 0x40080
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[0x40]=IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[0x40+1]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetBufferSwap(u32 screenid, const GSPGPU_FramebufferInfo*framebufinfo)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,8,0); // 0x50200
	cmdbuf[1] = screenid;
	memcpy(&cmdbuf[2], framebufinfo, sizeof(GSPGPU_FramebufferInfo));

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_FlushDataCache(const void* adr, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x8,2,2); // 0x80082
	cmdbuf[1]=(u32)adr;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_SharedHandles(1);
	cmdbuf[4]=CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_InvalidateDataCache(const void* adr, u32 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,2,2); // 0x90082
	cmdbuf[1] = (u32)adr;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetLcdForceBlack(u8 flags)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0xB,1,0); // 0xB0040
	cmdbuf[1]=flags;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_TriggerCmdReqQueue(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0xC,0,0); // 0xC0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RegisterInterruptRelayQueue(Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x13,1,2); // 0x130042
	cmdbuf[1]=flags;
	cmdbuf[2]=IPC_Desc_SharedHandles(1);
	cmdbuf[3]=eventHandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	if(threadID)*threadID=cmdbuf[2] & 0xFF;
	if(outMemHandle)*outMemHandle=cmdbuf[4];

	return cmdbuf[1];
}

Result GSPGPU_UnregisterInterruptRelayQueue(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x14,0,0); // 0x140000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_AcquireRight(u8 flags)
{
	if(gspGpuRight) return 0;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x16,1,2); // 0x160042
	cmdbuf[1]=flags;
	cmdbuf[2]=IPC_Desc_SharedHandles(1);
	cmdbuf[3]=CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;
	if(R_SUCCEEDED(cmdbuf[1])) gspGpuRight=true;

	return cmdbuf[1];
}

Result GSPGPU_ReleaseRight(void)
{
	if(!gspGpuRight) return 0;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x17,0,0); // 0x170000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;
	if(R_SUCCEEDED(cmdbuf[1])) gspGpuRight=false;

	return cmdbuf[1];
}

Result GSPGPU_ImportDisplayCaptureInfo(GSPGPU_CaptureInfo* captureinfo)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x18,0,0); // 0x180000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret)) memcpy(captureinfo, &cmdbuf[2], 0x20);

	return ret;
}

Result GSPGPU_SaveVramSysArea(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x19,0,0); // 0x190000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RestoreVramSysArea(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x1A,0,0); // 0x1A0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ResetGpuCore(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x1B,0,0); // 0x001B0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetLedForceOff(bool disable)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1C,1,0); // 0x1C0040
	cmdbuf[1] = disable & 0xFF;

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspGpuHandle))) return ret;

	return cmdbuf[1];
}
