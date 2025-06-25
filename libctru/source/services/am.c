#include <3ds/synchronization.h>
#include <3ds/services/am.h>
#include <3ds/services/fs.h>
#include <3ds/util/utf.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/ipc.h>

#include <string.h>
#include <stdlib.h>

static Handle amHandle;
static int amRefCount;

Result amInit(void)
{
	Result ret;

	if (AtomicPostIncrement(&amRefCount)) return 0;

	ret = srvGetServiceHandle(&amHandle, "am:net");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&amHandle, "am:u");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&amHandle, "am:sys");
	if (R_FAILED(ret)) AtomicDecrement(&amRefCount);

	return ret;
}

Result amAppInit(void)
{
	Result ret;

	if (AtomicPostIncrement(&amRefCount)) return 0;

	ret = srvGetServiceHandle(&amHandle, "am:sys");
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

	if(count) *count = cmdbuf[2];

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

Result AM_GetTitleInfo(FS_MediaType mediatype, u32 titleCount, u64 *titleIds, AM_TitleInfo *titleInfo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,2,4); // 0x00030084
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = IPC_Desc_Buffer(titleCount*sizeof(u64),IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;
	cmdbuf[5] = IPC_Desc_Buffer(titleCount*sizeof(AM_TitleInfo),IPC_BUFFER_W);
	cmdbuf[6] = (u32)titleInfo;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteUserTitle(FS_MediaType mediatype, u64 titleID)
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

	if(extDataId) *extDataId = (u64)cmdbuf[2] | ((u64)cmdbuf[3] << 32);

	return (Result)cmdbuf[1];
}

Result AM_DeleteTicket(u64 titleId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,2,0); // 0x00070080
	cmdbuf[1] = titleId & 0xffffffff;
	cmdbuf[2] = (u32)(titleId >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetTicketCount(u32 *count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,0,0); // 0x00080000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(count) *count = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_GetTicketList(u32 *ticketsRead, u32 ticketCount, u32 skip, u64 *ticketTitleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,2,2); // 0x00090082
	cmdbuf[1] = ticketCount;
	cmdbuf[2] = skip;
	cmdbuf[3] = IPC_Desc_Buffer(ticketCount*sizeof(u64),IPC_BUFFER_W);
	cmdbuf[4] = (u32)ticketTitleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(ticketsRead) *ticketsRead = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_GetDeviceId(s32 *outInternalResult, u32 *outDeviceId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0x000A0000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if (outInternalResult) *outInternalResult = (s32)cmdbuf[2];
	if (outDeviceId) *outDeviceId = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result AM_GetNumPendingTitles(u32 *outNum, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB, 1, 0); // 0x000B0040
	cmdbuf[1] = (u32)mediaType;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_GetPendingTitleList(u32 *outNum, u64 *outTitleIds, u32 count, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC, 2, 2); // 0x000C0082
	cmdbuf[1] = count;
	cmdbuf[2] = (u32)mediaType;
	cmdbuf[3] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_W);
	cmdbuf[4] = (u32)outTitleIds;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_GetPendingTitleInfo(u32 titleCount, FS_MediaType mediatype, u64 *titleIds, AM_PendingTitleInfo *titleInfo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,2,4); // 0x000D0084
	cmdbuf[1] = titleCount;
	cmdbuf[2] = mediatype;
	cmdbuf[3] = IPC_Desc_Buffer(titleCount*sizeof(u64),IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;
	cmdbuf[5] = IPC_Desc_Buffer(titleCount*sizeof(AM_TitleInfo),IPC_BUFFER_W);
	cmdbuf[6] = (u32)titleInfo;

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

Result AM_GetNumImportContentContexts(u32 *outNum, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xF, 3, 0); // 0x000F00C0
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_GetImportContentContextList(u32 *outNum, u16 *outContentIndices, u32 count, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x10, 4, 2); // 0x00100102
	cmdbuf[1] = count;
	cmdbuf[2] = (u32)mediaType;
	cmdbuf[3] = titleId & 0xFFFFFFFF;
	cmdbuf[4] = (u32)(titleId >> 32);
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_W);
	cmdbuf[6] = (u32)outContentIndices;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_GetImportContentContexts(AM_ImportContentContext *outContexts, u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x11, 4, 4); // 0x00110104
	cmdbuf[1] = count;
	cmdbuf[2] = (u32)mediaType;
	cmdbuf[3] = titleId & 0xFFFFFFFF;
	cmdbuf[4] = (u32)(titleId >> 32);
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[6] = (u32)contentIndices;
	cmdbuf[7] = IPC_Desc_Buffer(count * sizeof(AM_ImportContentContext), IPC_BUFFER_W);
	cmdbuf[8] = (u32)outContexts;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AM_DeleteImportContentContexts(u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x12, 4, 2); // 0x00120102
	cmdbuf[1] = count;
	cmdbuf[2] = (u32)mediaType;
	cmdbuf[3] = titleId & 0xFFFFFFFF;
	cmdbuf[4] = (u32)(titleId >> 32);
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[6] = (u32)contentIndices;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	return (Result)cmdbuf[1];
}

Result AM_NeedsCleanup(u8 *outNeedsCleanup, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x13, 1, 0); // 0x00130040
	cmdbuf[1] = (u32)mediaType;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNeedsCleanup) *outNeedsCleanup = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_DoCleanup(FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x14, 1, 0); // 0x00140040
	cmdbuf[1] = (u32)mediaType;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
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

