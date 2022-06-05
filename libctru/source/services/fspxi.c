#include <string.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/services/fspxi.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/types.h>

Result FSPXI_OpenFile(Handle serviceHandle, FSPXI_File* out, FSPXI_Archive archive, FS_Path path, u32 flags, u32 attributes)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 7, 2); // 0x000101C2
	cmdbuf[1] = 0; // transaction
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32)(archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = flags;
	cmdbuf[7] = attributes;
	cmdbuf[8] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[9] = (u32) path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (out) *out = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return (Result) cmdbuf[1];
}

Result FSPXI_DeleteFile(Handle serviceHandle, FSPXI_Archive archive, FS_Path path)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 5, 2); // 0x00020142
	cmdbuf[1] = 0; // transaction
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32)(archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[7] = (u32) path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_RenameFile(Handle serviceHandle, FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3, 9, 4); // 0x00030244
	cmdbuf[1] = 0; // transaction
	cmdbuf[2] = (u32) srcArchive;
	cmdbuf[3] = (u32)(srcArchive >> 32);
	cmdbuf[4] = srcPath.type;
	cmdbuf[5] = srcPath.size;
	cmdbuf[6] = (u32) dstArchive;
	cmdbuf[7] = (u32)(dstArchive >> 32);
	cmdbuf[8] = dstPath.type;
	cmdbuf[9] = dstPath.size;
	cmdbuf[10] = IPC_Desc_PXIBuffer(srcPath.size, 0, true);
	cmdbuf[11] = (u32) srcPath.data;
	cmdbuf[12] = IPC_Desc_PXIBuffer(dstPath.size, 1, true);
	cmdbuf[13] = (u32) dstPath.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_DeleteDirectory(Handle serviceHandle, FSPXI_Archive archive, FS_Path path)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4, 5, 2); // 0x00040142
	cmdbuf[1] = 0; // transaction
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32)(archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = IPC_Desc_PXIBuffer(path.size, 0, false);
	cmdbuf[7] = (u32) path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CreateFile(Handle serviceHandle, FSPXI_Archive archive, FS_Path path, u32 attributes, u64 fileSize)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5, 8, 2); // 0x00050202
	cmdbuf[1] = 0; // transaction
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32)(archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = attributes;
	cmdbuf[7] = (u32)fileSize;
	cmdbuf[8] = (u32)(fileSize >> 32);
	cmdbuf[9] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[10] = (u32) path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CreateDirectory(Handle serviceHandle, FSPXI_Archive archive, FS_Path path, u32 attributes)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6, 6, 2); // 0x00060182
	cmdbuf[1] = 0; // transaction
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32)(archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = attributes;
	cmdbuf[7] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[8] = (u32) path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_RenameDirectory(Handle serviceHandle, FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7, 9, 4); // 0x00070244
	cmdbuf[1] = 0; // transaction
	cmdbuf[2] = (u32) srcArchive;
	cmdbuf[3] = (u32)(srcArchive >> 32);
	cmdbuf[4] = srcPath.type;
	cmdbuf[5] = srcPath.size;
	cmdbuf[6] = (u32) dstArchive;
	cmdbuf[7] = (u32)(dstArchive >> 32);
	cmdbuf[8] = dstPath.type;
	cmdbuf[9] = dstPath.size;
	cmdbuf[10] = IPC_Desc_PXIBuffer(srcPath.size, 0, true);
	cmdbuf[11] = (u32) srcPath.data;
	cmdbuf[12] = IPC_Desc_PXIBuffer(dstPath.size, 1, true);
	cmdbuf[13] = (u32) dstPath.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_OpenDirectory(Handle serviceHandle, FSPXI_Directory* out, FSPXI_Archive archive, FS_Path path)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8, 4, 2); // 0x00080102
	cmdbuf[1] = 0; // transaction
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32)(archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[8] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[9] = (u32) path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (out) *out = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return (Result) cmdbuf[1];
}

