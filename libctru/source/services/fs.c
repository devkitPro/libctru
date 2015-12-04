#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/fs.h>
#include <3ds/ipc.h>
#include <3ds/env.h>
#include "../internal.h"

static Handle fsuHandle;
static int fsuRefCount;

static Handle fsSessionForArchive(FS_ArchiveID arch)
{
	ThreadVars* tv = getThreadVars();
	if (tv->fs_magic == FS_OVERRIDE_MAGIC && (arch != ARCHIVE_SDMC || tv->fs_sdmc))
		return tv->fs_session;
	return fsuHandle;
}

static Handle fsSession(void)
{
	return fsSessionForArchive(0);
}

Result fsInit(void)
{
	Result ret = 0;

	if (AtomicPostIncrement(&fsuRefCount)) return 0;

	ret = srvGetServiceHandle(&fsuHandle, "fs:USER");
	if (R_SUCCEEDED(ret) && envGetHandle("fs:USER") == 0)
	{
		ret = FSUSER_Initialize(fsuHandle);
		if (R_FAILED(ret)) svcCloseHandle(fsuHandle);
	}

	if (R_FAILED(ret)) AtomicDecrement(&fsuRefCount);
	return ret;
}

void fsExit(void)
{
	if (AtomicDecrement(&fsuRefCount)) return;
	svcCloseHandle(fsuHandle);
}

void fsUseSession(Handle session, bool sdmc)
{
	ThreadVars* tv = getThreadVars();
	tv->fs_magic   = FS_OVERRIDE_MAGIC;
	tv->fs_session = session;
	tv->fs_sdmc    = sdmc;
}

void fsEndUseSession(void)
{
	ThreadVars* tv = getThreadVars();
	tv->fs_magic   = 0;
}

FS_Path fsMakePath(FS_PathType type, const void* path)
{
	FS_Path p = { type, 0, path };
	switch (type)
	{
		case PATH_ASCII:
			p.size = strlen((const char*)path)+1;
			break;
		case PATH_UTF16:
		{
			const u16* str = (const u16*)path;
			while (*str++) p.size++;
			p.size = (p.size+1)*2;
			break;
		}
		case PATH_EMPTY:
			p.size = 1;
			p.data = "";
		default:
			break;
	}
	return p;
}

Handle* fsGetSessionHandle(void)
{
	return &fsuHandle;
}

Result FSUSER_Control(FS_Action action, void* input, u32 inputSize, void* output, u32 outputSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401,3,4); // 0x40100C4
	cmdbuf[1] = action;
	cmdbuf[2] = inputSize;
	cmdbuf[3] = outputSize;
	cmdbuf[4] = IPC_Desc_Buffer(inputSize, IPC_BUFFER_R);
	cmdbuf[5] = (u32) input;
	cmdbuf[6] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[7] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_Initialize(Handle session)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,0,2); // 0x8010002
	cmdbuf[1] = IPC_Desc_CurProcessHandle();

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(session))) return ret;

	return cmdbuf[1];
}