Result AM_DeleteAllTemporaryTitles(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x16,0,0); // 0x00160000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InitializeExternalTitleDatabase(bool overwrite)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x18,2,0); // 0x00180080
	cmdbuf[1] = 1; // No other media type is accepted
	cmdbuf[2] = overwrite;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_QueryAvailableExternalTitleDatabase(bool* available)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x19,1,0); // 0x00190040
	cmdbuf[1] = 1; // No other media type is accepted

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;
	if(R_FAILED(ret = (Result)cmdbuf[1])) return ret;

	// Only accept this if the command was a success
	if(available) *available = cmdbuf[2] & 0xFF;

	return ret;
}

Result AM_CalculateTwlBackupSize(u64 *outSize, u64 titleId, u8 exportType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1A, 3, 0); // 0x001A00C0
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32)(titleId >> 32);
	cmdbuf[3] = (u32)exportType;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outSize) *outSize = *(u64 *)(&cmdbuf[2]);
	return (Result)cmdbuf[1];
}

Result AM_ExportTwlBackup(u64 titleID, u8 operation, void* workbuf, u32 workbuf_size, const char *filepath)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	size_t len=255;
	ssize_t units=0;
	uint16_t filepath16[256];

	memset(filepath16, 0, sizeof(filepath16));
	units = utf8_to_utf16(filepath16, (uint8_t*)filepath, len);
	if(units < 0 || units > len)return -2;
	len = (units+1)*2;

	cmdbuf[0] = IPC_MakeHeader(0x1B,5,4); // 0x001B0144
	cmdbuf[1] = titleID & 0xffffffff;
	cmdbuf[2] = (u32)(titleID >> 32);
	cmdbuf[3] = len;
	cmdbuf[4] = workbuf_size;
	cmdbuf[5] = operation;
	cmdbuf[6] = IPC_Desc_Buffer(len,IPC_BUFFER_R);
	cmdbuf[7] = (u32)filepath16;
	cmdbuf[8] = IPC_Desc_Buffer(workbuf_size,IPC_BUFFER_W);
	cmdbuf[9] = (u32)workbuf;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_ImportTwlBackup(Handle filehandle, u8 operation, void* buffer, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1C,2,4); // 0x001C0084
	cmdbuf[1] = size;
	cmdbuf[2] = operation;
	cmdbuf[3] = IPC_Desc_MoveHandles(1);
	cmdbuf[4] = filehandle;
	cmdbuf[5] = IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[6] = (u32)buffer;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAllTwlUserTitles(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1D,0,0); // 0x001D0000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_ReadTwlBackupInfo(Handle filehandle, void* outinfo, u32 outinfo_size, void* workbuf, u32 workbuf_size, void* banner, u32 banner_size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1E,3,8); // 0x001E00C8
	cmdbuf[1] = outinfo_size;
	cmdbuf[2] = workbuf_size;
	cmdbuf[3] = banner_size;
	cmdbuf[4] = IPC_Desc_MoveHandles(1);
	cmdbuf[5] = filehandle;
	cmdbuf[6] = IPC_Desc_Buffer(outinfo_size,IPC_BUFFER_W);
	cmdbuf[7] = (u32)outinfo;
	cmdbuf[8] = IPC_Desc_Buffer(banner_size,IPC_BUFFER_W);
	cmdbuf[9] = (u32)banner;
	cmdbuf[10] = IPC_Desc_Buffer(workbuf_size,IPC_BUFFER_W);
	cmdbuf[11] = (u32)workbuf;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAllExpiredTitles(FS_MediaType mediatype)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1F,1,0); // 0x001F0040
	cmdbuf[1] = mediatype;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetTWLPartitionInfo(AM_TWLPartitionInfo *info)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x20,0,0); // 0x00200000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(info) memcpy(info, &cmdbuf[2], sizeof(AM_TWLPartitionInfo));

	return (Result)cmdbuf[1];
}