Result FSPXI_ReadFile(Handle serviceHandle, FSPXI_File file, u32* bytesRead, u64 offset, void* buffer, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9, 5, 2); // 0x00090142
	cmdbuf[1] = (u32) file;
	cmdbuf[2] = (u32)(file >> 32);
	cmdbuf[3] = (u32) offset;
	cmdbuf[4] = (u32)(offset >> 32);
	cmdbuf[5] = size;
	cmdbuf[6] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[7] = (u32) buffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(bytesRead) *bytesRead = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_CalculateFileHashSHA256(Handle serviceHandle, FSPXI_File file, void* buffer, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA, 3, 2); // 0x000A00C2
	cmdbuf[1] = (u32) file;
	cmdbuf[2] = (u32)(file >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[5] = (u32) buffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_WriteFile(Handle serviceHandle, FSPXI_File file, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,6,2); // 0x000B0182
	cmdbuf[1] = (u32) file;
	cmdbuf[2] = (u32)(file >> 32);
	cmdbuf[3] = (u32) offset;
	cmdbuf[4] = (u32) (offset >> 32);
	cmdbuf[5] = size;
	cmdbuf[6] = flags;
	cmdbuf[7] = IPC_Desc_PXIBuffer(size, 0, true);
	cmdbuf[8] = (u32) buffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(bytesWritten) *bytesWritten = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_CalcSavegameMAC(Handle serviceHandle, FSPXI_File file, const void* inBuffer, u32 inSize, void* outBuffer, u32 outSize)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC, 4, 4);
	cmdbuf[1] = (u32) file;
	cmdbuf[2] = (u32)(file >> 32);
	cmdbuf[3] = outSize;
	cmdbuf[4] = inSize;
	cmdbuf[5] = IPC_Desc_PXIBuffer(inSize, 0, true);
	cmdbuf[6] = (u32) inBuffer;
	cmdbuf[7] = IPC_Desc_PXIBuffer(outSize, 1, false);
	cmdbuf[8] = (u32) outBuffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetFileSize(Handle serviceHandle, FSPXI_File file, u64* size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD, 2, 0); // 0x000D0080
	cmdbuf[1] = (u32) file;
	cmdbuf[2] = (u32) (file >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (size) *size = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return (Result) cmdbuf[1];
}

Result FSPXI_SetFileSize(Handle serviceHandle, FSPXI_File file, u64 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xE, 4, 0); // 0x000E0100
	cmdbuf[1] = (u32) size;
	cmdbuf[2] = (u32) (size >> 32);
	cmdbuf[3] = (u32) file;
	cmdbuf[4] = (u32) (file >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CloseFile(Handle serviceHandle, FSPXI_File file)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xF, 2, 0); // 0x000F0080
	cmdbuf[1] = (u32) file;
	cmdbuf[2] = (u32) (file >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_ReadDirectory(Handle serviceHandle, FSPXI_Directory directory, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x10, 3, 2); // 0x001000C2
	cmdbuf[1] = (u32) directory;
	cmdbuf[2] = (u32) (directory >> 32);
	cmdbuf[3] = entryCount;
	cmdbuf[4] = IPC_Desc_PXIBuffer(sizeof(FS_DirectoryEntry) * entryCount, 0, false);
	cmdbuf[5] = (u32) entries;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(entriesRead) *entriesRead = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_CloseDirectory(Handle serviceHandle, FSPXI_Directory directory)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x11, 2, 0); // 0x00110080
	cmdbuf[1] = (u32) directory;
	cmdbuf[2] = (u32) (directory >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_OpenArchive(Handle serviceHandle, FSPXI_Archive* archive, FS_ArchiveID archiveID, FS_Path path)
{
	if (!archive) return -2;

	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x12, 3, 2);
	cmdbuf[1] = archiveID;
	cmdbuf[2] = path.type;
	cmdbuf[3] = path.size;
	cmdbuf[4] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[5] = (u32)path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	*archive = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return (Result) cmdbuf[1];
}