Result FSUSER_OpenFile(Handle* out, FS_Archive archive, FS_Path path, u32 openFlags, u32 attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,7,2); // 0x80201C2
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive.handle;
	cmdbuf[3] = (u32) (archive.handle >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = openFlags;
	cmdbuf[7] = attributes;
	cmdbuf[8] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[9] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result FSUSER_OpenFileDirectly(Handle* out, FS_Archive archive, FS_Path path, u32 openFlags, u32 attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,8,4); // 0x8030204
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.id;
	cmdbuf[3] = archive.lowPath.type;
	cmdbuf[4] = archive.lowPath.size;
	cmdbuf[5] = path.type;
	cmdbuf[6] = path.size;
	cmdbuf[7] = openFlags;
	cmdbuf[8] = attributes;
	cmdbuf[9] = IPC_Desc_StaticBuffer(archive.lowPath.size, 2);
	cmdbuf[10] = (u32) archive.lowPath.data;
	cmdbuf[11] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[12] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result FSUSER_DeleteFile(FS_Archive archive, FS_Path path)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804,5,2); // 0x8040142
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive.handle;
	cmdbuf[3] = (u32) (archive.handle >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[7] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_RenameFile(FS_Archive srcArchive, FS_Path srcPath, FS_Archive dstArchive, FS_Path dstPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,9,4); // 0x8050244
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) srcArchive.handle;
	cmdbuf[3] = (u32) (srcArchive.handle >> 32);
	cmdbuf[4] = srcPath.type;
	cmdbuf[5] = srcPath.size;
	cmdbuf[6] = (u32) dstArchive.handle;
	cmdbuf[7] = (u32) (dstArchive.handle >> 32);
	cmdbuf[8] = dstPath.type;
	cmdbuf[9] = dstPath.size;
	cmdbuf[10] = IPC_Desc_StaticBuffer(srcPath.size, 1);
	cmdbuf[11] = (u32) srcPath.data;
	cmdbuf[12] = IPC_Desc_StaticBuffer(dstPath.size, 2);
	cmdbuf[13] = (u32) dstPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(srcArchive.id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteDirectory(FS_Archive archive, FS_Path path)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x806,5,2); // 0x8060142
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive.handle;
	cmdbuf[3] = (u32) (archive.handle >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[7] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteDirectoryRecursively(FS_Archive archive, FS_Path path)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x807,5,2); // 0x8070142
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive.handle;
	cmdbuf[3] = (u32) (archive.handle >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[7] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CreateFile(FS_Archive archive, FS_Path path, u32 attributes, u64 fileSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x808,8,2); // 0x8080202
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive.handle;
	cmdbuf[3] = (u32) (archive.handle >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = attributes;
	cmdbuf[7] = (u32) fileSize;
	cmdbuf[8] = (u32) (fileSize >> 32);
	cmdbuf[9] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[10] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CreateDirectory(FS_Archive archive, FS_Path path, u32 attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x809,6,2); // 0x8090182
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive.handle;
	cmdbuf[3] = (u32) (archive.handle >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = attributes;
	cmdbuf[7] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[8] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_RenameDirectory(FS_Archive srcArchive, FS_Path srcPath, FS_Archive dstArchive, FS_Path dstPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80A,9,4); // 0x80A0244
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) srcArchive.handle;
	cmdbuf[3] = (u32) (srcArchive.handle >> 32);
	cmdbuf[4] = srcPath.type;
	cmdbuf[5] = srcPath.size;
	cmdbuf[6] = (u32) dstArchive.handle;
	cmdbuf[7] = (u32) (dstArchive.handle >> 32);
	cmdbuf[8] = dstPath.type;
	cmdbuf[9] = dstPath.size;
	cmdbuf[10] = IPC_Desc_StaticBuffer(srcPath.size, 1);
	cmdbuf[11] = (u32) srcPath.data;
	cmdbuf[12] = IPC_Desc_StaticBuffer(dstPath.size, 2);
	cmdbuf[13] = (u32) dstPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(srcArchive.id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_OpenDirectory(Handle* out, FS_Archive archive, FS_Path path)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80B,4,2); // 0x80B0102
	cmdbuf[1] = (u32) archive.handle;
	cmdbuf[2] = (u32) (archive.handle >> 32);
	cmdbuf[3] = path.type;
	cmdbuf[4] = path.size;
	cmdbuf[5] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[6] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result FSUSER_OpenArchive(FS_Archive* archive)
{
	if(!archive) return -2;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80C,3,2); // 0x80C00C2
	cmdbuf[1] = archive->id;
	cmdbuf[2] = archive->lowPath.type;
	cmdbuf[3] = archive->lowPath.size;
	cmdbuf[4] = IPC_Desc_StaticBuffer(archive->lowPath.size, 0);
	cmdbuf[5] = (u32) archive->lowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive->id)))) return ret;

	archive->handle = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return cmdbuf[1];
}

