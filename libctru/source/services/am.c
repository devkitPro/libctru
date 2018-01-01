#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/am.h>
#include <3ds/ipc.h>
#include <3ds/util/utf.h>

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

	if(count) *count = cmdbuf[2];

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

	if(count) *count = cmdbuf[2];

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

	if(deviceID) *deviceID = cmdbuf[3];

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
	cmdbuf[8] = IPC_Desc_Buffer(workbuf_size,IPC_BUFFER_W);
	cmdbuf[9] = (u32)workbuf;
	cmdbuf[10] = IPC_Desc_Buffer(banner_size,IPC_BUFFER_W);
	cmdbuf[11] = (u32)banner;

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

Result AM_StartDlpChildCiaInstall(Handle *ciaHandle)
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

Result AM_CommitImportPrograms(FS_MediaType mediaType, u32 titleCount, bool temp, const u64* titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x407,3,2); // 0x040700C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = temp ? 1 : 0;
	cmdbuf[4] = IPC_Desc_Buffer(titleCount * 8, IPC_BUFFER_R);
	cmdbuf[5] = (u32)titleIds;

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

	if(extDataId) *extDataId = (u64)cmdbuf[2] | ((u64)cmdbuf[3] << 32);

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

Result AM_GetCiaIcon(void *icon, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x409,0,4); // 0x04090004
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = fileHandle;
	cmdbuf[3] = IPC_Desc_Buffer(0x36C0, IPC_BUFFER_W);
	cmdbuf[4] = (u32)icon;

	if(R_FAILED(ret = svcSendSyncRequest(*amGetSessionHandle()))) return ret;

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

	if(R_FAILED(ret = svcSendSyncRequest(*amGetSessionHandle()))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetCiaMetaOffset(u64 *metaOffset, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40B,0,2); // 0x040B0002
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = fileHandle;

	if(R_FAILED(ret = svcSendSyncRequest(*amGetSessionHandle()))) return ret;

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

	if(R_FAILED(ret = svcSendSyncRequest(*amGetSessionHandle()))) return ret;

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

	if(R_FAILED(ret = svcSendSyncRequest(*amGetSessionHandle()))) return ret;

	if(requiredSpace) *requiredSpace = cmdbuf[2] | ((u64)cmdbuf[3] << 32);

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
	if(available) *available = cmdbuf[2] & 0xFF;

	return ret;
}

Result AM_InstallTicketBegin(Handle *ticketHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,0,0); // 0x08010000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(ticketHandle) *ticketHandle = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result AM_InstallTicketAbort(Handle ticketHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,0,2); // 0x08020002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = ticketHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallTicketFinish(Handle ticketHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,0,2); // 0x08030002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = ticketHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallTitleBegin(FS_MediaType mediaType, u64 titleId, bool unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804,4,0); // 0x08040100
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) (titleId & 0xFFFFFFFF);
	cmdbuf[3] = (u32) ((titleId >> 32) & 0xFFFFFFFF);
	cmdbuf[4] = unk;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallTitleStop(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,0,0); // 0x08050000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallTitleResume(FS_MediaType mediaType, u64 titleId)
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


Result AM_InstallTitleAbort(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x807,0,0); // 0x08070000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallTitleFinish(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x808,0,0); // 0x08080000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_CommitImportTitles(FS_MediaType mediaType, u32 titleCount, bool temp, const u64* titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x809,3,2); // 0x080900C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = temp ? 1 : 0;
	cmdbuf[4] = IPC_Desc_Buffer(titleCount * 8, IPC_BUFFER_R);
	cmdbuf[5] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallTmdBegin(Handle *tmdHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80A,0,0); // 0x080A0000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(tmdHandle) *tmdHandle = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result AM_InstallTmdAbort(Handle tmdHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80B,0,2); // 0x080B0002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = tmdHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallTmdFinish(Handle tmdHandle, bool unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80C,1,2); // 0x080C0042
	cmdbuf[1] = unk;
	cmdbuf[2] = IPC_Desc_MoveHandles(1);
	cmdbuf[3] = tmdHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_CreateImportContentContexts(u32 contentCount, u16* contentIndices)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80D,1,2); // 0x080D0042
	cmdbuf[1] = contentCount;
	cmdbuf[2] = IPC_Desc_Buffer(contentCount * 2, IPC_BUFFER_R);
	cmdbuf[3] = (u32)contentIndices;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallContentBegin(Handle *contentHandle, u16 index)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80E,1,0); // 0x080E0040
	cmdbuf[1] = index;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	if(contentHandle) *contentHandle = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result AM_InstallContentStop(Handle contentHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80F,0,2); // 0x080F0002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = contentHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallContentResume(Handle *contentHandle, u64* resumeOffset, u16 index)
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

Result AM_InstallContentCancel(Handle contentHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x811,0,2); // 0x08110002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = contentHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallContentFinish(Handle contentHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x812,0,2); // 0x08120002
	cmdbuf[1] = IPC_Desc_MoveHandles(1);
	cmdbuf[2] = contentHandle;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_ImportCertificates(u32 cert1Size, void* cert1, u32 cert2Size, void* cert2, u32 cert3Size, void* cert3, u32 cert4Size, void* cert4)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x819,4,8); // 0x08190108
	cmdbuf[1] = cert1Size;
	cmdbuf[2] = cert2Size;
	cmdbuf[3] = cert3Size;
	cmdbuf[4] = cert4Size;
	cmdbuf[5] = IPC_Desc_Buffer(cert1Size, IPC_BUFFER_R);
	cmdbuf[6] = (u32)cert1;
	cmdbuf[7] = IPC_Desc_Buffer(cert2Size, IPC_BUFFER_R);
	cmdbuf[8] = (u32)cert2;
	cmdbuf[9] = IPC_Desc_Buffer(cert3Size, IPC_BUFFER_R);
	cmdbuf[10] = (u32)cert3;
	cmdbuf[11] = IPC_Desc_Buffer(cert4Size, IPC_BUFFER_R);
	cmdbuf[12] = (u32)cert4;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_ImportCertificate(u32 certSize, void* cert)
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

Result AM_CommitImportTitlesAndUpdateFirmwareAuto(FS_MediaType mediaType, u32 titleCount, bool temp, u64* titleIds)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81B,3,2); // 0x081B00C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = temp ? 1 : 0;
	cmdbuf[4] = IPC_Desc_Buffer(titleCount * 8, IPC_BUFFER_R);
	cmdbuf[5] = (u32)titleIds;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAllDemoLaunchInfos(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x827,0,0); // 0x8270000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAllTemporaryTitles(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x16,0,0); // 0x160000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAllExpiredTitles(FS_MediaType mediatype)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1F,1,0); // 0x1F0040
	cmdbuf[1] = mediatype;

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAllTwlTitles(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1D,0,0); // 0x1D0000

	if(R_FAILED(ret = svcSendSyncRequest(amHandle))) return ret;

	return (Result)cmdbuf[1];
}