Result AM_GetPersonalizedTicketInfos(u32 *outNum, AM_TicketInfo *outInfos, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x21, 1, 2); // 0x00210042
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_Buffer(count * sizeof(AM_TicketInfo), IPC_BUFFER_W);
	cmdbuf[3] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_DeleteAllImportTitleContexts(FS_MediaType mediaType, u32 flags)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x22, 2, 0); // 0x00220080
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = flags;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AM_GetNumImportTitleContexts(u32 *outNum, FS_MediaType mediaType, u32 flags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x23,2,0); // 0x00230080
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = flags;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(outNum) *outNum = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_GetImportTitleContextList(u32 *outNum, u32 count, FS_MediaType mediaType, u32 flags, u64 *titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x24,3,2); // 0x002400C2
	cmdbuf[1] = count;
	cmdbuf[2] = (u32)mediaType;
	cmdbuf[3] = flags;
	cmdbuf[4] = IPC_Desc_Buffer(count*sizeof(u64),IPC_BUFFER_W);
	cmdbuf[5] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(outNum) *outNum = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_CheckContentRights(u8 *outHasRights, u64 titleId, u16 contentIndex)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x25, 3, 0); // 0x002500C0
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32)(titleId >> 32);
	cmdbuf[3] = (u32)contentIndex;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outHasRights) *outHasRights = (u8)cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_GetTicketLimitInfo(AM_TicketLimitInfo *outInfos, u64 *titleIds, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x26, 1, 4); // 0x00260044
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[3] = (u32)titleIds;
	cmdbuf[4] = IPC_Desc_Buffer(count * sizeof(AM_TicketLimitInfo), IPC_BUFFER_W);
	cmdbuf[5] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AM_GetDemoLaunchInfo(AM_DemoLaunchInfo *outInfos, u64 *titleIds, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x27, 1, 4); // 0x00270044
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[3] = (u32)titleIds;
	cmdbuf[4] = IPC_Desc_Buffer(count * sizeof(AM_DemoLaunchInfo), IPC_BUFFER_W);
	cmdbuf[5] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AM_ReadTwlBackupInfoEx(void *outInfo, u32 infoSize, void *outBanner, u32 bannerSize, void *workingBuffer, u32 workingBufferSize, Handle backupFile)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x28, 4, 8); // 0x00280108
	cmdbuf[1]  = infoSize;
	cmdbuf[2]  = bannerSize;
	cmdbuf[3]  = workingBufferSize;
	cmdbuf[4]  = 0; // not used
	cmdbuf[5]  = IPC_Desc_MoveHandles(1);
	cmdbuf[6]  = backupFile;
	cmdbuf[7]  = IPC_Desc_Buffer(infoSize, IPC_BUFFER_W);
	cmdbuf[8]  = (u32)outInfo;
	cmdbuf[9]  = IPC_Desc_Buffer(bannerSize, IPC_BUFFER_W);
	cmdbuf[10] = (u32)outBanner;
	cmdbuf[11] = IPC_Desc_Buffer(workingBufferSize, IPC_BUFFER_W);
	cmdbuf[12] = (u32)workingBuffer;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AM_DeleteUserTitles(u64 *titleIds, u32 count, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x29, 2, 2); // 0x00290082
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = count;
	cmdbuf[3] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AM_GetNumExistingContentInfos(u32 *outNum, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2A, 3, 0); // 0x002A00C0
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_ListExistingContentInfos(u32 *outNum, AM_ContentInfo *outInfos, u32 offset, u32 count, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2B, 5, 2); // 0x002B0142
	cmdbuf[1] = count;
	cmdbuf[2] = (u32)mediaType;
	cmdbuf[3] = titleId & 0xFFFFFFFF;
	cmdbuf[4] = (u32)(titleId >> 32);
	cmdbuf[5] = offset;
	cmdbuf[6] = IPC_Desc_Buffer(count * sizeof(AM_ContentInfo), IPC_BUFFER_W);
	cmdbuf[7] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_GetTitleInfosIgnorePlatform(AM_TitleInfo *outInfos, u64 *titleIds, u32 count, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2C, 2, 4); // 0x002C0084
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = count;
	cmdbuf[3] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(AM_TitleInfo), IPC_BUFFER_W);
	cmdbuf[6] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AM_CheckContentRightsIgnorePlatform(u8 *outHasRights, u64 titleId, u16 contentIndex)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2D, 3, 0); // 0x002D00C0
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32)(titleId >> 32);
	cmdbuf[3] = (u32)contentIndex;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outHasRights) *outHasRights = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}


