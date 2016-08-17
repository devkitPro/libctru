#include "ndsp-internal.h"
#include <3ds/services/cfgu.h>
#include <3ds/services/fs.h>
#include <3ds/env.h>
#include <3ds/thread.h>

#define NDSP_THREAD_STACK_SIZE 0x1000

u16 ndspFrameId, ndspBufferCurId, ndspBufferId;
void* ndspVars[16][2];

static bool bComponentLoaded = false, bDspReady = false, bSleeping = false, bActuallySleeping = false, bNeedsSync = false;
static u32 droppedFrames, frameCount;

static const void* componentBin;
static u32 componentSize;
static u16 componentProgMask, componentDataMask;
static bool componentFree;

static aptHookCookie aptCookie;

static Handle irqEvent, dspSem;
static LightEvent sleepEvent;
static LightLock ndspMutex;

static u8 dspVar5Backup[0x1080];

static volatile bool ndspThreadRun;
static Thread ndspThread;

static Result ndspLoadComponent(void)
{
	if (!componentBin) return 1;
	return DSP_LoadComponent(componentBin, componentSize, componentProgMask, componentDataMask, &bComponentLoaded);
}

static inline void ndspWaitForIrq(void)
{
	LightLock_Lock(&ndspMutex);
	svcWaitSynchronization(irqEvent, U64_MAX);
	svcClearEvent(irqEvent);
	LightLock_Unlock(&ndspMutex);
}

static inline void ndspSetCounter(int a, int counter)
{
	*(vu16*)ndspVars[0][a] = counter;
}

static inline int ndspGetCounter(int a)
{
	return *(vu16*)ndspVars[0][a];
}

enum
{
	MFLAG_MASTERVOL    = BIT(0),
	MFLAG_OUTPUTMODE   = BIT(1),
	MFLAG_CLIPPINGMODE = BIT(2),
	MFLAG_OUTPUTCOUNT  = BIT(3),
	MFLAG_SYNCMODE     = BIT(4),
	MFLAG_SURR_DEPTH   = BIT(5),
	MFLAG_SURR_POS     = BIT(6),
	MFLAG_SURR_RRATIO  = BIT(7),

#define MFLAG_AUX_ENABLE(i) BIT(8+(i))
#define MFLAG_AUX_BYPASS(i) BIT(10+(i))
#define MFLAG_AUX_VOLUME(i) BIT(12+(i))
};

static struct
{
	LightLock lock;
	u32 flags;
	float masterVol;
	u16 outputMode, clippingMode, outputCount, syncMode;
	ndspWaveBuf* capture;
	ndspCallback callback;
	void* callbackData;

	struct
	{
		u16 depth, pos, rearRatio;
	} surround;

	struct
	{
		u16 enable, frontBypass;
		float volume;
		ndspAuxCallback callback;
		void* callbackData;
	} aux[2];
} ndspMaster;

static void ndspDirtyMaster(void)
{
	ndspMaster.flags = ~0;
}

static void ndspInitMaster(void)
{
	memset(&ndspMaster, 0, sizeof(ndspMaster));
	LightLock_Init(&ndspMaster.lock);
	ndspMaster.masterVol = 1.0f;
	ndspMaster.outputMode = NDSP_OUTPUT_STEREO;
	ndspMaster.clippingMode = NDSP_CLIP_SOFT;
	ndspMaster.outputCount = 2;
	ndspMaster.surround.depth = 0x7FFF;
	ndspMaster.surround.rearRatio = 0x8000;
}