Result FSPXI_HasFile(Handle serviceHandle, FSPXI_Archive archive, bool* out, FS_Path path)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x13, 4, 2); // 0x00130102
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);
	cmdbuf[3] = path.type;
	cmdbuf[4] = path.size;
	cmdbuf[5] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[6] = (u32)path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (out) *out = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_HasDirectory(Handle serviceHandle, FSPXI_Archive archive, bool* out, FS_Path path)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x14, 4, 2); // 0x00140102
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);
	cmdbuf[3] = path.type;
	cmdbuf[4] = path.size;
	cmdbuf[5] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[6] = (u32)path.data;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (out) *out = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_CommitSaveData(Handle serviceHandle, FSPXI_Archive archive, u32 id)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x15, 3, 0); // 0x001500C0
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);
	cmdbuf[3] = id;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CloseArchive(Handle serviceHandle, FSPXI_Archive archive)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x16, 2, 0); // 0x00160080
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_Unknown0x17(Handle serviceHandle, FSPXI_Archive archive, bool* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x17, 2, 0); // 0x00170080
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (out) *out = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_GetCardType(Handle serviceHandle, FS_CardType* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x18, 0, 0); // 0x00180000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(out) *out = cmdbuf[2] & 0xFF;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetSdmcArchiveResource(Handle serviceHandle, FS_ArchiveResource* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x19, 0, 0); // 0x00190000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(out)
	{
		out->sectorSize = cmdbuf[2];
		out->clusterSize = cmdbuf[3];
		out->totalClusters = cmdbuf[4];
		out->freeClusters = cmdbuf[5];
	}

	return (Result) cmdbuf[1];
}

Result FSPXI_GetNandArchiveResource(Handle serviceHandle, FS_ArchiveResource* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1A, 0, 0); // 0x001A0000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(out)
	{
		out->sectorSize = cmdbuf[2];
		out->clusterSize = cmdbuf[3];
		out->totalClusters = cmdbuf[4];
		out->freeClusters = cmdbuf[5];
	}

	return (Result) cmdbuf[1];
}

Result FSPXI_GetSdmcFatFsError(Handle serviceHandle, u32* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1B, 0, 0); // 0x001B0000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(out) *out = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_IsSdmcDetected(Handle serviceHandle, bool* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1C, 0, 0); // 0x001C0000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(out) *out = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_IsSdmcWritable(Handle serviceHandle, bool* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1D, 0, 0); // 0x001D0000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(out) *out = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_GetSdmcCid(Handle serviceHandle, void* out, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1E, 0, 0); // 0x001E0000
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[3] = (u32) out;

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetNandCid(Handle serviceHandle, void* out, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1F, 0, 0); // 0x001F0000
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[3] = (u32) out;

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetSdmcSpeedInfo(Handle serviceHandle, FS_SdMmcSpeedInfo* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x20, 0, 0); // 0x00200000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(out) memcpy(out, &cmdbuf[2], sizeof(FS_SdMmcSpeedInfo));

	return (Result) cmdbuf[1];
}

Result FSPXI_GetNandSpeedInfo(Handle serviceHandle, FS_SdMmcSpeedInfo* out)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x21, 0, 0); // 0x00210000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(out) memcpy(out, &cmdbuf[2], sizeof(FS_SdMmcSpeedInfo));

	return (Result) cmdbuf[1];
}