Result AM_InstallFirm(u64 titleID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401,2,0); // 0x04010080
	cmdbuf[1] = titleID & 0xffffffff;
	cmdbuf[2] = (u32)(titleID >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_StartCiaInstall(FS_MediaType mediatype, Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x402,1,0); // 0x04020040
	cmdbuf[1] = mediatype;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(ciaHandle) *ciaHandle = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result AM_StartTempCiaInstall(Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x403,0,0); // 0x04030000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(ciaHandle) *ciaHandle = cmdbuf[3];

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

Result AM_FinishCiaInstallWithoutCommit(Handle ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x406,0,2); // 0x04060002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = ciaHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_CommitImportTitles(FS_MediaType mediaType, u32 titleCount, bool temp, const u64* titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x407,3,2); // 0x040700C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = temp ? 1 : 0;
	cmdbuf[4] = IPC_Desc_Buffer(titleCount * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[5] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetCiaFileInfo(FS_MediaType mediatype, AM_TitleInfo *titleEntry, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x408,1,2); // 0x04080042
	cmdbuf[1] = (u32)mediatype;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = fileHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(titleEntry) memcpy(titleEntry, &cmdbuf[2], sizeof(AM_TitleInfo));

	return (Result)cmdbuf[1];
}

Result AM_GetCiaIcon(void *icon, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x409,0,4); // 0x04090004
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = fileHandle;
	cmdbuf[3] = IPC_Desc_Buffer(0x36C0, IPC_BUFFER_W);
	cmdbuf[4] = (u32)icon;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetCiaDependencies(u64 *dependencies, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40A,0,2); // 0x040A0002
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = fileHandle;

	u32 *staticbuf = getThreadStaticBuffers();

	staticbuf[0] = IPC_Desc_StaticBuffer(0x300, 0);
	staticbuf[1] = (u32)dependencies;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetCiaMetaOffset(u64 *metaOffset, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40B,0,2); // 0x040B0002
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = fileHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(metaOffset) *metaOffset = cmdbuf[2] | ((u64)cmdbuf[3] << 32);

	return (Result)cmdbuf[1];
}

Result AM_GetCiaCoreVersion(u32 *coreVersion, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40C,0,2); // 0x040C0002
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = fileHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(coreVersion) *coreVersion = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AM_GetCiaRequiredSpace(u64 *requiredSpace, FS_MediaType mediaType, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40D,1,2); // 0x040D0042
	cmdbuf[1] = mediaType;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = fileHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(requiredSpace) *requiredSpace = cmdbuf[2] | ((u64)cmdbuf[3] << 32);

	return (Result)cmdbuf[1];
}

Result AM_CommitImportTitlesAndInstallFirmAuto(FS_MediaType mediaType, u32 titleCount, bool temp, u64* titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40E,3,2); // 0x040E00C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = temp ? 1 : 0;
	cmdbuf[4] = IPC_Desc_Buffer(titleCount * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[5] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallFirmAuto(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40F,0,0); // 0x040F0000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteTitle(FS_MediaType mediatype, u64 titleId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x410,3,0); // 0x041000C0
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleId & 0xffffffff;
	cmdbuf[3] = (u32)(titleId >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetTwlTitleListForReboot(u32 *outNum, u64 *titleIds, u32 *contentIds, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x411, 1, 4); // 0x04110044
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_W);
	cmdbuf[3] = (u32)titleIds;
	cmdbuf[4] = IPC_Desc_Buffer(count * sizeof(u32), IPC_BUFFER_W);
	cmdbuf[5] = (u32)contentIds;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_GetSystemUpdaterMutex(Handle *outMutex)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x412, 0, 0); // 0x04120000

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outMutex) *outMutex = (Handle)cmdbuf[3];
	return (Result)cmdbuf[1];
}