static void ndspUpdateMaster(void)
{
	DspMasterStatus* m = ndspiGetMasterStatus();
	LightLock_Lock(&ndspMaster.lock);

	u32 flags = m->flags, mflags = ndspMaster.flags;
	int i;

	m->headsetConnected = *(vu8*)0x1FF810C0;
	flags |= 0x10000000;

	if (mflags & MFLAG_MASTERVOL)
	{
		m->masterVol = ndspMaster.masterVol;
		flags |= 0x00010000;
	}

	if (mflags & MFLAG_OUTPUTMODE)
	{
		m->outputMode = ndspMaster.outputMode;
		flags |= 0x04000000;
	}

	if (mflags & MFLAG_CLIPPINGMODE)
	{
		m->clippingMode = ndspMaster.clippingMode;
		flags |= 0x08000000;
	}

	if (mflags & MFLAG_OUTPUTCOUNT)
	{
		m->outBufCount = ndspMaster.outputCount;
		flags |= 0x00008000;
	}

	if (mflags & MFLAG_SYNCMODE)
	{
		m->syncMode = ndspMaster.syncMode;
		m->unknown |= 0x10000; //?
	}

	if (mflags & MFLAG_SURR_DEPTH)
	{
		m->surroundDepth = ndspMaster.surround.depth;
		flags |= 0x20000000;
	}

	if (mflags & MFLAG_SURR_POS)
	{
		m->surroundSpeakerPos = ndspMaster.surround.pos;
		flags |= 0x40000000;
	}

	if (mflags & MFLAG_SURR_RRATIO)
	{
		m->rearRatio = ndspMaster.surround.rearRatio;
		flags |= 0x80000000;
	}

	for (i = 0; i < 2; i ++)
	{
		if (mflags & MFLAG_AUX_ENABLE(i))
		{
			m->auxBusEnable[i] = ndspMaster.aux[i].enable;
			flags |= 0x00000100 << i;
		}

		if (mflags & MFLAG_AUX_BYPASS(i))
		{
			m->auxFrontBypass[i] = ndspMaster.aux[i].frontBypass;
			flags |= 0x00000040 << i;
		}

		if (mflags & MFLAG_AUX_VOLUME(i))
		{
			m->auxReturnVol[i] = ndspMaster.aux[i].volume;
			flags |= 0x01000000 << i;
		}
	}

	m->flags = flags;
	ndspMaster.flags = 0;

	LightLock_Unlock(&ndspMaster.lock);
}

static void ndspUpdateCapture(s16* samples, u32 count)
{
	ndspWaveBuf* buf = ndspMaster.capture;
	if (!buf) return;
	memcpy(&buf->data_pcm16[buf->offset*2], samples, count*4);
	buf->offset += count;
	if (buf->offset >= buf->nsamples)
		buf->offset = 0;
}

static Result ndspInitialize(bool resume)
{
	Result rc;

	rc = ndspLoadComponent();
	if (R_FAILED(rc)) return rc;

	rc = svcCreateEvent(&irqEvent, RESET_STICKY);
	if (R_FAILED(rc)) goto _fail1;

	rc = DSP_RegisterInterruptEvents(irqEvent, 2, 2);
	if (R_FAILED(rc)) goto _fail2;

	rc = DSP_GetSemaphoreHandle(&dspSem);
	if (R_FAILED(rc)) goto _fail3;

	DSP_SetSemaphoreMask(0x2000);

	u16 val = resume ? 2 : 0;
	if (resume)
		memcpy(ndspVars[5][0], dspVar5Backup, sizeof(dspVar5Backup));
	DSP_WriteProcessPipe(2, &val, 4);
	DSP_SetSemaphore(0x4000);
	ndspWaitForIrq();

	DSP_ReadPipeIfPossible(2, 0, &val, sizeof(val), NULL);
	u16 vars[16];
	DSP_ReadPipeIfPossible(2, 0, vars, val*2, NULL);
	int i;
	for (i = 0; i < val; i ++)
	{
		DSP_ConvertProcessAddressFromDspDram(vars[i],           (u32*)&ndspVars[i][0]);
		DSP_ConvertProcessAddressFromDspDram(vars[i] | 0x10000, (u32*)&ndspVars[i][1]);
	}

	DSP_SetSemaphore(0x4000);
	ndspFrameId = 4;
	ndspSetCounter(0, 4);
	ndspFrameId++;
	svcSignalEvent(dspSem);
	ndspBufferCurId = ndspFrameId & 1;
	ndspBufferId = ndspFrameId & 1;
	bDspReady = true;

	ndspDirtyMaster();
	ndspUpdateMaster();

	if (resume)
	{
		ndspiDirtyChn();
		ndspiUpdateChn();
		// Force update effect params here
	}

	return 0;

_fail3:
	DSP_RegisterInterruptEvents(0, 2, 2);
_fail2:
	svcCloseHandle(irqEvent);
_fail1:
	DSP_UnloadComponent();
	return rc;
}

static void ndspFinalize(bool suspend)
{
	LightLock_Lock(&ndspMutex);
	u16 val = suspend ? 3 : 1;
	DSP_WriteProcessPipe(2, &val, 4);
	for (;;)
	{
		bool ready;
		DSP_RecvDataIsReady(0, &ready);
		if (ready)
		{
			DSP_RecvData(0, &val);
			if (val == 1)
				break;
		}
	}
	if (suspend)
		memcpy(dspVar5Backup, ndspVars[5][0], sizeof(dspVar5Backup));

	DSP_RegisterInterruptEvents(0, 2, 2);
	svcCloseHandle(irqEvent);
	svcCloseHandle(dspSem);
	DSP_UnloadComponent();
	bComponentLoaded = false;
	bDspReady = false;
	LightLock_Unlock(&ndspMutex);
}