Result FSUSER_ControlArchive(FS_Archive archive, FS_ArchiveAction action, void* input, u32 inputSize, void* output, u32 outputSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80D,5,4); // 0x80D0144
	cmdbuf[1] = (u32) archive.handle;
	cmdbuf[2] = (u32) (archive.handle >> 32);
	cmdbuf[3] = action;
	cmdbuf[4] = inputSize;
	cmdbuf[5] = outputSize;
	cmdbuf[6] = IPC_Desc_Buffer(inputSize, IPC_BUFFER_R);
	cmdbuf[7] = (u32) input;
	cmdbuf[8] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[9] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CloseArchive(FS_Archive* archive)
{
	if(!archive) return -2;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80E,2,0); // 0x80E0080
	cmdbuf[1] = (u32) archive->handle;
	cmdbuf[2] = (u32) (archive->handle >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive->id)))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetFreeBytes(u64* freeBytes, FS_Archive archive)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x812,2,0); // 0x8120080
	cmdbuf[1] = (u32) archive.handle;
	cmdbuf[2] = (u32) (archive.handle >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	if(freeBytes) *freeBytes = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return cmdbuf[1];
}

Result FSUSER_GetCardType(FS_CardType* type)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x813,0,0); // 0x8130000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(type) *type = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_GetSdmcArchiveResource(FS_ArchiveResource* archiveResource)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x814,0,0); // 0x8140000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(ARCHIVE_SDMC)))) return ret;

	if(archiveResource) memcpy(archiveResource, &cmdbuf[2], sizeof(FS_ArchiveResource));

	return cmdbuf[1];
}

Result FSUSER_GetNandArchiveResource(FS_ArchiveResource* archiveResource)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x815,0,0); // 0x8150000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(archiveResource) memcpy(archiveResource, &cmdbuf[2], sizeof(FS_ArchiveResource));

	return cmdbuf[1];
}

Result FSUSER_GetSdmcFatfsError(u32* error)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x816,0,0); // 0x8160000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(error) *error = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_IsSdmcDetected(bool *detected)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x817,0,0); // 0x8170000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(detected) *detected = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_IsSdmcWritable(bool *writable)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x818,0,0); // 0x8180000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(writable) *writable = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_GetSdmcCid(u8* out, u32 length)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x819,1,2); // 0x8190042
	cmdbuf[1] = length;
	cmdbuf[2] = IPC_Desc_Buffer(length, IPC_BUFFER_W);
	cmdbuf[3] = (u32) out;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetNandCid(u8* out, u32 length)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81A,1,2); // 0x81A0042
	cmdbuf[1] = length;
	cmdbuf[2] = IPC_Desc_Buffer(length, IPC_BUFFER_W);
	cmdbuf[3] = (u32) out;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetSdmcSpeedInfo(u32 *speedInfo)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81B,0,0); // 0x81B0000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(speedInfo) *speedInfo = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_GetNandSpeedInfo(u32 *speedInfo)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81C,0,0); // 0x81C0000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(speedInfo) *speedInfo = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_GetSdmcLog(u8* out, u32 length)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81D,1,2); // 0x81D0042
	cmdbuf[1] = length;
	cmdbuf[2] = IPC_Desc_Buffer(length, IPC_BUFFER_W);
	cmdbuf[3] = (u32) out;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetNandLog(u8* out, u32 length)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81E,1,2); // 0x81E0042
	cmdbuf[1] = length;
	cmdbuf[2] = IPC_Desc_Buffer(length, IPC_BUFFER_W);
	cmdbuf[3] = (u32) out;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_ClearSdmcLog(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81F,0,0); // 0x81F0000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_ClearNandLog(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x820,0,0); // 0x8200000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardSlotIsInserted(bool* inserted)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x821,0,0); // 0x8210000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(inserted) *inserted = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_CardSlotPowerOn(bool* status)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x822,0,0); // 0x8220000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(status) *status = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_CardSlotPowerOff(bool* status)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x823,0,0); // 0x8230000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(status) *status = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_CardSlotGetCardIFPowerStatus(bool* status)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x824,0,0); // 0x8240000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(status) *status = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectCommand(u8 commandId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x825,1,0); // 0x8250040
	cmdbuf[1] = commandId;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectCommandWithAddress(u8 commandId, u32 address)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x826,2,0); // 0x8260080
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectRead(u8 commandId, u32 size, u8* output)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x827,2,2); // 0x8270082
	cmdbuf[1] = commandId;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size * sizeof(u32), IPC_BUFFER_W);
	cmdbuf[4] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectReadWithAddress(u8 commandId, u32 address, u32 size, u8* output)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x828,3,2); // 0x82800C2
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_Buffer(size * sizeof(u32), IPC_BUFFER_W);
	cmdbuf[5] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectWrite(u8 commandId, u32 size, u8* input)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x829,2,2); // 0x8290082
	cmdbuf[1] = commandId;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size * sizeof(u32), IPC_BUFFER_R);
	cmdbuf[4] = (u32) input;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectWriteWithAddress(u8 commandId, u32 address, u32 size, u8* input)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x82A,3,2); // 0x82A00C2
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_Buffer(size * sizeof(u32), IPC_BUFFER_R);
	cmdbuf[5] = (u32) input;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectRead_4xIO(u8 commandId, u32 address, u32 size, u8* output)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x82B,3,2); // 0x82B00C2
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_Buffer(size * sizeof(u32), IPC_BUFFER_W);
	cmdbuf[5] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectCpuWriteWithoutVerify(u32 address, u32 size, u8* input)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x82C,2,2); // 0x82C0082
	cmdbuf[1] = address;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size * sizeof(u32), IPC_BUFFER_R);
	cmdbuf[4] = (u32) input;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CardNorDirectSectorEraseWithoutVerify(u32 address)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x82D,1,0); // 0x82D0040
	cmdbuf[1] = address;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetProductInfo(FS_ProductInfo* info, u32 processId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x82E,1,0); // 0x82E0040
	cmdbuf[1] = processId;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(info) memcpy(info, &cmdbuf[2], sizeof(FS_ProductInfo));

	return cmdbuf[1];
}