Result AM_GetCiaMetaSize(u32 *outSize, Handle ciaFile)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x413, 0, 2); // 0x04130002
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = ciaFile;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outSize) *outSize = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_GetCiaMetaSection(void *meta, u32 size, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x414,1,4); // 0x04140044
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = fileHandle;
	cmdbuf[4] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[5] = (u32)meta;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_CheckDemoLaunchRight(u8 *outHasRight, u64 titleId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x415, 2, 0); // 0x04150080
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outHasRight) *outHasRight = (u8)cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AM_GetInternalTitleLocationInfo(AM_InternalTitleLocationInfo *outInfo, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x416, 3, 0); // 0x41600C0
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outInfo) memcpy(outInfo, &cmdbuf[2], sizeof(AM_InternalTitleLocationInfo));
	return (Result)cmdbuf[1];
}

Result AM_MigrateAGBToSAV(u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x417, 3, 0); // 0x041700C0
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AM_StartCiaInstallOverwrite(Handle *outCiaHandle, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x418, 1, 0); // 0x04180040
	cmdbuf[1] = (u32)mediaType;

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outCiaHandle) *outCiaHandle = (Handle)cmdbuf[3];
	return (Result)cmdbuf[1];
}

Result AM_StartSystemCiaInstall(Handle *outCiaHandle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x419, 0, 0); // 0x04190000

	Result res = svcSendSyncRequest(amHandle);

	if (R_FAILED(res)) return res;

	if (outCiaHandle) *outCiaHandle = (Handle)cmdbuf[3];
	return (Result)cmdbuf[1];
}


Result AMNET_InstallTicketBegin(Handle *ticketHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,0,0); // 0x08010000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(ticketHandle) *ticketHandle = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTicketAbort(Handle ticketHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,0,2); // 0x08020002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = ticketHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTicketFinish(Handle ticketHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,0,2); // 0x08030002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = ticketHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTitleBegin(FS_MediaType mediaType, u64 titleId, bool inTempDb)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804,4,0); // 0x08040100
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) (titleId & 0xFFFFFFFF);
	cmdbuf[3] = (u32) ((titleId >> 32) & 0xFFFFFFFF);
	cmdbuf[4] = inTempDb ? 1 : 0;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTitleStop(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,0,0); // 0x08050000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTitleResume(FS_MediaType mediaType, u64 titleId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x806,3,0); // 0x080600C0
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) (titleId & 0xFFFFFFFF);
	cmdbuf[3] = (u32) ((titleId >> 32) & 0xFFFFFFFF);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTitleAbort(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x807,0,0); // 0x08070000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTitleFinish(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x808,0,0); // 0x08080000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_CommitImportTitles(FS_MediaType mediaType, u32 titleCount, bool temp, const u64* titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x809,3,2); // 0x080900C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = temp ? 1 : 0;
	cmdbuf[4] = IPC_Desc_Buffer(titleCount * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[5] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTmdBegin(Handle *tmdHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80A,0,0); // 0x080A0000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(tmdHandle) *tmdHandle = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTmdAbort(Handle tmdHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80B,0,2); // 0x080B0002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = tmdHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTmdFinish(Handle tmdHandle, bool unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80C,1,2); // 0x080C0042
	cmdbuf[1] = unk; /* this value is not used */
	cmdbuf[2] = IPC_Desc_MoveHandles(1);
	cmdbuf[3] = tmdHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_CreateImportContentContexts(u32 contentCount, u16* contentIndices)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80D,1,2); // 0x080D0042
	cmdbuf[1] = contentCount;
	cmdbuf[2] = IPC_Desc_Buffer(contentCount * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[3] = (u32)contentIndices;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallContentBegin(Handle *contentHandle, u16 index)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80E,1,0); // 0x080E0040
	cmdbuf[1] = index;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(contentHandle) *contentHandle = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result AMNET_InstallContentStop(Handle contentHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80F,0,2); // 0x080F0002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = contentHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallContentResume(Handle *contentHandle, u64* resumeOffset, u16 index)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x810,1,0); // 0x08100040
	cmdbuf[1] = index;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(contentHandle) *contentHandle = cmdbuf[5];
	if(resumeOffset) *resumeOffset = cmdbuf[2] | ((u64)cmdbuf[3] << 32);

	return (Result)cmdbuf[1];
}

