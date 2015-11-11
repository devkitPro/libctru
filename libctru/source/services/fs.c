#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/fs.h>
#include <3ds/ipc.h>

static Handle fsuHandle;
static int fsuRefCount;

// used to determine whether or not we should do FSUSER_Initialize on fsuHandle
Handle __get_handle_from_list(char* name);

Result fsInit(void)
{
	Result ret = 0;

	if (AtomicPostIncrement(&fsuRefCount)) return 0;

	ret = srvGetServiceHandle(&fsuHandle, "fs:USER");
	if (R_SUCCEEDED(ret) && __get_handle_from_list("fs:USER")==0)
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

Handle *fsGetSessionHandle(void)
{
	return &fsuHandle;
}

FS_path fsMakePath(FS_pathType type, const char *path)
{
	return (FS_path){type, strlen(path)+1, (const u8*)path};
}

Result FSUSER_Initialize(Handle handle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,0,2); // 0x8010002
	cmdbuf[1] = IPC_Desc_CurProcessHandle();

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_OpenFile(Handle *out, FS_archive archive, FS_path fileLowPath, u32 openFlags, u32 attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,7,2); // 0x80201C2
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = fileLowPath.type;
	cmdbuf[5] = fileLowPath.size;
	cmdbuf[6] = openFlags;
	cmdbuf[7] = attributes;
	cmdbuf[8] = IPC_Desc_StaticBuffer(fileLowPath.size,0);
	cmdbuf[9] = (u32)fileLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result FSUSER_OpenFileDirectly(Handle *out, FS_archive archive, FS_path fileLowPath, u32 openFlags, u32 attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,8,4); // 0x8030204
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.id;
	cmdbuf[3] = archive.lowPath.type;
	cmdbuf[4] = archive.lowPath.size;
	cmdbuf[5] = fileLowPath.type;
	cmdbuf[6] = fileLowPath.size;
	cmdbuf[7] = openFlags;
	cmdbuf[8] = attributes;
	cmdbuf[9] = IPC_Desc_StaticBuffer(archive.lowPath.size,2);
	cmdbuf[10] = (u32)archive.lowPath.data;
	cmdbuf[11] = IPC_Desc_StaticBuffer(fileLowPath.size,0);
	cmdbuf[12] = (u32)fileLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result FSUSER_DeleteFile(FS_archive archive, FS_path fileLowPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804,5,2); // 0x8040142
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = fileLowPath.type;
	cmdbuf[5] = fileLowPath.size;
	cmdbuf[6] = IPC_Desc_StaticBuffer(fileLowPath.size ,0);
	cmdbuf[7] = (u32)fileLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_RenameFile(FS_archive srcArchive, FS_path srcFileLowPath, FS_archive destArchive, FS_path destFileLowPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,9,4); // 0x8050244
	cmdbuf[1] = 0;
	cmdbuf[2] = srcArchive.handleLow;
	cmdbuf[3] = srcArchive.handleHigh;
	cmdbuf[4] = srcFileLowPath.type;
	cmdbuf[5] = srcFileLowPath.size;
	cmdbuf[6] = destArchive.handleLow;
	cmdbuf[7] = destArchive.handleHigh;
	cmdbuf[8] = destFileLowPath.type;
	cmdbuf[9] = destFileLowPath.size;
	cmdbuf[10] = IPC_Desc_StaticBuffer(srcFileLowPath.size,1);
	cmdbuf[11] = (u32)srcFileLowPath.data;
	cmdbuf[12] = IPC_Desc_StaticBuffer(destFileLowPath.size,2);
	cmdbuf[13] = (u32)destFileLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteDirectory(FS_archive archive, FS_path dirLowPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x806,5,2); // 0x8060142
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = dirLowPath.type;
	cmdbuf[5] = dirLowPath.size;
	cmdbuf[6] = IPC_Desc_StaticBuffer(dirLowPath.size,0);
	cmdbuf[7] = (u32)dirLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteDirectoryRecursively(FS_archive archive, FS_path dirLowPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x807,5,2); // 0x8070142
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = dirLowPath.type;
	cmdbuf[5] = dirLowPath.size;
	cmdbuf[6] = IPC_Desc_StaticBuffer(dirLowPath.size,0);
	cmdbuf[7] = (u32)dirLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CreateFile(FS_archive archive, FS_path fileLowPath, u32 fileSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x808,8,2); // 0x8080202
	cmdbuf[1]  = 0;
	cmdbuf[2]  = archive.handleLow;
	cmdbuf[3]  = archive.handleHigh;
	cmdbuf[4]  = fileLowPath.type;
	cmdbuf[5]  = fileLowPath.size;
	cmdbuf[6]  = 0;
	cmdbuf[7]  = fileSize;
	cmdbuf[8]  = 0;
	cmdbuf[9]  = IPC_Desc_StaticBuffer(fileLowPath.size,0);
	cmdbuf[10] = (u32)fileLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CreateDirectory(FS_archive archive, FS_path dirLowPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x809,6,2); // 0x8090182
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = dirLowPath.type;
	cmdbuf[5] = dirLowPath.size;
	cmdbuf[6] = 0;
	cmdbuf[7] = IPC_Desc_StaticBuffer(dirLowPath.size,0);
	cmdbuf[8] = (u32)dirLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_RenameDirectory(FS_archive srcArchive, FS_path srcDirLowPath, FS_archive destArchive, FS_path destDirLowPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80A,9,4); // 0x80A0244
	cmdbuf[1] = 0;
	cmdbuf[2] = srcArchive.handleLow;
	cmdbuf[3] = srcArchive.handleHigh;
	cmdbuf[4] = srcDirLowPath.type;
	cmdbuf[5] = srcDirLowPath.size;
	cmdbuf[6] = destArchive.handleLow;
	cmdbuf[7] = destArchive.handleHigh;
	cmdbuf[8] = destDirLowPath.type;
	cmdbuf[9] = destDirLowPath.size;
	cmdbuf[10] = IPC_Desc_StaticBuffer(srcDirLowPath.size,1);
	cmdbuf[11] = (u32)srcDirLowPath.data;
	cmdbuf[12] = IPC_Desc_StaticBuffer(destDirLowPath.size,2);
	cmdbuf[13] = (u32)destDirLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_OpenDirectory(Handle *out, FS_archive archive, FS_path dirLowPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80B,4,2); // 0x80B0102
	cmdbuf[1] = archive.handleLow;
	cmdbuf[2] = archive.handleHigh;
	cmdbuf[3] = dirLowPath.type;
	cmdbuf[4] = dirLowPath.size;
	cmdbuf[5] = IPC_Desc_StaticBuffer(dirLowPath.size,0);
	cmdbuf[6] = (u32)dirLowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result FSUSER_OpenArchive(FS_archive *archive)
{
	if(!archive) return -2;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80C,3,2); // 0x80C00C2
	cmdbuf[1] = archive->id;
	cmdbuf[2] = archive->lowPath.type;
	cmdbuf[3] = archive->lowPath.size;
	cmdbuf[4] = IPC_Desc_StaticBuffer(archive->lowPath.size,0);
	cmdbuf[5] = (u32)archive->lowPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	archive->handleLow  = cmdbuf[2];
	archive->handleHigh = cmdbuf[3];

	return cmdbuf[1];
}