Result FSUSER_GetProgramLaunchInfo(FS_ProgramInfo* info, u32 processId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x82F,1,0); // 0x82F0040
	cmdbuf[1] = processId;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(info) memcpy(info, &cmdbuf[2], sizeof(FS_ProgramInfo));

	return cmdbuf[1];
}

Result FSUSER_SetCardSpiBaudRate(FS_CardSpiBaudRate baudRate)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x837,1,0); // 0x8370040
	cmdbuf[1] = baudRate;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_SetCardSpiBusMode(FS_CardSpiBusMode busMode)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x838,1,0); // 0x8380040
	cmdbuf[1] = busMode;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_SendInitializeInfoTo9(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x839,0,0); // 0x8390000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetSpecialContentIndex(u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x83A,4,0); // 0x83A0100
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) programId;
	cmdbuf[3] = (u32) (programId >> 32);
	cmdbuf[4] = type;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(index) *index = cmdbuf[2] & 0xFFFF;

	return cmdbuf[1];
}

Result FSUSER_GetLegacyRomHeader(FS_MediaType mediaType, u64 programId, u8* header)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x83B,3,2); // 0x83B00C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) programId;
	cmdbuf[3] = (u32) (programId >> 32);
	cmdbuf[4] = IPC_Desc_Buffer(0x3B4, IPC_BUFFER_W);
	cmdbuf[5] = (u32) header;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetLegacyBannerData(FS_MediaType mediaType, u64 programId, u8* banner)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x83C,3,2); // 0x83C00C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) programId;
	cmdbuf[3] = (u32) (programId >> 32);
	cmdbuf[4] = IPC_Desc_Buffer(0x23C0, IPC_BUFFER_W);
	cmdbuf[5] = (u32) banner;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CheckAuthorityToAccessExtSaveData(bool* access, FS_MediaType mediaType, u64 saveId, u32 processId)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x83D,4,0); // 0x83D0100
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) saveId;
	cmdbuf[3] = (u32) (saveId >> 32);
	cmdbuf[4] = processId;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(access) *access = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_QueryTotalQuotaSize(u64* quotaSize, u32 directories, u32 files, u32 fileSizeCount, u64* fileSizes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x83E,3,2); // 0x83E00C2
	cmdbuf[1] = directories;
	cmdbuf[2] = files;
	cmdbuf[3] = fileSizeCount;
	cmdbuf[4] = IPC_Desc_Buffer(fileSizeCount * 8, IPC_BUFFER_R);
	cmdbuf[5] = (u32) fileSizes;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(quotaSize) *quotaSize = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return cmdbuf[1];
}