static void ndspAptHook(APT_HookType hook, void* param)
{
	switch (hook)
	{
		case APTHOOK_ONRESTORE:
		case APTHOOK_ONWAKEUP:
			bSleeping = false;
			ndspInitialize(true);
			if (bActuallySleeping)
			{
				bActuallySleeping = false;
				LightEvent_Signal(&sleepEvent);
			}
			break;

		case APTHOOK_ONSUSPEND:
		case APTHOOK_ONSLEEP:
			bSleeping = true;
			ndspFinalize(true);
			break;

		default:
			break;
	}
}

static void ndspSync(void)
{
	if (bSleeping)
	{
		bActuallySleeping = true;
		LightEvent_Wait(&sleepEvent);
	}

	ndspWaitForIrq();
	if (bDspReady)
	{
		int counter = ndspGetCounter(~ndspFrameId & 1);
		if (counter)
		{
			int next = (counter + 1) & 0xFFFF;
			ndspFrameId = next ? next : 2;
			ndspBufferId = ndspFrameId & 1;
			ndspiReadChnState();
			//memcpy(dspVar9Backup, dspVars[9][ndspBufferId], sizeof(dspVar9Backup));
			ndspUpdateCapture((s16*)ndspVars[6][ndspBufferId], 160);
			droppedFrames += *((u16*)ndspVars[5][ndspBufferId] + 1);
		}
		bNeedsSync = false;
	}
}

static void ndspThreadMain(void* arg)
{
	ndspThreadRun = true;
	while (ndspThreadRun)
	{
		ndspSync();

		// Call callbacks here
		if (ndspMaster.callback)
			ndspMaster.callback(ndspMaster.callbackData);

		if (bSleeping || !bDspReady)
			continue;

		if (bNeedsSync)
			ndspSync();

		ndspUpdateMaster();
		// Call aux user callback here if enabled
		// Execute DSP effects here
		ndspiUpdateChn();

		ndspSetCounter(ndspBufferCurId, ndspFrameId++);
		svcSignalEvent(dspSem);
		ndspBufferCurId = ndspFrameId & 1;

		frameCount++;
		bNeedsSync = true;
	}
}

void ndspUseComponent(const void* binary, u32 size, u16 progMask, u16 dataMask)
{
	componentBin = binary;
	componentSize = size;
	componentProgMask = progMask;
	componentDataMask = dataMask;
	componentFree = false;
}

static bool ndspFindAndLoadComponent(void)
{
	Result rc;
	Handle rsrc;
	void* bin;

	componentProgMask = 0xFF;
	componentDataMask = 0xFF;

	// Try loading the DSP component from the filesystem
	do
	{
		static const char dsp_filename[] = "/3ds/dspfirm.cdc";
		FS_Path archPath = { PATH_EMPTY, 1, (u8*)"" };
		FS_Path filePath = { PATH_ASCII, sizeof(dsp_filename), (u8*)dsp_filename };

		rc = FSUSER_OpenFileDirectly(&rsrc, ARCHIVE_SDMC, archPath, filePath, FS_OPEN_READ, 0);
		if (R_FAILED(rc)) break;

		u64 size = 0;
		rc = FSFILE_GetSize(rsrc, &size);
		if (R_FAILED(rc)) { FSFILE_Close(rsrc); break; }

		bin = malloc(size);
		if (!bin) { FSFILE_Close(rsrc); break; }

		u32 dummy = 0;
		rc = FSFILE_Read(rsrc, &dummy, 0, bin, size);
		FSFILE_Close(rsrc);
		if (R_FAILED(rc)) { free(bin); return false; }

		componentBin = bin;
		componentSize = size;
		componentFree = true;
		return true;
	} while (0);

	// Try loading the DSP component from hb:ndsp
	rsrc = envGetHandle("hb:ndsp");
	if (rsrc) do
	{
		extern u32 fake_heap_end;
		u32 mapAddr = (fake_heap_end+0xFFF) &~ 0xFFF;
		rc = svcMapMemoryBlock(rsrc, mapAddr, 0x3, 0x3);
		if (R_FAILED(rc)) break;

		componentSize = *(u32*)(mapAddr + 0x104);
		bin = malloc(componentSize);
		if (bin)
			memcpy(bin, (void*)mapAddr, componentSize);
		svcUnmapMemoryBlock(rsrc, mapAddr);
		if (!bin) break;

		componentBin = bin;
		componentFree = true;
		return true;
	} while (0);

	return false;
}

