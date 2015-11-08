#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/mic.h>
#include <3ds/ipc.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>

static Handle micHandle;
static int micRefCount;

static u8* micSharedMem;
static u32 micSharedMemSize;
static Handle micSharedMemHandle;

Result micInit(u8* buffer, u32 bufferSize)
{
	Result ret = 0;

	if (AtomicPostIncrement(&micRefCount)) return 0;

	ret = srvGetServiceHandle(&micHandle, "mic:u");
	if (R_FAILED(ret)) goto end;

	micSharedMem = buffer;
	micSharedMemSize = bufferSize;

	ret = svcCreateMemoryBlock(&micSharedMemHandle, (u32) micSharedMem, micSharedMemSize, MEMPERM_READ | MEMPERM_WRITE, MEMPERM_READ | MEMPERM_WRITE);
	if (R_FAILED(ret)) goto end;

	ret = MICU_MapSharedMem(micSharedMemSize, micSharedMemHandle);
	if (R_FAILED(ret)) goto end;

	ret = MICU_SetPower(true);
end:
	if (R_FAILED(ret)) micExit();
	return ret;
}

void micExit(void)
{
	if (AtomicDecrement(&micRefCount)) return;

	if (micSharedMemHandle)
	{
		MICU_UnmapSharedMem();
		svcCloseHandle(micSharedMemHandle);
		micSharedMemHandle = 0;
	}

	if (micHandle)
	{
		MICU_SetPower(false);
		svcCloseHandle(micHandle);
		micHandle = 0;
	}

	micSharedMem = NULL;
	micSharedMemSize = 0;
}

u32 micGetSampleDataSize(void)
{
	return micSharedMemSize - 4;
}

u32 micGetLastSampleOffset(void)
{
	if(micSharedMem) return *(u32*) &micSharedMem[micGetSampleDataSize()];
	return 0;
}

Result MICU_MapSharedMem(u32 size, Handle handle)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1,1,2); // 0x10042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = handle;

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}

Result MICU_UnmapSharedMem(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}

Result MICU_StartSampling(MICU_Encoding encoding, MICU_SampleRate sampleRate, u32 offset, u32 size, bool loop)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3,5,0); // 0x30140
	cmdbuf[1] = encoding;
	cmdbuf[2] = sampleRate;
	cmdbuf[3] = offset;
	cmdbuf[4] = size;
	cmdbuf[5] = loop;

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}

Result MICU_AdjustSampling(MICU_SampleRate sampleRate)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x4,1,0); // 0x40040
	cmdbuf[1] = sampleRate;

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}

Result MICU_StopSampling(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x5,0,0); // 0x50000

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}

Result MICU_IsSampling(bool* sampling)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	if (sampling) *sampling = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result MICU_GetEventHandle(Handle* handle)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x7,0,0); // 0x70000

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	if (handle) *handle = cmdbuf[3];
	return cmdbuf[1];
}

Result MICU_SetGain(u8 gain)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x8,1,0); // 0x80040
	cmdbuf[1] = gain;

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}

Result MICU_GetGain(u8* gain)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x9,0,0); // 0x90000

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	if (gain) *gain = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result MICU_SetPower(bool power)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xA,1,0); // 0xA0040
	cmdbuf[1] = power;

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}

Result MICU_GetPower(bool* power)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0xB0000

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	if (power) *power = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result MICU_SetClamp(bool clamp)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xD,1,0); // 0xD0040
	cmdbuf[1] = clamp;

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}

Result MICU_GetClamp(bool* clamp)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xE,0,0); // 0xE0000

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	if (clamp) *clamp = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result MICU_SetAllowShellClosed(bool allowShellClosed)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xF,1,0); // 0xF0040
	cmdbuf[1] = allowShellClosed;

	if (R_FAILED(ret = svcSendSyncRequest(micHandle))) return ret;
	return cmdbuf[1];
}