Result FSPXI_GetSdmcLog(Handle serviceHandle, void* out, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x22, 1, 2); // 0x00220042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[3] = (u32) out;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetNandLog(Handle serviceHandle, void* out, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x23, 1, 2); // 0x00230042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[3] = (u32) out;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_ClearSdmcLog(Handle serviceHandle)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x24, 0, 0); // 0x00240000

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_ClearNandLog(Handle serviceHandle)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x25, 0, 0); // 0x00250000

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardSlotIsInserted(Handle serviceHandle, bool* inserted)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x26, 0, 0); // 0x00260000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(inserted) *inserted = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_CardSlotPowerOn(Handle serviceHandle, bool* status)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x27, 0, 0); // 0x00270000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(status) *status = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_CardSlotPowerOff(Handle serviceHandle, bool* status)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x28, 0, 0); // 0x00280000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(status) *status = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_CardSlotGetCardIFPowerStatus(Handle serviceHandle, bool* status)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x29, 0, 0); // 0x00290000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(status) *status = (bool)cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectCommand(Handle serviceHandle, u8 commandId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2A, 1, 0); // 0x002A0040
	cmdbuf[1] = commandId;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectCommandWithAddress(Handle serviceHandle, u8 commandId, u32 address)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2B, 2, 0); // 0x002B0080
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectRead(Handle serviceHandle, u8 commandId, u32 size, void* output)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2C, 2, 2); // 0x002C0082
	cmdbuf[1] = commandId;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_PXIBuffer(size * sizeof(u32), 0, false);
	cmdbuf[4] = (u32) output;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectReadWithAddress(Handle serviceHandle, u8 commandId, u32 address, u32 size, void* output)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2D, 3, 2); // 0x002D00C2
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_PXIBuffer(size * sizeof(u32), 0, false);
	cmdbuf[5] = (u32) output;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectWrite(Handle serviceHandle, u8 commandId, u32 size, const void* input)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2E, 2, 2); // 0x002E0082
	cmdbuf[1] = commandId;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_PXIBuffer(size * sizeof(u32), 0, true);
	cmdbuf[4] = (u32) input;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectWriteWithAddress(Handle serviceHandle, u8 commandId, u32 address, u32 size, const void* input)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2F, 3, 2); // 0x002F00C2
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_PXIBuffer(size * sizeof(u32), 0, true);
	cmdbuf[5] = (u32) input;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectRead_4xIO(Handle serviceHandle, u8 commandId, u32 address, u32 size, void* output)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x30, 3, 2); // 0x003000C2
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_PXIBuffer(size * sizeof(u32), 0, false);
	cmdbuf[5] = (u32) output;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectCpuWriteWithoutVerify(Handle serviceHandle, u32 address, u32 size, const void* input)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x31, 2, 2); // 0x00310082
	cmdbuf[1] = address;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_PXIBuffer(size * sizeof(u32), 0, true);
	cmdbuf[4] = (u32) input;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CardNorDirectSectorEraseWithoutVerify(Handle serviceHandle, u32 address)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x32, 1, 0); // 0x00320040
	cmdbuf[1] = address;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetProductInfo(Handle serviceHandle, FS_ProductInfo* info, FSPXI_Archive archive)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x33, 2, 0); // 0x00330080
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32)(archive >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(info) memcpy(info, &cmdbuf[2], sizeof(FS_ProductInfo));

	return (Result) cmdbuf[1];
}

Result FSPXI_SetCardSpiBaudrate(Handle serviceHandle, FS_CardSpiBaudRate baudRate)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x34, 1, 0); // 0x00340040
	cmdbuf[1] = baudRate;

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_SetCardSpiBusMode(Handle serviceHandle, FS_CardSpiBusMode busMode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x35, 1, 0); // 0x00350040
	cmdbuf[1] = busMode;

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_SendInitializeInfoTo9(Handle serviceHandle, u8 unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x36, 1, 0); // 0x00360040
	cmdbuf[1] = unk;

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CreateExtSaveData(Handle serviceHandle, FS_ExtSaveDataInfo info)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x37, 4, 0);
	memcpy(&cmdbuf[1], &info, sizeof(FS_ExtSaveDataInfo));

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_DeleteExtSaveData(Handle serviceHandle, FS_ExtSaveDataInfo info)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x38, 4, 0);
	memcpy(&cmdbuf[1], &info, sizeof(FS_ExtSaveDataInfo));

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_EnumerateExtSaveData(Handle serviceHandle, u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x39, 4, 2); // 0x00390102
	cmdbuf[1] = idsSize;
	cmdbuf[2] = mediaType;
	cmdbuf[3] = idSize;
	cmdbuf[4] = shared;
	cmdbuf[5] = IPC_Desc_Buffer(idsSize, IPC_BUFFER_W);
	cmdbuf[6] = (u32) ids;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(idsWritten) *idsWritten = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_GetSpecialContentIndex(Handle serviceHandle, u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3A, 4, 0); // 0x003A0100
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) programId;
	cmdbuf[3] = (u32) (programId >> 32);
	cmdbuf[4] = type;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(index) *index = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_GetLegacyRomHeader(Handle serviceHandle, FS_MediaType mediaType, u64 programId, void* header)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3B, 3, 2); // 0x003B00C2
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) programId;
	cmdbuf[3] = (u32) (programId >> 32);
	cmdbuf[4] = IPC_Desc_PXIBuffer(0x378, 0, false);
	cmdbuf[5] = (u32) header;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetLegacyBannerData(Handle serviceHandle, FS_MediaType mediaType, u64 programId, void* banner, u8 unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3C, 4, 2); // 0x003C0102
	cmdbuf[1] = mediaType;
	cmdbuf[2] = (u32) programId;
	cmdbuf[3] = (u32) (programId >> 32);
	cmdbuf[4] = unk;
	cmdbuf[5] = IPC_Desc_PXIBuffer(0x23c0, 0, false);
	cmdbuf[6] = (u32) banner;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_FormatCardNorDevice(Handle serviceHandle, u32 unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3D, 1, 0); // 0x003D0040
	cmdbuf[1] = unk;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_DeleteSdmcRoot(Handle serviceHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3E, 0, 0); // 0x003E0000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_DeleteAllExtSaveDataOnNand(Handle serviceHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3F, 1, 0); // 0x003F0000
	cmdbuf[1] = 0; // Transaction

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_InitializeCtrFilesystem(Handle serviceHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40, 0, 0); // 0x00400000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_CreateSeed(Handle serviceHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x41, 0, 0); // 0x00410000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetSdmcCtrRootPath(Handle serviceHandle, u16* out, u32 length)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x42, 1, 2); // 0x00420042
	cmdbuf[1] = length;
	cmdbuf[2] = IPC_Desc_PXIBuffer(length, 0, false);
	cmdbuf[3] = (u32) out;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetArchiveResource(Handle serviceHandle, FS_ArchiveResource* archiveResource, FS_SystemMediaType mediaType)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x43, 1, 0); // 0x00430040
	cmdbuf[1] = mediaType;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	memcpy(archiveResource, &cmdbuf[2], sizeof(FS_ArchiveResource));

	return (Result) cmdbuf[1];
}