static int ndspRefCount = 0;

Result ndspInit(void)
{
	Result rc = 0;
	if (AtomicPostIncrement(&ndspRefCount)) return 0;

	if (!componentBin && !ndspFindAndLoadComponent())
	{
		rc = MAKERESULT(RL_PERMANENT, RS_NOTFOUND, RM_DSP, RD_NOT_FOUND);
		goto _fail0;
	}

	LightLock_Init(&ndspMutex);
	ndspInitMaster();
	ndspiInitChn();

	rc = cfguInit();
	if (R_SUCCEEDED(rc))
	{
		u8 outMode;
		rc = CFGU_GetConfigInfoBlk2(sizeof(outMode), 0x70001, &outMode);
		if (R_SUCCEEDED(rc))
			ndspMaster.outputMode = outMode;
		cfguExit();
	}

	rc = dspInit();
	if (R_FAILED(rc)) return rc;

	rc = ndspInitialize(false);
	if (R_FAILED(rc)) goto _fail1;

	LightEvent_Init(&sleepEvent, RESET_ONESHOT);

	ndspThread = threadCreate(ndspThreadMain, 0x0, NDSP_THREAD_STACK_SIZE, 0x18, -2, true);
	if (!ndspThread) goto _fail2;

	aptHook(&aptCookie, ndspAptHook, NULL);
	return 0;

_fail2:
	ndspFinalize(false);
_fail1:
	dspExit();
	if (componentFree)
	{
		free((void*)componentBin);
		componentBin = NULL;
	}
_fail0:
	AtomicDecrement(&ndspRefCount);
	return rc;
}

void ndspExit(void)
{
	if (AtomicDecrement(&ndspRefCount)) return;
	if (!bDspReady) return;
	ndspThreadRun = false;
	if (bActuallySleeping)
	{
		bActuallySleeping = false;
		LightEvent_Signal(&sleepEvent);
	}
	threadJoin(ndspThread, U64_MAX);
	aptUnhook(&aptCookie);
	if (!bSleeping)
		ndspFinalize(false);
	bSleeping = false;
	bNeedsSync = false;
	dspExit();
	if (componentFree)
	{
		free((void*)componentBin);
		componentBin = NULL;
	}
}

u32 ndspGetDroppedFrames(void)
{
	return droppedFrames;
}

u32 ndspGetFrameCount(void)
{
	return frameCount;
}

void ndspSetMasterVol(float volume)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.masterVol = volume;
	ndspMaster.flags |= MFLAG_MASTERVOL;
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspSetOutputMode(ndspOutputMode mode)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.outputMode = mode;
	ndspMaster.flags |= MFLAG_OUTPUTMODE;
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspSetClippingMode(ndspClippingMode mode)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.clippingMode = mode;
	ndspMaster.flags |= MFLAG_CLIPPINGMODE;
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspSetOutputCount(int count)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.outputCount = count;
	ndspMaster.flags |= MFLAG_OUTPUTCOUNT;
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspSetCapture(ndspWaveBuf* capture)
{
	ndspMaster.capture = capture;
}

void ndspSetCallback(ndspCallback callback, void* data)
{
	ndspMaster.callback = callback;
	ndspMaster.callbackData = data;
}

void ndspSurroundSetDepth(u16 depth)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.surround.depth = depth;
	ndspMaster.flags |= MFLAG_SURR_DEPTH;
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspSurroundSetPos(ndspSpeakerPos pos)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.surround.pos = pos;
	ndspMaster.flags |= MFLAG_SURR_POS;
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspSurroundSetRearRatio(u16 ratio)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.surround.rearRatio = ratio;
	ndspMaster.flags |= MFLAG_SURR_RRATIO;
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspAuxSetEnable(int id, bool enable)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.aux[id].enable = enable ? 1 : 0;
	ndspMaster.flags |= MFLAG_AUX_ENABLE(id);
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspAuxSetFrontBypass(int id, bool bypass)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.aux[id].frontBypass = bypass ? 1 : 0;
	ndspMaster.flags |= MFLAG_AUX_BYPASS(id);
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspAuxSetVolume(int id, float volume)
{
	LightLock_Lock(&ndspMaster.lock);
	ndspMaster.aux[id].volume = volume;
	ndspMaster.flags |= MFLAG_AUX_VOLUME(id);
	LightLock_Unlock(&ndspMaster.lock);
}

void ndspAuxSetCallback(int id, ndspAuxCallback callback, void* data)
{
	ndspMaster.aux[id].callback = callback;
	ndspMaster.aux[id].callbackData = data;
}