Result FSUSER_AbnegateAccessRight(u32 accessRight)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x840,1,0); // 0x8400040
	cmdbuf[1] = accessRight;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteSdmcRoot(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x841,0,0); // 0x8410000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteAllExtSaveDataOnNand(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x842,1,0); // 0x8420040
	cmdbuf[1] = 0;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_InitializeCtrFileSystem(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x843,0,0); // 0x8430000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CreateSeed(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x844,0,0); // 0x8440000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetFormatInfo(u32* totalSize, u32* directories, u32* files, bool* duplicateData, FS_ArchiveID archiveId, FS_Path path)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x845,3,2); // 0x84500C2
	cmdbuf[1] = archiveId;
	cmdbuf[2] = path.type;
	cmdbuf[3] = path.size;
	cmdbuf[4] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[5] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archiveId)))) return ret;

	if(totalSize) *totalSize = cmdbuf[2];
	if(directories) *directories = cmdbuf[3];
	if(files) *files = cmdbuf[4];
	if(duplicateData) *duplicateData = cmdbuf[5] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_GetLegacyRomHeader2(u32 headerSize, FS_MediaType mediaType, u64 programId, u8* header)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x846,4,2); // 0x8460102
	cmdbuf[1] = headerSize;
	cmdbuf[2] = mediaType;
	cmdbuf[3] = (u32) programId;
	cmdbuf[4] = (u32) (programId >> 32);
	cmdbuf[5] = IPC_Desc_Buffer(headerSize, IPC_BUFFER_W);
	cmdbuf[6] = (u32) header;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetSdmcCtrRootPath(u8* out, u32 length)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x848,1,2); // 0x8480042
	cmdbuf[1] = length;
	cmdbuf[2] = IPC_Desc_Buffer(length, IPC_BUFFER_W);
	cmdbuf[3] = (u32) out;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetArchiveResource(FS_ArchiveResource* archiveResource, FS_MediaType mediaType)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x849,1,0); // 0x8490040
	cmdbuf[1] = mediaType;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(archiveResource) memcpy(archiveResource, &cmdbuf[2], sizeof(FS_ArchiveResource));

	return cmdbuf[1];
}

Result FSUSER_ExportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x84A,0,2); // 0x84A0002
	cmdbuf[2] = IPC_Desc_Buffer(sizeof(FS_IntegrityVerificationSeed), IPC_BUFFER_W);
	cmdbuf[3] = (u32) seed;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_ImportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x84B,0,2); // 0x84B0002
	cmdbuf[2] = IPC_Desc_Buffer(sizeof(FS_IntegrityVerificationSeed), IPC_BUFFER_R);
	cmdbuf[3] = (u32) seed;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_FormatSaveData(FS_ArchiveID archiveId, FS_Path path, u32 blocks, u32 directories, u32 files, u32 directoryBuckets, u32 fileBuckets, bool duplicateData)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x84C,9,2); // 0x84C0242
	cmdbuf[1] = archiveId;
	cmdbuf[2] = path.type;
	cmdbuf[3] = path.size;
	cmdbuf[4] = blocks;
	cmdbuf[5] = directories;
	cmdbuf[6] = files;
	cmdbuf[7] = directoryBuckets;
	cmdbuf[8] = fileBuckets;
	cmdbuf[9] = duplicateData;
	cmdbuf[10] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[11] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetLegacySubBannerData(u32 bannerSize, FS_MediaType mediaType, u64 programId, u8* banner)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x84D,4,2); // 0x84D0102
	cmdbuf[1] = bannerSize;
	cmdbuf[2] = mediaType;
	cmdbuf[3] = (u32) programId;
	cmdbuf[4] = (u32) (programId >> 32);
	cmdbuf[5] = IPC_Desc_Buffer(bannerSize, IPC_BUFFER_W);
	cmdbuf[6] = (u32) banner;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_ReadSpecialFile(u32* bytesRead, u64 fileOffset, u32 size, u8* data)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x84F,4,2); // 0x84F0102
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) fileOffset;
	cmdbuf[3] = (u32) (fileOffset >> 32);
	cmdbuf[4] = size;
	cmdbuf[5] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[6] = (u32) data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(bytesRead) *bytesRead = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_GetSpecialFileSize(u64* fileSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x850,1,0); // 0x8500040
	cmdbuf[1] = 0;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(fileSize) *fileSize = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return cmdbuf[1];
}