Result AMNET_InstallContentCancel(Handle contentHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x811,0,2); // 0x08110002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = contentHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallContentFinish(Handle contentHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x812,0,2); // 0x08120002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = contentHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_GetNumCurrentImportContentContexts(u32 *outNum)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x813, 0, 0); // 0x08130000

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AMNET_GetCurrentImportContentContextsList(u32 *outNum, u16 *outIndices, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x814, 1, 2); // 0x08140040
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_W);
	cmdbuf[3] = (u32)outIndices;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMNET_GetCurrentImportContentContexts(AM_ImportContentContext *outContexts, u16 *indices, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x815, 1, 4); // 0x08150044
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[3] = (u32)indices;
	cmdbuf[4] = IPC_Desc_Buffer(count * sizeof(AM_ImportContentContext), IPC_BUFFER_W);
	cmdbuf[5] = (u32)outContexts;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AMNET_Sign(s32 *outInternalResult, void *outSignature, u32 signatureSize, void *outCertificate, u32 certificateSize, void *indata, u32 indataSize, u64 titleId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x816, 5, 6); // 0x08160146
	cmdbuf[1]  = signatureSize;
	cmdbuf[2]  = certificateSize;
	cmdbuf[3]  = titleId & 0xFFFFFFFF;
	cmdbuf[4]  = (u32)(titleId >> 32);
	cmdbuf[5]  = indataSize;
	cmdbuf[6]  = IPC_Desc_Buffer(indataSize, IPC_BUFFER_R);
	cmdbuf[7]  = (u32)indata;
	cmdbuf[8]  = IPC_Desc_Buffer(signatureSize, IPC_BUFFER_W);
	cmdbuf[9]  = (u32)outSignature;
	cmdbuf[10] = IPC_Desc_Buffer(certificateSize, IPC_BUFFER_W);
	cmdbuf[11] = (u32)outCertificate;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outInternalResult) *outInternalResult = (s32)cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMNET_GetCTCert(s32 *outInternalResult, void *outCert, u32 certSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x818, 1, 2); // 0x08180042
	cmdbuf[1] = certSize;
	cmdbuf[2] = IPC_Desc_Buffer(certSize, IPC_BUFFER_W);
	cmdbuf[3] = (u32)outCert;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outInternalResult) *outInternalResult = (s32)cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMNET_ImportCertificates(u32 cert1Size, void* cert1, u32 cert2Size, void* cert2, u32 cert3Size, void* cert3, u32 cert4Size, void* cert4)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x819,4,8); // 0x08190108
	cmdbuf[1]  = cert1Size;
	cmdbuf[2]  = cert2Size;
	cmdbuf[3]  = cert3Size;
	cmdbuf[4]  = cert4Size;
	cmdbuf[5]  = IPC_Desc_Buffer(cert1Size, IPC_BUFFER_R);
	cmdbuf[6]  = (u32)cert1;
	cmdbuf[7]  = IPC_Desc_Buffer(cert2Size, IPC_BUFFER_R);
	cmdbuf[8]  = (u32)cert2;
	cmdbuf[9]  = IPC_Desc_Buffer(cert3Size, IPC_BUFFER_R);
	cmdbuf[10] = (u32)cert3;
	cmdbuf[11] = IPC_Desc_Buffer(cert4Size, IPC_BUFFER_R);
	cmdbuf[12] = (u32)cert4;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;
	return (Result)cmdbuf[1];
}

Result AMNET_ImportCertificate(u32 certSize, void* cert)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81A,1,2); // 0x081A0042
	cmdbuf[1] = certSize;
	cmdbuf[2] = IPC_Desc_Buffer(certSize, IPC_BUFFER_R);
	cmdbuf[3] = (u32)cert;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_CommitImportTitlesAndInstallFirmAuto(FS_MediaType mediaType, u32 titleCount, bool temp, u64* titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81B,3,2); // 0x081B00C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = temp ? 1 : 0;
	cmdbuf[4] = IPC_Desc_Buffer(titleCount * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[5] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_DeleteTicket(u64 titleId, u64 ticketId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81C, 4, 0); // 0x081C0100
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32)(titleId >> 32);
	cmdbuf[3] = ticketId & 0xFFFFFFFF;
	cmdbuf[4] = (u32)(ticketId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AMNET_GetTitleNumTicketIds(u32 *outNum, u64 titleId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81D, 2, 0); // 0x081D0080
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AMNET_GetTitleTicketIds(u32 *outNum, u64 *outTicketIds, u32 count, u64 titleId, bool verifyTickets)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81E, 4, 2); // 0x081E0102
	cmdbuf[1] = count;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);
	cmdbuf[4] = verifyTickets ? 1 : 0;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_W);
	cmdbuf[6] = (u32)outTicketIds;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMNET_GetTitleNumTickets(u32 *outNum, u64 titleId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81F, 2, 0); // 0x081F0080
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AMNET_GetTicketInfos(u32 *outNum, AM_TicketInfo *outInfos, u32 offset, u32 count, u64 titleId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x820, 4, 2); // 0x08200102
	cmdbuf[1] = count;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);
	cmdbuf[4] = offset;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(AM_TicketInfo), IPC_BUFFER_W);
	cmdbuf[6] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMNET_ExportLicenseTicket(u32 *outActualSize, void *outdata, u32 outdataSize, u64 titleId, u64 ticketId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x821, 5, 2); // 0x08210142
	cmdbuf[1] = outdataSize;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);
	cmdbuf[4] = ticketId & 0xFFFFFFFF;
	cmdbuf[5] = (u32)(ticketId >> 32);
	cmdbuf[6] = IPC_Desc_Buffer(outdataSize, IPC_BUFFER_W);
	cmdbuf[7] = (u32)outdata;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outActualSize) *outActualSize = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMNET_GetNumCurrentContentInfos(u32 *outNum)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x822, 0, 0); // 0x08220000

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMNET_FindCurrentContentInfos(AM_ContentInfo *outInfos, u16 *contentIndices, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x823, 1, 4); // 0x08230044
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[3] = (u32)contentIndices;
	cmdbuf[4] = IPC_Desc_Buffer(count * sizeof(AM_ContentInfo), IPC_BUFFER_W);
	cmdbuf[5] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	return (Result)cmdbuf[1];
}