Result FSPXI_ExportIntegrityVerificationSeed(Handle serviceHandle, FS_IntegrityVerificationSeed* seed)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x44, 0, 2); // 0x00440002
	cmdbuf[1] = IPC_Desc_PXIBuffer(0x130, 0, false);
	cmdbuf[2] = (u32)seed;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_ImportIntegrityVerificationSeed(Handle serviceHandle, const FS_IntegrityVerificationSeed* seed)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x45, 0, 2); // 0x00450002
	cmdbuf[1] = IPC_Desc_PXIBuffer(0x130, 0, true);
	cmdbuf[2] = (u32)seed;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetLegacySubBannerData(Handle serviceHandle, u32 bannerSize, FS_MediaType mediaType, u64 programId, void* banner)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x46, 4, 2); // 0x00460102
	cmdbuf[1] = bannerSize;
	cmdbuf[2] = mediaType;
	cmdbuf[3] = (u32) programId;
	cmdbuf[4] = (u32) (programId >> 32);
	cmdbuf[5] = IPC_Desc_PXIBuffer(bannerSize, 0, false);
	cmdbuf[6] = (u32) banner;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GenerateRandomBytes(Handle serviceHandle, void* buf, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x47, 1, 2); // 0x00470042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[3] = (u32) buf;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_GetFileLastModified(Handle serviceHandle, FSPXI_Archive archive, u64* out, const u16* path, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x48, 3, 2); // 0x004800C2
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_PXIBuffer(size, 0, true);
	cmdbuf[5] = (u32) path;

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (out) *out = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return (Result) cmdbuf[1];
}

Result FSPXI_ReadSpecialFile(Handle serviceHandle, u32* bytesRead, u64 fileOffset, u32 size, void* data)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x49, 4, 2); // 0x00490102
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) fileOffset;
	cmdbuf[3] = (u32) (fileOffset >> 32);
	cmdbuf[4] = size;
	cmdbuf[5] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[6] = (u32) data;

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (bytesRead) *bytesRead = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_GetSpecialFileSize(Handle serviceHandle, u64* fileSize)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4A, 1, 0); // 0x004A0040
	cmdbuf[1] = 0; // Must be 0

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (fileSize) *fileSize = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return (Result) cmdbuf[1];
}

Result FSPXI_StartDeviceMoveAsSource(Handle serviceHandle, FS_DeviceMoveContext* context)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4B, 0, 0); // 0x004B0000

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(context) memcpy(context, &cmdbuf[2], sizeof(FS_DeviceMoveContext));

	return (Result) cmdbuf[1];
}