Result FSUSER_CreateExtSaveData(FS_ExtSaveDataInfo info, u32 directories, u32 files, u64 sizeLimit, u32 smdhSize, u8* smdh)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x851,9,2); // 0x8510242
	memcpy(&cmdbuf[1], &info, sizeof(FS_ExtSaveDataInfo));
	cmdbuf[5] = directories;
	cmdbuf[6] = files;
	cmdbuf[7] = (u32) sizeLimit;
	cmdbuf[8] = (u32) (sizeLimit >> 32);
	cmdbuf[9] = smdhSize;
	cmdbuf[10] = IPC_Desc_Buffer(smdhSize, IPC_BUFFER_R);
	cmdbuf[11] = (u32) smdh;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteExtSaveData(FS_ExtSaveDataInfo info)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x852,4,0); // 0x8520100
	memcpy(&cmdbuf[1], &info, sizeof(FS_ExtSaveDataInfo));

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_ReadExtSaveDataIcon(u32* bytesRead, FS_ExtSaveDataInfo info, u32 smdhSize, u8* smdh)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x853,5,2); // 0x8530142
	memcpy(&cmdbuf[1], &info, sizeof(FS_ExtSaveDataInfo));
	cmdbuf[5] = smdhSize;
	cmdbuf[6] = IPC_Desc_Buffer(smdhSize, IPC_BUFFER_W);
	cmdbuf[7] = (u32) smdh;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(bytesRead) *bytesRead = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_GetExtDataBlockSize(u64* totalBlocks, u64* freeBlocks, u32* blockSize, FS_ExtSaveDataInfo info)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x854,4,0); // 0x8540100
	memcpy(&cmdbuf[1], &info, sizeof(FS_ExtSaveDataInfo));

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(totalBlocks) *totalBlocks = cmdbuf[2] | ((u64) cmdbuf[3] << 32);
	if(freeBlocks) *freeBlocks = cmdbuf[4] | ((u64) cmdbuf[5] << 32);
	if(blockSize) *blockSize = cmdbuf[6];

	return cmdbuf[1];
}

Result FSUSER_EnumerateExtSaveData(u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x855,4,2); // 0x8550102
	cmdbuf[1] = idsSize;
	cmdbuf[2] = mediaType;
	cmdbuf[3] = idSize;
	cmdbuf[4] = shared;
	cmdbuf[5] = IPC_Desc_Buffer(idsSize, IPC_BUFFER_W);
	cmdbuf[6] = (u32) ids;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(idsWritten) *idsWritten = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_CreateSystemSaveData(FS_SystemSaveDataInfo info, u32 totalSize, u32 blockSize, u32 directories, u32 files, u32 directoryBuckets, u32 fileBuckets, bool duplicateData)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x856,9,0); // 0x8560240
	memcpy(&cmdbuf[1], &info, sizeof(FS_SystemSaveDataInfo));
	cmdbuf[3] = totalSize;
	cmdbuf[4] = blockSize;
	cmdbuf[5] = directories;
	cmdbuf[6] = files;
	cmdbuf[7] = directoryBuckets;
	cmdbuf[8] = fileBuckets;
	cmdbuf[9] = duplicateData;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteSystemSaveData(FS_SystemSaveDataInfo info)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x857,2,0); // 0x8570080
	memcpy(&cmdbuf[1], &info, sizeof(FS_SystemSaveDataInfo));

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_StartDeviceMoveAsSource(FS_DeviceMoveContext* context)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x858,0,0); // 0x8580000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(context) memcpy(context, &cmdbuf[2], sizeof(FS_DeviceMoveContext));

	return cmdbuf[1];
}

