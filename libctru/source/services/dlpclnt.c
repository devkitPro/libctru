#include <malloc.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/dlpclnt.h>
#include <3ds/services/ndm.h>
#include <3ds/ipc.h>

static Handle dlpClntHandle;
static Handle dlpClntMemHandle;
static u8* dlpClntMemAddr;
static size_t dlpClntMemSize;
static Handle dlpClntEventHandle;

static int dlpClntRefCount;

static u32 ndm_state;

Result dlpClntInit(void) {
	Result ret = 0;
	ndm_state = 0;

	if (AtomicPostIncrement(&dlpClntRefCount)) return 0;

	ret = ndmuInit();
	if (R_FAILED(ret))goto end;

	ndm_state = 1;
	ret = NDMU_EnterExclusiveState(NDM_EXCLUSIVE_STATE_LOCAL_COMMUNICATIONS);
	if (R_FAILED(ret))goto end;

	ndm_state = 2;

	dlpClntMemSize = dlpCalcSharedMemSize(0x10, 0x200000);
	dlpClntMemAddr = memalign(0x1000, dlpClntMemSize);
	if (dlpClntMemAddr == NULL) {
		ret = -1;
		goto end;
	}

	ret = svcCreateMemoryBlock(&dlpClntMemHandle, (u32)dlpClntMemAddr, dlpClntMemSize, 0x0, MEMPERM_READ | MEMPERM_WRITE);
	if (R_FAILED(ret))goto end;

	ret = svcCreateEvent(&dlpClntEventHandle, RESET_ONESHOT);
	if (R_FAILED(ret))goto end;

	ret = srvGetServiceHandle(&dlpClntHandle, "dlp:CLNT");
	if (R_FAILED(ret))goto end;

	ret = DLPCLNT_Initialize(dlpClntMemSize, 0x10, 0x200000, dlpClntMemHandle, dlpClntEventHandle);
	if (R_FAILED(ret)) {
		svcCloseHandle(dlpClntHandle);
		dlpClntHandle = 0;
		goto end;
	}

	return ret;
end:
	dlpClntExit();
	return ret;
}

void dlpClntExit(void) {
	if (AtomicDecrement(&dlpClntRefCount)) return;

	if (dlpClntHandle)
	{
		DLPCLNT_Finalize();
		svcCloseHandle(dlpClntHandle);
		dlpClntHandle = 0;
	}

	if (ndm_state) {
		if (ndm_state == 2)NDMU_LeaveExclusiveState();
		ndmuExit();
		ndm_state = 0;
	}

	if (dlpClntEventHandle) {
		svcCloseHandle(dlpClntMemHandle);
		dlpClntMemHandle = 0;
	}

	if (dlpClntMemHandle)
	{
		svcCloseHandle(dlpClntMemHandle);
		dlpClntMemHandle = 0;
	}

	dlpClntMemAddr = NULL;
	dlpClntMemSize = 0;
}

bool dlpClntWaitForEvent(bool nextEvent, bool wait) {
	bool ret = true;
	u64 delayvalue = U64_MAX;

	if (!wait)delayvalue = 0;

	if (nextEvent)svcClearEvent(dlpClntEventHandle);

	if (svcWaitSynchronization(dlpClntEventHandle, delayvalue) != 0 && !wait)ret = false;

	if (!nextEvent)svcClearEvent(dlpClntEventHandle);

	return ret;
}

size_t dlpCalcSharedMemSize(u8 maxTitles, size_t constantMemSize) {
	size_t size = maxTitles * 0x2b70 + 0x5a20 + constantMemSize;
	return (size + 0xFFF) & ~0xFFF; // Round up to alignment
}

u64 dlpCreateChildTid(u32 uniqueId, u32 variation) {
	u64 tid = 0;
	if (uniqueId) {
		tid = (u64)0x40001 << 32;
		tid |= variation | ((uniqueId & 0xff0fffff) << 8);
	}
	return tid;
}

Result DLPCLNT_Initialize(size_t sharedMemSize, u8 maxScanTitles, size_t constantMemSize, Handle sharedMemHandle, Handle eventHandle) {
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,3,3); // 0x100C3
	cmdbuf[1] = sharedMemSize;
	cmdbuf[2] = maxScanTitles;
	cmdbuf[3] = constantMemSize;
	cmdbuf[4] = IPC_Desc_SharedHandles(2);
	cmdbuf[5] = sharedMemHandle;
	cmdbuf[6] = eventHandle;

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	return cmdbuf[1];
}