Result FSUSER_CloseArchive(FS_archive *archive)
{
	if(!archive) return -2;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80E,2,0); // 0x80E0080
	cmdbuf[1] = archive->handleLow;
	cmdbuf[2] = archive->handleHigh;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_GetSdmcArchiveResource(u32 *sectorSize, u32 *clusterSize, u32 *numClusters, u32 *freeClusters)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x814,0,0); // 0x8140000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(sectorSize) *sectorSize = cmdbuf[2];
	if(clusterSize) *clusterSize = cmdbuf[3];
	if(numClusters) *numClusters = cmdbuf[4];
	if(freeClusters) *freeClusters = cmdbuf[5];

	return cmdbuf[1];
}

Result FSUSER_GetNandArchiveResource(u32 *sectorSize, u32 *clusterSize, u32 *numClusters, u32 *freeClusters)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x815,0,0); // 0x8150000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(sectorSize) *sectorSize = cmdbuf[2];
	if(clusterSize) *clusterSize = cmdbuf[3];
	if(numClusters) *numClusters = cmdbuf[4];
	if(freeClusters) *freeClusters = cmdbuf[5];

	return cmdbuf[1];
}

Result FSUSER_IsSdmcDetected(u8 *detected)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x817,0,0); // 0x8170000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(detected) *detected = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_GetMediaType(u8* mediatype)
{
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x868,0,0); // 0x8680000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(mediatype) *mediatype = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result FSUSER_IsSdmcWritable(u8 *writable)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x818,0,0); // 0x8180000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(writable) *writable = cmdbuf[2] & 0xFF;

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

Result FSFILE_Read(Handle handle, u32 *bytesRead, u64 offset, void *buffer, u32 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,3,2); // 0x80200C2
	cmdbuf[1] = (u32)offset;
	cmdbuf[2] = (u32)(offset >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[5] = (u32)buffer;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(bytesRead) *bytesRead = cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_Write(Handle handle, u32 *bytesWritten, u64 offset, const void *buffer, u32 size, u32 flushFlags)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,4,2); // 0x8030102
	cmdbuf[1] = (u32)offset;
	cmdbuf[2] = (u32)(offset >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = flushFlags;
	cmdbuf[5] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[6] = (u32)buffer;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(bytesWritten) *bytesWritten = cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_GetSize(Handle handle, u64 *size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804,0,0); // 0x8040000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(size) *size = (u64)cmdbuf[2] | ((u64)cmdbuf[3] << 32);

	return cmdbuf[1];
}

Result FSFILE_SetSize(Handle handle, u64 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,2,0); // 0x8050080
	cmdbuf[1] = (u32)size;
	cmdbuf[2] = (u32)(size >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSFILE_GetAttributes(Handle handle, u32 *attributes)
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

Result FSFILE_Flush(Handle handle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x809,0,0); // 0x8090000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSDIR_Read(Handle handle, u32 *entriesRead, u32 entryCount, FS_dirent *buffer)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,1,2); // 0x8010042
	cmdbuf[1] = entryCount;
	cmdbuf[2] = IPC_Desc_Buffer(entryCount*0x228,IPC_BUFFER_W);
	cmdbuf[3] = (u32)buffer;

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