Result FSPXI_StartDeviceMoveAsDestination(Handle serviceHandle, FS_DeviceMoveContext context, bool clear)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4C, 9, 0); // 0x004C0240
	memcpy(&cmdbuf[1], &context, sizeof(FS_DeviceMoveContext));
	cmdbuf[9] = clear;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_ReadFileSHA256(Handle serviceHandle, FSPXI_File file, u32* bytesRead, u64 offset, void* readBuffer, u32 readBufferSize, void* hashtable, u32 hashtableSize, u32 unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4D, 7, 4); // 0x004D01C4
	cmdbuf[1] = (u32) file;
	cmdbuf[2] = (u32) (file >> 32);
	cmdbuf[3] = (u32) offset;
	cmdbuf[4] = (u32) (offset >> 32);
	cmdbuf[5] = readBufferSize;
	cmdbuf[6] = unk;
	cmdbuf[7] = hashtableSize;
	cmdbuf[8] = IPC_Desc_PXIBuffer(hashtableSize, 0, false);
	cmdbuf[9] = (u32) hashtable;
	cmdbuf[10] = IPC_Desc_PXIBuffer(readBufferSize, 1, false);
	cmdbuf[11] = (u32) readBuffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (bytesRead) *bytesRead = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_WriteFileSHA256(Handle serviceHandle, FSPXI_File file, u32* bytesWritten, u64 offset, const void* writeBuffer, u32 writeBufferSize, void* hashtable, u32 hashtableSize, u32 unk1, u32 unk2)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4E, 8, 4); // 0x004E0204
	cmdbuf[1] = (u32) file;
	cmdbuf[2] = (u32) (file >> 32);
	cmdbuf[3] = (u32) offset;
	cmdbuf[4] = (u32) (offset >> 32);
	cmdbuf[5] = writeBufferSize;
	cmdbuf[6] = unk1;
	cmdbuf[7] = hashtableSize;
	cmdbuf[8] = unk2;
	cmdbuf[9] = IPC_Desc_PXIBuffer(hashtableSize, 0, true);
	cmdbuf[10] = (u32) hashtable;
	cmdbuf[11] = IPC_Desc_PXIBuffer(writeBufferSize, 1, false);
	cmdbuf[12] = (u32) writeBuffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (bytesWritten) *bytesWritten = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_SetCtrCardLatencyParameter(Handle serviceHandle, u64 latency)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4F, 2, 0); // 0x004F0080
	cmdbuf[1] = (u32) latency;
	cmdbuf[2] = (u32) (latency >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_SetPriority(Handle serviceHandle, u32 priority)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x50, 1, 0); // 0x00500040
	cmdbuf[1] = priority;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_SwitchCleanupInvalidSaveData(Handle serviceHandle, bool enable)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x51, 1, 0); // 0x00500040
	cmdbuf[1] = enable;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_EnumerateSystemSaveData(Handle serviceHandle, u32* idsWritten, u32 idsSize, u32* ids)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x54, 1, 2); // 0x00540042
	cmdbuf[1] = idsSize;
	cmdbuf[2] = IPC_Desc_PXIBuffer(idsSize, 0, false);
	cmdbuf[3] = (u32) ids;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if (idsWritten) *idsWritten = cmdbuf[2];

	return (Result) cmdbuf[1];
}

Result FSPXI_ReadNandReport(Handle serviceHandle, void* buffer, u32 size, u32 unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x55, 2, 2); // 0x00550082
	cmdbuf[1] = size;
	cmdbuf[2] = unk;
	cmdbuf[3] = IPC_Desc_PXIBuffer(size, 0, false);
	cmdbuf[4] = (u32) buffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

Result FSPXI_Unknown0x56(Handle serviceHandle, u32 out[4], FS_Archive archive, FS_Path path)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x56, 4, 2); // 0x00560102
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);
	cmdbuf[3] = path.type;
	cmdbuf[4] = path.size;
	cmdbuf[5] = IPC_Desc_PXIBuffer(path.size, 0, true);
	cmdbuf[6] = (u32)path.data;

	if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	out[0] = cmdbuf[2];
	out[1] = cmdbuf[3];
	out[2] = cmdbuf[4];
	out[3] = cmdbuf[5];

	return (Result) cmdbuf[1];
}