Result AMNET_ListCurrentContentInfos(u32 *outNum, AM_ContentInfo *outInfos, u32 offset, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x824, 2, 2); // 0x08240082
	cmdbuf[1] = count;
	cmdbuf[2] = offset;
	cmdbuf[3] = IPC_Desc_Buffer(count * sizeof(AM_ContentInfo), IPC_BUFFER_W);
	cmdbuf[4] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMNET_CalculateContextRequiredSize(u64 *outRequiredSize, u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x825, 4, 2); // 0x08250102
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);
	cmdbuf[4] = count;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[6] = (u32)contentIndices;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outRequiredSize) *outRequiredSize = *((u64 *)(&cmdbuf[2]));
	return (Result)cmdbuf[1];
}

Result AMNET_UpdateImportContentContexts(u16 *contentIndices, u32 count)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x826, 1, 2); // 0x08260042
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[3] = (u32)contentIndices;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AMNET_DeleteAllDemoLaunchInfos(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x827,0,0); // 0x8270000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AMNET_InstallTitleBeginForOverwrite(u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x828, 3, 0); // 0x082800C0
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AMNET_ExportTicketWrapped(u32 *outWrappedTicketSize, u32 *outKeyIvSize, void *outWrappedTicket, u32 wrappedTicketSize, void *outKeyIv, u32 keyIvSize, u64 titleId, u64 ticketId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x829, 6, 4); // 0x08290184
	cmdbuf[1]  = wrappedTicketSize;
	cmdbuf[2]  = keyIvSize;
	cmdbuf[3]  = titleId & 0xFFFFFFFF;
	cmdbuf[4]  = (u32)(titleId >> 32);
	cmdbuf[5]  = ticketId & 0xFFFFFFFF;
	cmdbuf[6]  = (u32)(ticketId >> 32);
	cmdbuf[7]  = IPC_Desc_Buffer(wrappedTicketSize, IPC_BUFFER_W);
	cmdbuf[8]  = (u32)outWrappedTicket;
	cmdbuf[9]  = IPC_Desc_Buffer(keyIvSize, IPC_BUFFER_W);
	cmdbuf[10] = (u32)outKeyIv;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outWrappedTicketSize) *outWrappedTicketSize = cmdbuf[2];
	if (outKeyIvSize) *outKeyIvSize = cmdbuf[3];
	return (Result)cmdbuf[1];
}