Result FSUSER_StartDeviceMoveAsDestination(FS_DeviceMoveContext context, bool clear)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x859,9,0); // 0x8590240
	memcpy(&cmdbuf[1], &context, sizeof(FS_DeviceMoveContext));
	cmdbuf[9] = clear;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_SetArchivePriority(FS_Archive archive, u32 priority)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x85A,3,0); // 0x85A00C0
	cmdbuf[1] = (u32) archive.handle;
	cmdbuf[2] = (u32) (archive.handle >> 32);
	cmdbuf[3] = priority;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetArchivePriority(u32* priority, FS_Archive archive)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x85B,2,0); // 0x85B0080
	cmdbuf[1] = (u32) archive.handle;
	cmdbuf[2] = (u32) (archive.handle >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSessionForArchive(archive.id)))) return ret;

	if(priority) *priority = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_SetCtrCardLatencyParameter(u64 latency, bool emulateEndurance)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x85C,3,0); // 0x85C00C0
	cmdbuf[1] = (u32) latency;
	cmdbuf[2] = (u32) (latency >> 32);
	cmdbuf[3] = emulateEndurance;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_SwitchCleanupInvalidSaveData(bool enable)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x85F,1,0); // 0x85F0040
	cmdbuf[1] = enable;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_EnumerateSystemSaveData(u32* idsWritten, u32 idsSize, u64* ids)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x860,1,2); // 0x8600042
	cmdbuf[1] = idsSize;
	cmdbuf[2] = IPC_Desc_Buffer(idsSize, IPC_BUFFER_W);
	cmdbuf[3] = (u32) ids;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(idsWritten) *idsWritten = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_InitializeWithSdkVersion(Handle session, u32 version)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x861,1,2); // 0x8610042
	cmdbuf[1] = version;
	cmdbuf[2] = IPC_Desc_CurProcessHandle();

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(session))) return ret;

	return cmdbuf[1];
}

Result FSUSER_SetPriority(u32 priority)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x862,1,0); // 0x8620040
	cmdbuf[1] = priority;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetPriority(u32* priority)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x863,0,0); // 0x8630000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(priority) *priority = cmdbuf[2];

	return cmdbuf[1];
}

Result FSUSER_SetSaveDataSecureValue(u64 value, FS_SecureValueSlot slot, u32 titleUniqueId, u8 titleVariation)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x865,5,0); // 0x8650140
	cmdbuf[1] = (u32) value;
	cmdbuf[2] = (u32) (value >> 32);
	cmdbuf[3] = slot;
	cmdbuf[4] = titleUniqueId;
	cmdbuf[5] = titleVariation;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetSaveDataSecureValue(bool* exists, u64* value, FS_SecureValueSlot slot, u32 titleUniqueId, u8 titleVariation)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x866,3,0); // 0x86600C0
	cmdbuf[1] = slot;
	cmdbuf[2] = titleUniqueId;
	cmdbuf[3] = titleVariation;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(exists) *exists = cmdbuf[2] & 0xFF;
	if(value) *value = cmdbuf[3] | ((u64) cmdbuf[4] << 32);

	return cmdbuf[1];
}

Result FSUSER_ControlSecureSave(FS_SecureSaveAction action, void* input, u32 inputSize, void* output, u32 outputSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x867,3,4); // 0x86700C4
	cmdbuf[1] = action;
	cmdbuf[2] = inputSize;
	cmdbuf[3] = outputSize;
	cmdbuf[4] = IPC_Desc_Buffer(inputSize, IPC_BUFFER_R);
	cmdbuf[5] = (u32) input;
	cmdbuf[6] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[7] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetMediaType(FS_MediaType* mediaType)
{
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x868,0,0); // 0x8680000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsSession()))) return ret;

	if(mediaType) *mediaType = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSFILE_Control(Handle handle, FS_FileAction action, void* input, u32 inputSize, void* output, u32 outputSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401,3,4); // 0x40100C4
	cmdbuf[1] = action;
	cmdbuf[2] = inputSize;
	cmdbuf[3] = outputSize;
	cmdbuf[4] = IPC_Desc_Buffer(inputSize, IPC_BUFFER_R);
	cmdbuf[5] = (u32) input;
	cmdbuf[6] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[7] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSFILE_OpenSubFile(Handle handle, Handle* subFile, u64 offset, u64 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,4,0); // 0x8010100
	cmdbuf[1] = (u32) offset;
	cmdbuf[2] = (u32) (offset >> 32);
	cmdbuf[3] = (u32) size;
	cmdbuf[4] = (u32) (size >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(subFile) *subFile = cmdbuf[3];

	return cmdbuf[1];
}

