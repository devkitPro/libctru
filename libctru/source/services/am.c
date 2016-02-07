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

Result AM_GetTitleCount(FS_MediaType mediatype, u32 *count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,1,0); // 0x00010040
	cmdbuf[1] = mediatype;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*count = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result AM_GetTitleList(u32* titlesRead, FS_MediaType mediatype, u32 titleCount, u64 *titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,2,2); // 0x00020082
	cmdbuf[1] = titleCount;
	cmdbuf[2] = mediatype;
	cmdbuf[3] = IPC_Desc_Buffer(titleCount*sizeof(u64),IPC_BUFFER_W);
	cmdbuf[4] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;
	
	if(titlesRead) *titlesRead = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_GetTitleInfo(FS_MediaType mediatype, u32 titleCount, u64 *titleIds, AM_TitleEntry *titleInfo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,2,4); // 0x00030084
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = IPC_Desc_Buffer(titleCount*sizeof(u64),IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;
	cmdbuf[5] = IPC_Desc_Buffer(titleCount*sizeof(AM_TitleEntry),IPC_BUFFER_W);
	cmdbuf[6] = (u32)titleInfo;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetTicketCount(u32 *count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,0,0); // 0x00080000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*count = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result AM_GetTicketList(u32 *ticketsRead, u32 ticketCount, u32 skip, u64 *ticketIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,2,2); // 0x00090082
	cmdbuf[1] = ticketCount;
	cmdbuf[2] = skip;
	cmdbuf[3] = IPC_Desc_Buffer(ticketCount*sizeof(u64),IPC_BUFFER_W);
	cmdbuf[4] = (u32)ticketIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;
	
	if(ticketsRead) *ticketsRead = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_GetPendingTitleCount(u32 *count, FS_MediaType mediatype, u32 statusMask)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x23,2,0); // 0x00230080
	cmdbuf[1] = mediatype;
	cmdbuf[2] = statusMask;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*count = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result AM_GetPendingTitleList(u32 *titlesRead, u32 titleCount, FS_MediaType mediatype, u32 statusMask, u64 *titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x24,3,2); // 0x002400C2
	cmdbuf[1] = titleCount;
	cmdbuf[2] = mediatype;
	cmdbuf[3] = statusMask;
	cmdbuf[4] = IPC_Desc_Buffer(titleCount*sizeof(u64),IPC_BUFFER_W);
	cmdbuf[5] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;
	
	if(titlesRead) *titlesRead = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_GetPendingTitleInfo(u32 titleCount, FS_MediaType mediatype, u64 *titleIds, AM_PendingTitleEntry *titleInfo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,2,4); // 0x000D0084
	cmdbuf[1] = titleCount;
	cmdbuf[2] = mediatype;
	cmdbuf[3] = IPC_Desc_Buffer(titleCount*sizeof(u64),IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;
	cmdbuf[5] = IPC_Desc_Buffer(titleCount*sizeof(AM_TitleEntry),IPC_BUFFER_W);
	cmdbuf[6] = (u32)titleInfo;

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

Result AM_StartCiaInstall(FS_MediaType mediatype, Handle *ciaHandle)
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

Result AM_CancelCIAInstall(Handle ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x404,0,2); // 0x04040002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = ciaHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_FinishCiaInstall(Handle ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x405,0,2); // 0x04050002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = ciaHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteTitle(FS_MediaType mediatype, u64 titleID)
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

Result AM_DeleteAppTitle(FS_MediaType mediatype, u64 titleID)
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

Result AM_DeleteTicket(u64 ticketId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,2,0); // 0x00070080
	cmdbuf[1] = ticketId & 0xffffffff;
	cmdbuf[2] = (u32)(ticketId >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeletePendingTitle(FS_MediaType mediatype, u64 titleId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xE,3,0); // 0x000E00C0
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleId & 0xffffffff;
	cmdbuf[3] = (u32)(titleId >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeletePendingTitles(FS_MediaType mediatype, u32 flags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x22,2,0); // 0x00220080
	cmdbuf[1] = mediatype;
	cmdbuf[2] = flags;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAllPendingTitles(FS_MediaType mediatype)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x15,1,0); // 0x00150040
	cmdbuf[1] = mediatype;

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

Result AM_InstallFirm(u64 titleID){
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401,2,0); // 0x04010080
	cmdbuf[1] = titleID & 0xffffffff;
	cmdbuf[2] = (u32)(titleID >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetTitleProductCode(FS_MediaType mediatype, u64 titleId, char *productCode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x5,3,0); // 0x000500C0
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleId & 0xffffffff;
	cmdbuf[3] = (u32)(titleId >> 32);
	
	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	// The product code string can use the full 16 bytes without NULL terminator
	if(productCode) strncpy(productCode, (char*)&cmdbuf[2], 16);

	return (Result)cmdbuf[1];
}

Result AM_GetTitleExtDataId(u64 *extDataId, FS_MediaType mediatype, u64 titleId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x6,3,0); // 0x000600C0
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleId & 0xffffffff;
	cmdbuf[3] = (u32)(titleId >> 32);
	
	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(extDataId) *extDataId = (u64) cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return (Result)cmdbuf[1];
}

Result AM_GetCiaFileInfo(FS_MediaType mediatype, AM_TitleEntry *titleEntry, Handle fileHandle)
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