Result AMAPP_GetDLCContentInfoCount(u32* count, FS_MediaType mediatype, u64 titleID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1001,3,0); // 0x100100C0
	cmdbuf[1] = (u32)mediatype;
	cmdbuf[2] = titleID & 0xffffffff;
	cmdbuf[3] = (u32)(titleID >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*count = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AMAPP_FindDLCContentInfos(AM_ContentInfo *outInfos, u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1002, 4, 4); // 0x10020104
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);
	cmdbuf[4] = count;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[6] = (u32)contentIndices;
	cmdbuf[7] = IPC_Desc_Buffer(count * sizeof(AM_ContentInfo), IPC_BUFFER_W);
	cmdbuf[8] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AMAPP_ListDLCContentInfos(u32* contentInfoRead, FS_MediaType mediatype, u64 titleID, u32 contentInfoCount, u32 offset, AM_ContentInfo* contentInfos)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = (IPC_MakeHeader(0x1003,5,2)); // 0x10030142
	cmdbuf[1] = contentInfoCount;
	cmdbuf[2] = (u32)mediatype;
	cmdbuf[3] = titleID & 0xffffffff;
	cmdbuf[4] = (u32)(titleID >> 32);
	cmdbuf[5] = offset;
	cmdbuf[6] = IPC_Desc_Buffer(contentInfoCount * sizeof(AM_ContentInfo), IPC_BUFFER_W);
	cmdbuf[7] = (u32)contentInfos;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	*contentInfoRead = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result AMAPP_DeleteDLCContents(u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1004, 4, 2); // 0x10040102
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);
	cmdbuf[4] = count;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(u16), IPC_BUFFER_R);
	cmdbuf[6] = (u32)contentIndices;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AMAPP_GetDLCTitleInfos(AM_TitleInfo *outInfos, u64 *titleIds, u32 count, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1005, 2, 4); // 0x10050084
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = count;
	cmdbuf[3] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(AM_TitleInfo), IPC_BUFFER_W);
	cmdbuf[6] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result AMAPP_GetDLCOrLicenseNumTickets(u32 *outNum, u64 titleId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1006, 2, 0); // 0x10060080
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMAPP_ListDLCOrLicenseTicketInfos(u32 *outNum, AM_TicketInfo *outInfos, u32 offset, u32 count, u64 titleId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1007, 4, 2); // 0x10070102
	cmdbuf[1] = count;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);
	cmdbuf[4] = offset;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(AM_TicketInfo), IPC_BUFFER_W);
	cmdbuf[6] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMAPP_GetDLCOrLicenseItemRights(u32 *outNumRecords, u32 *outNextOffset, void *outRights, u32 rightsSize, u32 rightsType, u32 recordOffset, u64 titleId, u64 ticketId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1008, 7, 2); // 0x100801C2
	cmdbuf[1] = rightsSize;
	cmdbuf[2] = rightsType;
	cmdbuf[3] = titleId & 0xFFFFFFFF;
	cmdbuf[4] = (u32)(titleId >> 32);
	cmdbuf[5] = ticketId & 0xFFFFFFFF;
	cmdbuf[6] = (u32)(ticketId >> 32);
	cmdbuf[7] = recordOffset;
	cmdbuf[8] = IPC_Desc_Buffer(rightsSize, IPC_BUFFER_W);
	cmdbuf[9] = (u32)outRights;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNumRecords) *outNumRecords = cmdbuf[2];
	if (outNextOffset) *outNextOffset = cmdbuf[3];
	return (Result)cmdbuf[1];
}

Result AMAPP_IsDLCTitleInUse(u8 *outInUse, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1009, 3, 0); // 0x100900C0
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outInUse) *outInUse = (u8)cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMAPP_QueryAvailableExternalTitleDatabase(bool *available)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x100A, 0, 0); // 0x100A0000

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (available) *available = (u8)cmdbuf[2] == 1;
	return (Result)cmdbuf[1];
}

Result AMAPP_GetNumExistingDLCContentInfos(u32 *outNum, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x100B, 3, 0); // 0x100B00C0
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = titleId & 0xFFFFFFFF;
	cmdbuf[3] = (u32)(titleId >> 32);

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMAPP_ListExistingDLCContentInfos(u32 *outNum, AM_ContentInfo *outInfos, u32 offset, u32 count, u64 titleId, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x100C, 5, 2); // 0x100C0142
	cmdbuf[1] = count;
	cmdbuf[2] = (u32)mediaType;
	cmdbuf[3] = titleId & 0xFFFFFFFF;
	cmdbuf[4] = (u32)(titleId >> 32);
	cmdbuf[5] = offset;
	cmdbuf[6] = IPC_Desc_Buffer(count * sizeof(AM_ContentInfo), IPC_BUFFER_W);
	cmdbuf[7] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;

	if (outNum) *outNum = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result AMAPP_GetPatchTitleInfos(AM_TitleInfo *outInfos, u64 *titleIds, u32 count, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x100D, 2, 4); // 0x100D0084
	cmdbuf[1] = (u32)mediaType;
	cmdbuf[2] = count;
	cmdbuf[3] = IPC_Desc_Buffer(count * sizeof(u64), IPC_BUFFER_R);
	cmdbuf[4] = (u32)titleIds;
	cmdbuf[5] = IPC_Desc_Buffer(count * sizeof(AM_TitleInfo), IPC_BUFFER_W);
	cmdbuf[6] = (u32)outInfos;

	Result res = svcSendSyncRequest(amHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}