Result FSFILE_Read(Handle handle, u32* bytesRead, u64 offset, void* buffer, u32 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,3,2); // 0x80200C2
	cmdbuf[1] = (u32) offset;
	cmdbuf[2] = (u32) (offset >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[5] = (u32) buffer;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(bytesRead) *bytesRead = cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_Write(Handle handle, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,4,2); // 0x8030102
	cmdbuf[1] = (u32) offset;
	cmdbuf[2] = (u32) (offset >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = flags;
	cmdbuf[5] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[6] = (u32) buffer;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(bytesWritten) *bytesWritten = cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_GetSize(Handle handle, u64* size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804,0,0); // 0x8040000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(size) *size = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return cmdbuf[1];
}

Result FSFILE_SetSize(Handle handle, u64 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,2,0); // 0x8050080
	cmdbuf[1] = (u32) size;
	cmdbuf[2] = (u32) (size >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSFILE_GetAttributes(Handle handle, u32* attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x806,0,0); // 0x8060000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(attributes) *attributes = cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_SetAttributes(Handle handle, u32 attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x807,1,0); // 0x8070040
	cmdbuf[1] = attributes;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSFILE_Close(Handle handle)
{
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x808,0,0); // 0x8080000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	ret = cmdbuf[1];
	if(R_SUCCEEDED(ret)) ret = svcCloseHandle(handle);

	return ret;
}

Result FSFILE_Flush(Handle handle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x809,0,0); // 0x8090000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSFILE_SetPriority(Handle handle, u32 priority)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80A,1,0); // 0x80A0040
	cmdbuf[1] = priority;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSFILE_GetPriority(Handle handle, u32* priority)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80B,0,0); // 0x80B0000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(priority) *priority = cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_OpenLinkFile(Handle handle, Handle* linkFile)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80C,0,0); // 0x80C0000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(linkFile) *linkFile = cmdbuf[3];

	return cmdbuf[1];
}

Result FSDIR_Control(Handle handle, FS_DirectoryAction action, void* input, u32 inputSize, void* output, u32 outputSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401,3,4); // 0x40100C4
	cmdbuf[1] = action;
	cmdbuf[2] = inputSize;
	cmdbuf[3] = outputSize;
	cmdbuf[4] = IPC_Desc_Buffer(inputSize, IPC_BUFFER_R);
	cmdbuf[5] = (u32) input;
	cmdbuf[6] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[7] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSDIR_Read(Handle handle, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,1,2); // 0x8010042
	cmdbuf[1] = entryCount;
	cmdbuf[2] = IPC_Desc_Buffer(entryCount * sizeof(FS_DirectoryEntry), IPC_BUFFER_W);
	cmdbuf[3] = (u32) entries;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(entriesRead) *entriesRead = cmdbuf[2];

	return cmdbuf[1];
}

Result FSDIR_Close(Handle handle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,0,0); // 0x8020000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	ret = cmdbuf[1];
	if(R_SUCCEEDED(ret)) ret = svcCloseHandle(handle);

	return ret;
}

Result FSDIR_SetPriority(Handle handle, u32 priority)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,1,0); // 0x8030040
	cmdbuf[1] = priority;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSDIR_GetPriority(Handle handle, u32* priority)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804,0,0); // 0x8040000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(priority) *priority = cmdbuf[2];

	return cmdbuf[1];
}