Result DLPCLNT_Finalize(void) {
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	return cmdbuf[1];
}

Result DLPCLNT_GetChannel(u16* channel) {
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	*channel = cmdbuf[2];

	return cmdbuf[1];
}

Result DLPCLNT_StartScan(u16 channel, u8* macAddrFilter, u64 tidFilter) {
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,6,0); // 0x50180
	cmdbuf[1] = channel;
	cmdbuf[2] = tidFilter & 0xFFFFFFFF;
	cmdbuf[3] = tidFilter >> 32;
	if (macAddrFilter) {
		memcpy(cmdbuf + 4, macAddrFilter, 6);
	}
	else {
		cmdbuf[4] = 0;
		cmdbuf[5] = 0;
	}
	cmdbuf[6] = 0; // unknown state filter

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	return cmdbuf[1];
}

Result DLPCLNT_StopScan(void) {
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	return cmdbuf[1];
}

Result DLPCLNT_GetTitleInfo(dlpClntTitleInfo* titleInfo, size_t* actual_size, u8* macAddr, u32 uniqueId, u32 variation) {
	u32* cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	u64 tid = dlpCreateChildTid(uniqueId, variation);

	cmdbuf[0] = IPC_MakeHeader(0x9, 1, 0); // 0x90040
	memcpy(cmdbuf + 1, macAddr, 6);
	cmdbuf[3] = tid & 0xFFFFFFFF;
	cmdbuf[4] = tid >> 32;

	u32* staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(*titleInfo), 0);
	staticbufs[1] = (u32)titleInfo;

	Result ret = 0;
	ret = svcSendSyncRequest(dlpClntHandle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if (R_FAILED(ret))return ret;

	ret = cmdbuf[1];

	if (R_SUCCEEDED(ret))
	{
		if (actual_size)*actual_size = cmdbuf[2];
	}

	return ret;
}

Result DLPCLNT_GetTitleInfoInOrder(dlpClntTitleInfo* titleInfo, size_t* actual_size) {
	u32* cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	cmdbuf[0] = IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1] = 0; // 0 = Iterate?, 1 = Don't Iterate?

	u32* staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(*titleInfo), 0);
	staticbufs[1] = (u32)titleInfo;

	Result ret = 0;
	ret = svcSendSyncRequest(dlpClntHandle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if (R_FAILED(ret))return ret;

	ret = cmdbuf[1];

	if (R_SUCCEEDED(ret))
	{
		if (actual_size)*actual_size = cmdbuf[2];
	}

	return ret;
}

Result DLPCLNT_PrepareForSystemDownload(u8* macAddr, u32 uniqueId, u32 variation) {
	u32* cmdbuf = getThreadCommandBuffer();

	u64 tid = dlpCreateChildTid(uniqueId, variation);

	cmdbuf[0] = IPC_MakeHeader(0xB,4,0); // 0xB0100
	memcpy(cmdbuf + 1, macAddr, 6);
	cmdbuf[3] = tid & 0xFFFFFFFF;
	cmdbuf[4] = tid >> 32;

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	return cmdbuf[1];
}

Result DLPCLNT_StartSystemDownload(void) {
	u32* cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	return cmdbuf[1];
}

Result DLPCLNT_StartTitleDownload(u8* macAddr, u32 uniqueId, u32 variation) {
	u32* cmdbuf = getThreadCommandBuffer();

	u64 tid = dlpCreateChildTid(uniqueId, variation);

	cmdbuf[0] = IPC_MakeHeader(0xD,4,0); // 0xD0100
	memcpy(cmdbuf + 1, macAddr, 6);
	cmdbuf[3] = tid & 0xFFFFFFFF;
	cmdbuf[4] = tid >> 32;

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	return cmdbuf[1];
}

Result DLPCLNT_GetMyStatus(dlpClntMyStatus* status) {
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xE,0,0); // 0xE00000

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	status->state = cmdbuf[2] >> 0x18; // Other bytes unknown.
	status->unitsTotal = cmdbuf[3];
	status->unitsRecvd = cmdbuf[4];

	return cmdbuf[1];
}

Result DLPCLNT_GetWirelessRebootPassphrase(void* buf) {
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x11,0,0); // 0x110000

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	memcpy(buf, cmdbuf + 2, 9);

	return cmdbuf[1];
}

Result DLPCLNT_StopSession(void) {
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x12,0,0); // 0x120000

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(dlpClntHandle)))return ret;

	return cmdbuf[1];
}
