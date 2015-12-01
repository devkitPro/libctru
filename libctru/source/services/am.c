#include <string.h>
#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/am.h>
#include <3ds/ipc.h>

static Handle amHandle;
static int amRefCount;

Result amInit(void)
{
	Result ret;

	if (AtomicPostIncrement(&amRefCount)) return 0;

	ret = srvGetServiceHandle(&amHandle, "am:net");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&amHandle, "am:u");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&amHandle, "am:sys");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&amHandle, "am:app");
	if (R_FAILED(ret)) AtomicDecrement(&amRefCount);

	return ret;
}

void amExit(void)
{
	if (AtomicDecrement(&amRefCount)) return;
	svcCloseHandle(amHandle);
}

Handle *amGetSessionHandle(void)
{
	return &amHandle;
}

Result AM_GetTitleCount(u8 mediatype, u32 *count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,1,0); // 0x00010040
	cmdbuf[1] = mediatype;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*count = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result AM_GetTitleIdList(u8 mediatype, u32 count, u64 *titleIDs)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,2,2); // 0x00020082
	cmdbuf[1] = count;
	cmdbuf[2] = mediatype;
	cmdbuf[3] = IPC_Desc_Buffer(count*sizeof(u64),IPC_BUFFER_W);
	cmdbuf[4] = (u32)titleIDs;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;
	
	return (Result)cmdbuf[1];
}

Result AM_ListTitles(u8 mediatype, u32 titleCount, u64 *titleIdList, AM_TitleEntry *titleList)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,2,4); // 0x00030084
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = IPC_Desc_Buffer(titleCount*sizeof(u64),IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIdList;
	cmdbuf[5] = IPC_Desc_Buffer(titleCount*sizeof(AM_TitleEntry),IPC_BUFFER_W);
	cmdbuf[6] = (u32)titleList;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetDeviceId(u32 *deviceID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0x000A0000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*deviceID = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_StartCiaInstall(u8 mediatype, Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x402,1,0); // 0x04020040
	cmdbuf[1] = mediatype;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*ciaHandle = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_StartDlpChildCiaInstall(Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x403,0,0); // 0x04030000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*ciaHandle = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_CancelCIAInstall(Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x404,0,2); // 0x04040002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = *ciaHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_FinishCiaInstall(u8 mediatype, Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x405,0,2); // 0x04050002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = *ciaHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteTitle(u8 mediatype, u64 titleID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x410,3,0); // 0x041000C0
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleID & 0xffffffff;
	cmdbuf[3] = (u32)(titleID >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAppTitle(u8 mediatype, u64 titleID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,3,0); // 0x000400C0
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleID & 0xffffffff;
	cmdbuf[3] = (u32)(titleID >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallNativeFirm(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40F,0,0); // 0x040F0000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetTitleProductCode(u8 mediatype, u64 titleID, char* productCode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x5,3,0); // 0x000500C0
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleID & 0xffffffff;
	cmdbuf[3] = (u32)(titleID >> 32);
	
	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	// The product code string can use the full 16 bytes without NULL terminator
	if(productCode) strncpy(productCode, (char*)&cmdbuf[2], 16);

	return (Result)cmdbuf[1];
}

Result AM_GetCiaFileInfo(u8 mediatype, AM_TitleEntry *titleEntry, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x408,1,2); // 0x04080042
	cmdbuf[1] = mediatype;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = fileHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(titleEntry) memcpy(titleEntry, &cmdbuf[2], sizeof(AM_TitleEntry));

	return (Result)cmdbuf[1];
}

Result AM_InitializeExternalTitleDatabase(bool overwrite)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x18,2,0); // 0x180080
	cmdbuf[1] = 1; // No other media type is accepted
	cmdbuf[2] = overwrite;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_QueryAvailableExternalTitleDatabase(bool* available)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x19,1,0); // 0x190040
	cmdbuf[1] = 1; // No other media type is accepted

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;
	if(R_FAILED(ret = (Result)cmdbuf[1])) return ret;

	// Only accept this if the command was a success
	if(available) *available = cmdbuf[2];

	return ret;
}
