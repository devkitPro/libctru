#include <string.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/services/fspxi.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/types.h>

static Handle pxifs0Handle;
static int pxifs0RefCount;

static Handle pxifs1Handle;
static int pxifs1RefCount;

static Handle pxifsRHandle;
static int pxifsRRefCount;

static Handle pxifsBHandle;
static int pxifsBRefCount;

Result pxifs0Init(Handle servhandle)
{
     Result res=0;
     if (AtomicPostIncrement(&pxifs0RefCount)) return 0;
     if (servhandle)
     {
         pxifs0Handle = servhandle;
     }
     else
     {
         res = srvGetServiceHandle(&pxifs0Handle, "PxiFS0");
         if (R_FAILED(res)) AtomicDecrement(&pxifs0RefCount);
     }
     return res;
}

Result pxifs1Init(Handle servhandle)
{
     Result res=0;
     if (AtomicPostIncrement(&pxifs1RefCount)) return 0;
     if (servhandle)
     {
         pxifs1Handle = servhandle;
     }
     else
     {
         res = srvGetServiceHandle(&pxifs1Handle, "PxiFS1");
         if (R_FAILED(res)) AtomicDecrement(&pxifs1RefCount);
     }
     return res;
}

Result pxifsRInit(Handle servhandle)
{
     Result res=0;
     if (AtomicPostIncrement(&pxifsRRefCount)) return 0;
     if (servhandle)
     {
         pxifsRHandle = servhandle;
     }
     else
     {
         res = srvGetServiceHandle(&pxifsRHandle, "PxiFSR");
         if (R_FAILED(res)) AtomicDecrement(&pxifsRRefCount);
     }
     return res;
}

Result pxifsBInit(Handle servhandle)
{
     Result res=0;
     if (AtomicPostIncrement(&pxifsBRefCount)) return 0;
     if (servhandle)
     {
         pxifsBHandle = servhandle;
     }
     else
     {
         res = srvGetServiceHandle(&pxifsBHandle, "PxiFSB");
         if (R_FAILED(res)) AtomicDecrement(&pxifsBRefCount);
     }
     return res;
}

void pxifs0Exit(void)
{
    if (AtomicDecrement(&pxifs0RefCount)) return;
    svcCloseHandle(pxifs0Handle);
}

void pxifs1Exit(void)
{
    if (AtomicDecrement(&pxifs1RefCount)) return;
    svcCloseHandle(pxifs1Handle);
}

void pxifsRExit(void)
{
    if (AtomicDecrement(&pxifsRRefCount)) return;
    svcCloseHandle(pxifsRHandle);
}

void pxifsBExit(void)
{
    if (AtomicDecrement(&pxifsBRefCount)) return;
    svcCloseHandle(pxifsBHandle);
}

static Result FSPXI_OpenFile(Handle serviceHandle, FSPXI_File* out, FSPXI_Archive archive, FS_Path path, u32 flags, u32 attributes)
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

static Result FSPXI_DeleteFile(Handle serviceHandle, FSPXI_Archive archive, FS_Path path)
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

static Result FSPXI_RenameFile(Handle serviceHandle, FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath)
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

static Result FSPXI_DeleteDirectory(Handle serviceHandle, FSPXI_Archive archive, FS_Path path)
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

static Result FSPXI_CreateFile(Handle serviceHandle, FSPXI_Archive archive, FS_Path path, u32 attributes, u64 fileSize)
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
    cmdbuf[8] = IPC_Desc_PXIBuffer(path.size, 0, true);
    cmdbuf[9] = (u32) path.data;

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_CreateDirectory(Handle serviceHandle, FSPXI_Archive archive, FS_Path path, u32 attributes)
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

static Result FSPXI_RenameDirectory(Handle serviceHandle, FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath)
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

static Result FSPXI_OpenDirectory(Handle serviceHandle, FSPXI_Directory* out, FSPXI_Archive archive, FS_Path path)
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

static Result FSPXI_ReadFile(Handle serviceHandle, FSPXI_File file, u32* bytesRead, u64 offset, void* buffer, u32 size)
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

static Result FSPXI_CalculateFileHashSHA256(Handle serviceHandle, FSPXI_File file, u32* buffer, u32 size)
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

static Result FSPXI_WriteFile(Handle serviceHandle, FSPXI_File file, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,6,2); // 0x000B0182
    cmdbuf[1] = (u32) file;
    cmdbuf[2] = (u32)(file >> 32);
	cmdbuf[3] = (u32) offset;
	cmdbuf[4] = (u32) (offset >> 32);
	cmdbuf[5] = flags;
	cmdbuf[6] = size;
	cmdbuf[7] = IPC_Desc_PXIBuffer(size, 0, true);
	cmdbuf[8] = (u32) buffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	if(bytesWritten) *bytesWritten = cmdbuf[2];

	return (Result) cmdbuf[1];
}

static Result FSPXI_CalcSavegameMAC(Handle serviceHandle, FSPXI_File file, void* inBuffer, u32 inSize, void* outBuffer, u32 outSize)
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

static Result FSPXI_GetFileSize(Handle serviceHandle, FSPXI_File file, u64* size)
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

static Result FSPXI_SetFileSize(Handle serviceHandle, FSPXI_File file, u64 size)
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

static Result FSPXI_CloseFile(Handle serviceHandle, FSPXI_File file)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0xF, 2, 0); // 0x000F0080
    cmdbuf[1] = (u32) file;
    cmdbuf[2] = (u32) (file >> 32);

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_ReadDirectory(Handle serviceHandle, FSPXI_Directory directory, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries)
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

static Result FSPXI_CloseDirectory(Handle serviceHandle, FSPXI_Directory directory)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x11, 2, 0); // 0x00110080
    cmdbuf[1] = (u32) directory;
    cmdbuf[2] = (u32) (directory >> 32);

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_OpenArchive(Handle serviceHandle, FSPXI_Archive* archive, FS_ArchiveID archiveID, FS_Path path)
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

static Result FSPXI_0x13(Handle serviceHandle, FSPXI_Archive archive, u8* out, FS_Path path)
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

    if (out) *out = (u8)cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_0x14(Handle serviceHandle, FSPXI_Archive archive, u32* out, FS_Path path)
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

    if (out) *out = (u8)cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_CommitSaveData(Handle serviceHandle, FSPXI_Archive archive, u32 id)
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

static Result FSPXI_CloseArchive(Handle serviceHandle, FSPXI_Archive archive)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x16, 2, 0); // 0x00160080
    cmdbuf[1] = (u32) archive;
    cmdbuf[2] = (u32) (archive >> 32);

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_0x17(Handle serviceHandle, FSPXI_Archive archive, bool* out)
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

static Result FSPXI_GetCardType(Handle serviceHandle, FS_CardType* out)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x18, 0, 0); // 0x00180000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(out) *out = cmdbuf[2] & 0xFF;

    return (Result) cmdbuf[1];
}

static Result FSPXI_GetSdmcArchiveResource(Handle serviceHandle, FS_ArchiveResource* out)
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

static Result FSPXI_GetNandArchiveResource(Handle serviceHandle, FS_ArchiveResource* out)
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

static Result FSPXI_GetSdmcFatFsError(Handle serviceHandle, u32* out)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x1B, 0, 0); // 0x001B0000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(out) *out = cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_IsSdmcDetected(Handle serviceHandle, bool* out)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x1C, 0, 0); // 0x001C0000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(out) *out = (bool)cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_IsSdmcWritable(Handle serviceHandle, bool* out)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x1D, 0, 0); // 0x001D0000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(out) *out = (bool)cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_GetSdmcCid(Handle serviceHandle, void* out, u32 size)
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

static Result FSPXI_GetNandCid(Handle serviceHandle, void* out, u32 size)
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

static Result FSPXI_GetSdmcSpeedInfo(Handle serviceHandle, u32* out)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x20, 0, 0); // 0x00200000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(out) *out = cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_GetNandSpeedInfo(Handle serviceHandle, u32* out)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x21, 0, 0); // 0x00210000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(out) *out = cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_GetSdmcLog(Handle serviceHandle, void* out, u32 size)
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

static Result FSPXI_GetNandLog(Handle serviceHandle, void* out, u32 size)
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

static Result FSPXI_ClearSdmcLog(Handle serviceHandle)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x24, 0, 0); // 0x00240000

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_ClearNandLog(Handle serviceHandle)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x25, 0, 0); // 0x00250000

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_CardSlotIsInserted(Handle serviceHandle, bool* inserted)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x26, 0, 0); // 0x00260000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(inserted) *inserted = (bool)cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_CardSlotPowerOn(Handle serviceHandle, bool* status)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x27, 0, 0); // 0x00270000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(status) *status = (bool)cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_CardSlotPowerOff(Handle serviceHandle, bool* status)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x28, 0, 0); // 0x00280000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(status) *status = (bool)cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_CardSlotGetCardIFPowerStatus(Handle serviceHandle, bool* status)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x29, 0, 0); // 0x00290000

    if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(status) *status = (bool)cmdbuf[2];

    return (Result) cmdbuf[1];
}

static Result FSPXI_CardNorDirectCommand(Handle serviceHandle, u8 commandId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2A, 1, 0); // 0x002A0040
	cmdbuf[1] = commandId;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_CardNorDirectCommandWithAddress(Handle serviceHandle, u8 commandId, u32 address)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2B, 2, 0); // 0x002B0080
	cmdbuf[1] = commandId;
	cmdbuf[2] = address;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_CardNorDirectRead(Handle serviceHandle, u8 commandId, u32 size, u8* output)
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

static Result FSPXI_CardNorDirectReadWithAddress(Handle serviceHandle, u8 commandId, u32 address, u32 size, u8* output)
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

static Result FSPXI_CardNorDirectWrite(Handle serviceHandle, u8 commandId, u32 size, u8* input)
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

static Result FSPXI_CardNorDirectWriteWithAddress(Handle serviceHandle, u8 commandId, u32 address, u32 size, u8* input)
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

static Result FSPXI_CardNorDirectRead_4xIO(Handle serviceHandle, u8 commandId, u32 address, u32 size, u8* output)
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

static Result FSPXI_CardNorDirectCpuWriteWithoutVerify(Handle serviceHandle, u32 address, u32 size, u8* input)
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

static Result FSPXI_CardNorDirectSectorEraseWithoutVerify(Handle serviceHandle, u32 address)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x32, 1, 0); // 0x00320040
	cmdbuf[1] = address;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_GetProductInfo(Handle serviceHandle, FS_ProductInfo* info, FSPXI_Archive archive)
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

static Result FSPXI_SetCardSpiBaudrate(Handle serviceHandle, FS_CardSpiBaudRate baudRate)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x34, 1, 0); // 0x00340040
    cmdbuf[1] = baudRate;

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_SetCardSpiBusMode(Handle serviceHandle, FS_CardSpiBusMode busMode)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x35, 1, 0); // 0x00350040
    cmdbuf[1] = busMode;

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_SendInitializeInfoTo9(Handle serviceHandle, u8 unk)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x36, 1, 0); // 0x00360040
    cmdbuf[1] = unk;

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_CreateExtSaveData(Handle serviceHandle, FS_ExtSaveDataInfo info)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x37, 4, 0);
    memcpy(&cmdbuf[1], &info, sizeof(FS_ExtSaveDataInfo));

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_DeleteExtSaveData(Handle serviceHandle, FS_ExtSaveDataInfo info)
{
    Result ret = 0;
    u32* cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x38, 4, 0);
    memcpy(&cmdbuf[1], &info, sizeof(FS_ExtSaveDataInfo));

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    return (Result) cmdbuf[1];
}

static Result FSPXI_EnumerateExtSaveData(Handle serviceHandle, u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids)
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

static Result FSPXI_GetSpecialContentIndex(Handle serviceHandle, u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type)
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

static Result FSPXI_GetLegacyRomHeader(Handle serviceHandle, FS_MediaType mediaType, u64 programId, u8* header)
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

static Result FSPXI_GetLegacyBannerData(Handle serviceHandle, FS_MediaType mediaType, u64 programId, u8* banner, u8 unk)
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

static Result FSPXI_0x3D(Handle serviceHandle, u32 unk)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x3D, 1, 0); // 0x003D0040
    cmdbuf[1] = unk;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_DeleteSdmcRoot(Handle serviceHandle)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x3E, 0, 0); // 0x003E0000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_DeleteAllExtSaveDataOnNand(Handle serviceHandle)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x3F, 1, 0); // 0x003F0000
    cmdbuf[1] = 0; // Transaction

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_InitializeCtrFilesystem(Handle serviceHandle)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x40, 0, 0); // 0x00400000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_CreateSeed(Handle serviceHandle)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x41, 0, 0); // 0x00410000

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_GetSdmcCtrRootPath(Handle serviceHandle, u8* out, u32 length)
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

static Result FSPXI_GetArchiveResource(Handle serviceHandle, FS_ArchiveResource* archiveResource, FS_SystemMediaType mediaType)
{
	Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();
    
	cmdbuf[0] = IPC_MakeHeader(0x43, 1, 0); // 0x00430040
	cmdbuf[1] = mediaType;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    memcpy(archiveResource, &cmdbuf[2], sizeof(FS_ArchiveResource));

	return (Result) cmdbuf[1];
}

static Result FSPXI_ExportIntegrityVerificationSeed(Handle serviceHandle, FS_IntegrityVerificationSeed* seed)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x44, 0, 2); // 0x00440002
    cmdbuf[1] = IPC_Desc_PXIBuffer(0x130, 0, false);
    cmdbuf[2] = (u32)seed;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_ImportIntegrityVerificationSeed(Handle serviceHandle, FS_IntegrityVerificationSeed* seed)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x45, 0, 2); // 0x00450002
    cmdbuf[1] = IPC_Desc_PXIBuffer(0x130, 0, true);
    cmdbuf[2] = (u32)seed;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_GetLegacySubBannerData(Handle serviceHandle, u32 bannerSize, FS_MediaType mediaType, u64 programId, u8* banner)
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

static Result FSPXI_0x47(Handle serviceHandle, void* buf, u32 size)
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

static Result FSPXI_GetFileLastModified(Handle serviceHandle, FSPXI_Archive archive, u64* out, u16* path, u32 size)
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

static Result FSPXI_ReadSpecialFile(Handle serviceHandle, u32* bytesRead, u64 fileOffset, u32 size, u8* data)
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

static Result FSPXI_GetSpecialFileSize(Handle serviceHandle, u64* fileSize)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x4A, 1, 0); // 0x004A0040
    cmdbuf[1] = 0; // Must be 0

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if (fileSize) *fileSize = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

    return (Result) cmdbuf[1];
}

static Result FSPXI_StartDeviceMoveAsSource(Handle serviceHandle, FS_DeviceMoveContext* context)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x4B, 0, 0); // 0x004B0000

    if (R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if(context) memcpy(context, &cmdbuf[2], sizeof(FS_DeviceMoveContext));

    return (Result) cmdbuf[1];
}

static Result FSPXI_StartDeviceMoveAsDestination(Handle serviceHandle, FS_DeviceMoveContext context, bool clear)
{
	Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4C, 9, 0); // 0x004C0240
	memcpy(&cmdbuf[1], &context, sizeof(FS_DeviceMoveContext));
	cmdbuf[9] = clear;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_ReadFileSHA256(Handle serviceHandle, FSPXI_File file, u32* bytesRead, u64 offset, void* readBuffer, u32 readBufferSize, void* hashtable, u32 hashtableSize, u32 unk)
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
    cmdbuf[8] = IPC_Desc_PXIBuffer(hashtableSize, 0, true);
    cmdbuf[9] = (u32) hashtable;
    cmdbuf[10] = IPC_Desc_PXIBuffer(readBufferSize, 1, false);
    cmdbuf[11] = (u32) readBuffer;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

    if (bytesRead) *bytesRead = cmdbuf[2];

	return (Result) cmdbuf[1];
}

static Result FSPXI_WriteFileSHA256(Handle serviceHandle, FSPXI_File file, u32* bytesWritten, u64 offset, void* writeBuffer, u32 writeBufferSize, void* hashtable, u32 hashtableSize, u32 unk1, u32 unk2)
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

static Result FSPXI_0x4F(Handle serviceHandle, u64 unk)
{
	Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x4F, 2, 0); // 0x004F0080
    cmdbuf[1] = (u32) unk;
    cmdbuf[2] = (u32) (unk >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_SetPriority(Handle serviceHandle, u32 priority)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x50, 1, 0); // 0x00500040
    cmdbuf[1] = priority;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_SwitchCleanupInvalidSaveData(Handle serviceHandle, bool enable)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x51, 1, 0); // 0x00500040
    cmdbuf[1] = enable;

	if(R_FAILED(ret = svcSendSyncRequest(serviceHandle))) return ret;

	return (Result) cmdbuf[1];
}

static Result FSPXI_EnumerateSystemSaveData(Handle serviceHandle, u32* idsWritten, u32 idsSize, u32* ids)
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

static Result FSPXI_ReadNandReport(Handle serviceHandle, void* buffer, u32 size, u32 unk)
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

static Result FSPXI_0x56(Handle serviceHandle, u32 (*out)[4], FS_Archive archive, FS_Path path)
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

    (*out)[0] = cmdbuf[2];
    (*out)[1] = cmdbuf[3];
    (*out)[2] = cmdbuf[4];
    (*out)[3] = cmdbuf[5];

    return (Result) cmdbuf[1];
}

Result PXIFS0_OpenFile(FSPXI_File* out, FSPXI_Archive archive, FS_Path path, u32 flags, u32 attributes) { return FSPXI_OpenFile(pxifs0Handle, out, archive, path, flags, attributes); }
Result PXIFS1_OpenFile(FSPXI_File* out, FSPXI_Archive archive, FS_Path path, u32 flags, u32 attributes) { return FSPXI_OpenFile(pxifs1Handle, out, archive, path, flags, attributes); }
Result PXIFSR_OpenFile(FSPXI_File* out, FSPXI_Archive archive, FS_Path path, u32 flags, u32 attributes) { return FSPXI_OpenFile(pxifsRHandle, out, archive, path, flags, attributes); }
Result PXIFSB_OpenFile(FSPXI_File* out, FSPXI_Archive archive, FS_Path path, u32 flags, u32 attributes) { return FSPXI_OpenFile(pxifsBHandle, out, archive, path, flags, attributes); }

Result PXIFS0_DeleteFile(FSPXI_Archive archive, FS_Path path) { return FSPXI_DeleteFile(pxifs0Handle, archive, path); }
Result PXIFS1_DeleteFile(FSPXI_Archive archive, FS_Path path) { return FSPXI_DeleteFile(pxifs1Handle, archive, path); }
Result PXIFSR_DeleteFile(FSPXI_Archive archive, FS_Path path) { return FSPXI_DeleteFile(pxifsRHandle, archive, path); }
Result PXIFSB_DeleteFile(FSPXI_Archive archive, FS_Path path) { return FSPXI_DeleteFile(pxifsBHandle, archive, path); }

Result PXIFS0_RenameFile(FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath) { return FSPXI_RenameFile(pxifs0Handle, srcArchive, srcPath, dstArchive, dstPath); }
Result PXIFS1_RenameFile(FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath) { return FSPXI_RenameFile(pxifs1Handle, srcArchive, srcPath, dstArchive, dstPath); }
Result PXIFSR_RenameFile(FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath) { return FSPXI_RenameFile(pxifsRHandle, srcArchive, srcPath, dstArchive, dstPath); }
Result PXIFSB_RenameFile(FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath) { return FSPXI_RenameFile(pxifsBHandle, srcArchive, srcPath, dstArchive, dstPath); }

Result PXIFS0_DeleteDirectory(FSPXI_Archive archive, FS_Path path) { return FSPXI_DeleteDirectory(pxifs0Handle, archive, path); }
Result PXIFS1_DeleteDirectory(FSPXI_Archive archive, FS_Path path) { return FSPXI_DeleteDirectory(pxifs1Handle, archive, path); }
Result PXIFSR_DeleteDirectory(FSPXI_Archive archive, FS_Path path) { return FSPXI_DeleteDirectory(pxifsRHandle, archive, path); }
Result PXIFSB_DeleteDirectory(FSPXI_Archive archive, FS_Path path) { return FSPXI_DeleteDirectory(pxifsBHandle, archive, path); }

Result PXIFS0_CreateFile(FSPXI_Archive archive, FS_Path path, u32 attributes, u64 fileSize) { return FSPXI_CreateFile(pxifs0Handle, archive, path, attributes, fileSize); }
Result PXIFS1_CreateFile(FSPXI_Archive archive, FS_Path path, u32 attributes, u64 fileSize) { return FSPXI_CreateFile(pxifs1Handle, archive, path, attributes, fileSize); }
Result PXIFSR_CreateFile(FSPXI_Archive archive, FS_Path path, u32 attributes, u64 fileSize) { return FSPXI_CreateFile(pxifsRHandle, archive, path, attributes, fileSize); }
Result PXIFSB_CreateFile(FSPXI_Archive archive, FS_Path path, u32 attributes, u64 fileSize) { return FSPXI_CreateFile(pxifsBHandle, archive, path, attributes, fileSize); }

Result PXIFS0_CreateDirectory(FSPXI_Archive archive, FS_Path path, u32 attributes) { return FSPXI_CreateDirectory(pxifs0Handle, archive, path, attributes); }
Result PXIFS1_CreateDirectory(FSPXI_Archive archive, FS_Path path, u32 attributes) { return FSPXI_CreateDirectory(pxifs1Handle, archive, path, attributes); }
Result PXIFSR_CreateDirectory(FSPXI_Archive archive, FS_Path path, u32 attributes) { return FSPXI_CreateDirectory(pxifsRHandle, archive, path, attributes); }
Result PXIFSB_CreateDirectory(FSPXI_Archive archive, FS_Path path, u32 attributes) { return FSPXI_CreateDirectory(pxifsBHandle, archive, path, attributes); }

Result PXIFS0_RenameDirectory(FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath) { return FSPXI_RenameDirectory(pxifs0Handle, srcArchive, srcPath, dstArchive, dstPath); }
Result PXIFS1_RenameDirectory(FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath) { return FSPXI_RenameDirectory(pxifs1Handle, srcArchive, srcPath, dstArchive, dstPath); }
Result PXIFSR_RenameDirectory(FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath) { return FSPXI_RenameDirectory(pxifsRHandle, srcArchive, srcPath, dstArchive, dstPath); }
Result PXIFSB_RenameDirectory(FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath) { return FSPXI_RenameDirectory(pxifsBHandle, srcArchive, srcPath, dstArchive, dstPath); }

Result PXIFS0_OpenDirectory(FSPXI_Directory* out, FSPXI_Archive archive, FS_Path path) { return FSPXI_OpenDirectory(pxifs0Handle, out, archive, path); }
Result PXIFS1_OpenDirectory(FSPXI_Directory* out, FSPXI_Archive archive, FS_Path path) { return FSPXI_OpenDirectory(pxifs1Handle, out, archive, path); }
Result PXIFSR_OpenDirectory(FSPXI_Directory* out, FSPXI_Archive archive, FS_Path path) { return FSPXI_OpenDirectory(pxifsRHandle, out, archive, path); }
Result PXIFSB_OpenDirectory(FSPXI_Directory* out, FSPXI_Archive archive, FS_Path path) { return FSPXI_OpenDirectory(pxifsBHandle, out, archive, path); }

Result PXIFS0_ReadFile(FSPXI_File file, u32* bytesRead, u64 offset, void* buffer, u32 size) { return FSPXI_ReadFile(pxifs0Handle, file, bytesRead, offset, buffer, size); }
Result PXIFS1_ReadFile(FSPXI_File file, u32* bytesRead, u64 offset, void* buffer, u32 size) { return FSPXI_ReadFile(pxifs1Handle, file, bytesRead, offset, buffer, size); }
Result PXIFSR_ReadFile(FSPXI_File file, u32* bytesRead, u64 offset, void* buffer, u32 size) { return FSPXI_ReadFile(pxifsRHandle, file, bytesRead, offset, buffer, size); }
Result PXIFSB_ReadFile(FSPXI_File file, u32* bytesRead, u64 offset, void* buffer, u32 size) { return FSPXI_ReadFile(pxifsBHandle, file, bytesRead, offset, buffer, size); }

Result PXIFS0_CalculateFileHashSHA256(FSPXI_File file, u32* buffer, u32 size) { return FSPXI_CalculateFileHashSHA256(pxifs0Handle, file, buffer, size); }
Result PXIFS1_CalculateFileHashSHA256(FSPXI_File file, u32* buffer, u32 size) { return FSPXI_CalculateFileHashSHA256(pxifs1Handle, file, buffer, size); }
Result PXIFSR_CalculateFileHashSHA256(FSPXI_File file, u32* buffer, u32 size) { return FSPXI_CalculateFileHashSHA256(pxifsRHandle, file, buffer, size); }
Result PXIFSB_CalculateFileHashSHA256(FSPXI_File file, u32* buffer, u32 size) { return FSPXI_CalculateFileHashSHA256(pxifsBHandle, file, buffer, size); }

Result PXIFS0_WriteFile(FSPXI_File file, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags) { return FSPXI_WriteFile(pxifs0Handle, file, bytesWritten, offset, buffer, size, flags); }
Result PXIFS1_WriteFile(FSPXI_File file, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags) { return FSPXI_WriteFile(pxifs1Handle, file, bytesWritten, offset, buffer, size, flags); }
Result PXIFSR_WriteFile(FSPXI_File file, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags) { return FSPXI_WriteFile(pxifsRHandle, file, bytesWritten, offset, buffer, size, flags); }
Result PXIFSB_WriteFile(FSPXI_File file, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags) { return FSPXI_WriteFile(pxifsBHandle, file, bytesWritten, offset, buffer, size, flags); }

Result PXIFS0_CalcSavegameMAC(FSPXI_File file, void* inBuffer, u32 inSize, void* outBuffer, u32 outSize) { return FSPXI_CalcSavegameMAC(pxifs0Handle, file, inBuffer, inSize, outBuffer, outSize); }
Result PXIFS1_CalcSavegameMAC(FSPXI_File file, void* inBuffer, u32 inSize, void* outBuffer, u32 outSize) { return FSPXI_CalcSavegameMAC(pxifs1Handle, file, inBuffer, inSize, outBuffer, outSize); }
Result PXIFSR_CalcSavegameMAC(FSPXI_File file, void* inBuffer, u32 inSize, void* outBuffer, u32 outSize) { return FSPXI_CalcSavegameMAC(pxifsRHandle, file, inBuffer, inSize, outBuffer, outSize); }
Result PXIFSB_CalcSavegameMAC(FSPXI_File file, void* inBuffer, u32 inSize, void* outBuffer, u32 outSize) { return FSPXI_CalcSavegameMAC(pxifsBHandle, file, inBuffer, inSize, outBuffer, outSize); }

Result PXIFS0_GetFileSize(FSPXI_File file, u64* size) { return FSPXI_GetFileSize(pxifs0Handle, file, size); }
Result PXIFS1_GetFileSize(FSPXI_File file, u64* size) { return FSPXI_GetFileSize(pxifs1Handle, file, size); }
Result PXIFSR_GetFileSize(FSPXI_File file, u64* size) { return FSPXI_GetFileSize(pxifsRHandle, file, size); }
Result PXIFSB_GetFileSize(FSPXI_File file, u64* size) { return FSPXI_GetFileSize(pxifsBHandle, file, size); }

Result PXIFS0_SetFileSize(FSPXI_File file, u64 size) { return FSPXI_SetFileSize(pxifs0Handle, file, size); }
Result PXIFS1_SetFileSize(FSPXI_File file, u64 size) { return FSPXI_SetFileSize(pxifs1Handle, file, size); }
Result PXIFSR_SetFileSize(FSPXI_File file, u64 size) { return FSPXI_SetFileSize(pxifsRHandle, file, size); }
Result PXIFSB_SetFileSize(FSPXI_File file, u64 size) { return FSPXI_SetFileSize(pxifsBHandle, file, size); }

Result PXIFS0_CloseFile(FSPXI_File file) { return FSPXI_CloseFile(pxifs0Handle, file); }
Result PXIFS1_CloseFile(FSPXI_File file) { return FSPXI_CloseFile(pxifs1Handle, file); }
Result PXIFSR_CloseFile(FSPXI_File file) { return FSPXI_CloseFile(pxifsRHandle, file); }
Result PXIFSB_CloseFile(FSPXI_File file) { return FSPXI_CloseFile(pxifsBHandle, file); }

Result PXIFS0_ReadDirectory(FSPXI_Directory directory, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries) { return FSPXI_ReadDirectory(pxifs0Handle, directory, entriesRead, entryCount, entries); }
Result PXIFS1_ReadDirectory(FSPXI_Directory directory, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries) { return FSPXI_ReadDirectory(pxifs1Handle, directory, entriesRead, entryCount, entries); }
Result PXIFSR_ReadDirectory(FSPXI_Directory directory, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries) { return FSPXI_ReadDirectory(pxifsRHandle, directory, entriesRead, entryCount, entries); }
Result PXIFSB_ReadDirectory(FSPXI_Directory directory, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries) { return FSPXI_ReadDirectory(pxifsBHandle, directory, entriesRead, entryCount, entries); }

Result PXIFS0_CloseDirectory(FSPXI_Directory directory) { return FSPXI_CloseDirectory(pxifs0Handle, directory); }
Result PXIFS1_CloseDirectory(FSPXI_Directory directory) { return FSPXI_CloseDirectory(pxifs1Handle, directory); }
Result PXIFSR_CloseDirectory(FSPXI_Directory directory) { return FSPXI_CloseDirectory(pxifsRHandle, directory); }
Result PXIFSB_CloseDirectory(FSPXI_Directory directory) { return FSPXI_CloseDirectory(pxifsBHandle, directory); }

Result PXIFS0_OpenArchive(FSPXI_Archive* archive, FS_ArchiveID archiveID, FS_Path path) { return FSPXI_OpenArchive(pxifs0Handle, archive, archiveID, path); }
Result PXIFS1_OpenArchive(FSPXI_Archive* archive, FS_ArchiveID archiveID, FS_Path path) { return FSPXI_OpenArchive(pxifs1Handle, archive, archiveID, path); }
Result PXIFSR_OpenArchive(FSPXI_Archive* archive, FS_ArchiveID archiveID, FS_Path path) { return FSPXI_OpenArchive(pxifsRHandle, archive, archiveID, path); }
Result PXIFSB_OpenArchive(FSPXI_Archive* archive, FS_ArchiveID archiveID, FS_Path path) { return FSPXI_OpenArchive(pxifsBHandle, archive, archiveID, path); }

Result PXIFS0_0x13(FSPXI_Archive archive, u8* out, FS_Path path) { return FSPXI_0x13(pxifs0Handle, archive, out, path); }
Result PXIFS1_0x13(FSPXI_Archive archive, u8* out, FS_Path path) { return FSPXI_0x13(pxifs1Handle, archive, out, path); }
Result PXIFSR_0x13(FSPXI_Archive archive, u8* out, FS_Path path) { return FSPXI_0x13(pxifsRHandle, archive, out, path); }
Result PXIFSB_0x13(FSPXI_Archive archive, u8* out, FS_Path path) { return FSPXI_0x13(pxifsBHandle, archive, out, path); }

Result PXIFS0_0x14(FSPXI_Archive archive, u32* out, FS_Path path) { return FSPXI_0x14(pxifs0Handle, archive, out, path); }
Result PXIFS1_0x14(FSPXI_Archive archive, u32* out, FS_Path path) { return FSPXI_0x14(pxifs1Handle, archive, out, path); }
Result PXIFSR_0x14(FSPXI_Archive archive, u32* out, FS_Path path) { return FSPXI_0x14(pxifsRHandle, archive, out, path); }
Result PXIFSB_0x14(FSPXI_Archive archive, u32* out, FS_Path path) { return FSPXI_0x14(pxifsBHandle, archive, out, path); }

Result PXIFS0_CommitSaveData(FSPXI_Archive archive, u32 unknown) { return FSPXI_CommitSaveData(pxifs0Handle, archive, unknown); }
Result PXIFS1_CommitSaveData(FSPXI_Archive archive, u32 unknown) { return FSPXI_CommitSaveData(pxifs1Handle, archive, unknown); }
Result PXIFSR_CommitSaveData(FSPXI_Archive archive, u32 unknown) { return FSPXI_CommitSaveData(pxifsRHandle, archive, unknown); }
Result PXIFSB_CommitSaveData(FSPXI_Archive archive, u32 unknown) { return FSPXI_CommitSaveData(pxifsBHandle, archive, unknown); }

Result PXIFS0_CloseArchive(FSPXI_Archive archive) { return FSPXI_CloseArchive(pxifs0Handle, archive); }
Result PXIFS1_CloseArchive(FSPXI_Archive archive) { return FSPXI_CloseArchive(pxifs1Handle, archive); }
Result PXIFSR_CloseArchive(FSPXI_Archive archive) { return FSPXI_CloseArchive(pxifsRHandle, archive); }
Result PXIFSB_CloseArchive(FSPXI_Archive archive) { return FSPXI_CloseArchive(pxifsBHandle, archive); }

Result PXIFS0_0x17(FSPXI_Archive archive, bool* out) { return FSPXI_0x17(pxifs0Handle, archive, out); }
Result PXIFS1_0x17(FSPXI_Archive archive, bool* out) { return FSPXI_0x17(pxifs1Handle, archive, out); }
Result PXIFSR_0x17(FSPXI_Archive archive, bool* out) { return FSPXI_0x17(pxifsRHandle, archive, out); }
Result PXIFSB_0x17(FSPXI_Archive archive, bool* out) { return FSPXI_0x17(pxifsBHandle, archive, out); }

Result PXIFS0_GetCardType(FS_CardType* out) { return FSPXI_GetCardType(pxifs0Handle, out); }
Result PXIFS1_GetCardType(FS_CardType* out) { return FSPXI_GetCardType(pxifs1Handle, out); }
Result PXIFSR_GetCardType(FS_CardType* out) { return FSPXI_GetCardType(pxifsRHandle, out); }
Result PXIFSB_GetCardType(FS_CardType* out) { return FSPXI_GetCardType(pxifsBHandle, out); }

Result PXIFS0_GetSdmcArchiveResource(FS_ArchiveResource* out) { return FSPXI_GetSdmcArchiveResource(pxifs0Handle, out); }
Result PXIFS1_GetSdmcArchiveResource(FS_ArchiveResource* out) { return FSPXI_GetSdmcArchiveResource(pxifs1Handle, out); }
Result PXIFSR_GetSdmcArchiveResource(FS_ArchiveResource* out) { return FSPXI_GetSdmcArchiveResource(pxifsRHandle, out); }
Result PXIFSB_GetSdmcArchiveResource(FS_ArchiveResource* out) { return FSPXI_GetSdmcArchiveResource(pxifsBHandle, out); }

Result PXIFS0_GetNandArchiveResource(FS_ArchiveResource* out) { return FSPXI_GetNandArchiveResource(pxifs0Handle, out); }
Result PXIFS1_GetNandArchiveResource(FS_ArchiveResource* out) { return FSPXI_GetNandArchiveResource(pxifs1Handle, out); }
Result PXIFSR_GetNandArchiveResource(FS_ArchiveResource* out) { return FSPXI_GetNandArchiveResource(pxifsRHandle, out); }
Result PXIFSB_GetNandArchiveResource(FS_ArchiveResource* out) { return FSPXI_GetNandArchiveResource(pxifsBHandle, out); }

Result PXIFS0_GetSdmcFatFsError(u32* out) { return FSPXI_GetSdmcFatFsError(pxifs0Handle, out); }
Result PXIFS1_GetSdmcFatFsError(u32* out) { return FSPXI_GetSdmcFatFsError(pxifs1Handle, out); }
Result PXIFSR_GetSdmcFatFsError(u32* out) { return FSPXI_GetSdmcFatFsError(pxifsRHandle, out); }
Result PXIFSB_GetSdmcFatFsError(u32* out) { return FSPXI_GetSdmcFatFsError(pxifsBHandle, out); }

Result PXIFS0_IsSdmcDetected(bool* out) { return FSPXI_IsSdmcDetected(pxifs0Handle, out); }
Result PXIFS1_IsSdmcDetected(bool* out) { return FSPXI_IsSdmcDetected(pxifs1Handle, out); }
Result PXIFSR_IsSdmcDetected(bool* out) { return FSPXI_IsSdmcDetected(pxifsRHandle, out); }
Result PXIFSB_IsSdmcDetected(bool* out) { return FSPXI_IsSdmcDetected(pxifsBHandle, out); }

Result PXIFS0_IsSdmcWritable(bool* out) { return FSPXI_IsSdmcWritable(pxifs0Handle, out); }
Result PXIFS1_IsSdmcWritable(bool* out) { return FSPXI_IsSdmcWritable(pxifs1Handle, out); }
Result PXIFSR_IsSdmcWritable(bool* out) { return FSPXI_IsSdmcWritable(pxifsRHandle, out); }
Result PXIFSB_IsSdmcWritable(bool* out) { return FSPXI_IsSdmcWritable(pxifsBHandle, out); }

Result PXIFS0_GetSdmcCid(void* out, u32 size) { return FSPXI_GetSdmcCid(pxifs0Handle, out, size); }
Result PXIFS1_GetSdmcCid(void* out, u32 size) { return FSPXI_GetSdmcCid(pxifs1Handle, out, size); }
Result PXIFSR_GetSdmcCid(void* out, u32 size) { return FSPXI_GetSdmcCid(pxifsRHandle, out, size); }
Result PXIFSB_GetSdmcCid(void* out, u32 size) { return FSPXI_GetSdmcCid(pxifsBHandle, out, size); }

Result PXIFS0_GetNandCid(void* out, u32 size) { return FSPXI_GetNandCid(pxifs0Handle, out, size); }
Result PXIFS1_GetNandCid(void* out, u32 size) { return FSPXI_GetNandCid(pxifs1Handle, out, size); }
Result PXIFSR_GetNandCid(void* out, u32 size) { return FSPXI_GetNandCid(pxifsRHandle, out, size); }
Result PXIFSB_GetNandCid(void* out, u32 size) { return FSPXI_GetNandCid(pxifsBHandle, out, size); }

Result PXIFS0_GetSdmcSpeedInfo(u32* out) { return FSPXI_GetSdmcSpeedInfo(pxifs0Handle, out); }
Result PXIFS1_GetSdmcSpeedInfo(u32* out) { return FSPXI_GetSdmcSpeedInfo(pxifs1Handle, out); }
Result PXIFSR_GetSdmcSpeedInfo(u32* out) { return FSPXI_GetSdmcSpeedInfo(pxifsRHandle, out); }
Result PXIFSB_GetSdmcSpeedInfo(u32* out) { return FSPXI_GetSdmcSpeedInfo(pxifsBHandle, out); }

Result PXIFS0_GetNandSpeedInfo(u32* out) { return FSPXI_GetNandSpeedInfo(pxifs0Handle, out); }
Result PXIFS1_GetNandSpeedInfo(u32* out) { return FSPXI_GetNandSpeedInfo(pxifs1Handle, out); }
Result PXIFSR_GetNandSpeedInfo(u32* out) { return FSPXI_GetNandSpeedInfo(pxifsRHandle, out); }
Result PXIFSB_GetNandSpeedInfo(u32* out) { return FSPXI_GetNandSpeedInfo(pxifsBHandle, out); }

Result PXIFS0_GetSdmcLog(void* out, u32 size) { return FSPXI_GetSdmcLog(pxifs0Handle, out, size); }
Result PXIFS1_GetSdmcLog(void* out, u32 size) { return FSPXI_GetSdmcLog(pxifs1Handle, out, size); }
Result PXIFSR_GetSdmcLog(void* out, u32 size) { return FSPXI_GetSdmcLog(pxifsRHandle, out, size); }
Result PXIFSB_GetSdmcLog(void* out, u32 size) { return FSPXI_GetSdmcLog(pxifsBHandle, out, size); }

Result PXIFS0_GetNandLog(void* out, u32 size) { return FSPXI_GetNandLog(pxifs0Handle, out, size); }
Result PXIFS1_GetNandLog(void* out, u32 size) { return FSPXI_GetNandLog(pxifs1Handle, out, size); }
Result PXIFSR_GetNandLog(void* out, u32 size) { return FSPXI_GetNandLog(pxifsRHandle, out, size); }
Result PXIFSB_GetNandLog(void* out, u32 size) { return FSPXI_GetNandLog(pxifsBHandle, out, size); }

Result PXIFS0_ClearSdmcLog(void) { return FSPXI_ClearSdmcLog(pxifs0Handle); }
Result PXIFS1_ClearSdmcLog(void) { return FSPXI_ClearSdmcLog(pxifs1Handle); }
Result PXIFSR_ClearSdmcLog(void) { return FSPXI_ClearSdmcLog(pxifsRHandle); }
Result PXIFSB_ClearSdmcLog(void) { return FSPXI_ClearSdmcLog(pxifsBHandle); }

Result PXIFS0_ClearNandLog(void) { return FSPXI_ClearNandLog(pxifs0Handle); }
Result PXIFS1_ClearNandLog(void) { return FSPXI_ClearNandLog(pxifs1Handle); }
Result PXIFSR_ClearNandLog(void) { return FSPXI_ClearNandLog(pxifsRHandle); }
Result PXIFSB_ClearNandLog(void) { return FSPXI_ClearNandLog(pxifsBHandle); }

Result PXIFS0_CardSlotIsInserted(bool* inserted) { return FSPXI_CardSlotIsInserted(pxifs0Handle, inserted); }
Result PXIFS1_CardSlotIsInserted(bool* inserted) { return FSPXI_CardSlotIsInserted(pxifs1Handle, inserted); }
Result PXIFSR_CardSlotIsInserted(bool* inserted) { return FSPXI_CardSlotIsInserted(pxifsRHandle, inserted); }
Result PXIFSB_CardSlotIsInserted(bool* inserted) { return FSPXI_CardSlotIsInserted(pxifsBHandle, inserted); }

Result PXIFS0_CardSlotPowerOn(bool* status) { return FSPXI_CardSlotPowerOn(pxifs0Handle, status); }
Result PXIFS1_CardSlotPowerOn(bool* status) { return FSPXI_CardSlotPowerOn(pxifs1Handle, status); }
Result PXIFSR_CardSlotPowerOn(bool* status) { return FSPXI_CardSlotPowerOn(pxifsRHandle, status); }
Result PXIFSB_CardSlotPowerOn(bool* status) { return FSPXI_CardSlotPowerOn(pxifsBHandle, status); }

Result PXIFS0_CardSlotPowerOff(bool* status) { return FSPXI_CardSlotPowerOff(pxifs0Handle, status); }
Result PXIFS1_CardSlotPowerOff(bool* status) { return FSPXI_CardSlotPowerOff(pxifs1Handle, status); }
Result PXIFSR_CardSlotPowerOff(bool* status) { return FSPXI_CardSlotPowerOff(pxifsRHandle, status); }
Result PXIFSB_CardSlotPowerOff(bool* status) { return FSPXI_CardSlotPowerOff(pxifsBHandle, status); }

Result PXIFS0_CardSlotGetCardIFPowerStatus(bool* status) { return FSPXI_CardSlotGetCardIFPowerStatus(pxifs0Handle, status); }
Result PXIFS1_CardSlotGetCardIFPowerStatus(bool* status) { return FSPXI_CardSlotGetCardIFPowerStatus(pxifs1Handle, status); }
Result PXIFSR_CardSlotGetCardIFPowerStatus(bool* status) { return FSPXI_CardSlotGetCardIFPowerStatus(pxifsRHandle, status); }
Result PXIFSB_CardSlotGetCardIFPowerStatus(bool* status) { return FSPXI_CardSlotGetCardIFPowerStatus(pxifsBHandle, status); }

Result PXIFS0_CardNorDirectCommand(u8 commandId) { return FSPXI_CardNorDirectCommand(pxifs0Handle, commandId); }
Result PXIFS1_CardNorDirectCommand(u8 commandId) { return FSPXI_CardNorDirectCommand(pxifs1Handle, commandId); }
Result PXIFSR_CardNorDirectCommand(u8 commandId) { return FSPXI_CardNorDirectCommand(pxifsRHandle, commandId); }
Result PXIFSB_CardNorDirectCommand(u8 commandId) { return FSPXI_CardNorDirectCommand(pxifsBHandle, commandId); }

Result PXIFS0_CardNorDirectCommandWithAddress(u8 commandId, u32 address) { return FSPXI_CardNorDirectCommandWithAddress(pxifs0Handle, commandId, address); }
Result PXIFS1_CardNorDirectCommandWithAddress(u8 commandId, u32 address) { return FSPXI_CardNorDirectCommandWithAddress(pxifs1Handle, commandId, address); }
Result PXIFSR_CardNorDirectCommandWithAddress(u8 commandId, u32 address) { return FSPXI_CardNorDirectCommandWithAddress(pxifsRHandle, commandId, address); }
Result PXIFSB_CardNorDirectCommandWithAddress(u8 commandId, u32 address) { return FSPXI_CardNorDirectCommandWithAddress(pxifsBHandle, commandId, address); }

Result PXIFS0_CardNorDirectRead(u8 commandId, u32 size, u8* output) { return FSPXI_CardNorDirectRead(pxifs0Handle, commandId, size, output); }
Result PXIFS1_CardNorDirectRead(u8 commandId, u32 size, u8* output) { return FSPXI_CardNorDirectRead(pxifs1Handle, commandId, size, output); }
Result PXIFSR_CardNorDirectRead(u8 commandId, u32 size, u8* output) { return FSPXI_CardNorDirectRead(pxifsRHandle, commandId, size, output); }
Result PXIFSB_CardNorDirectRead(u8 commandId, u32 size, u8* output) { return FSPXI_CardNorDirectRead(pxifsBHandle, commandId, size, output); }

Result PXIFS0_CardNorDirectReadWithAddress(u8 commandId, u32 address, u32 size, u8* output) { return FSPXI_CardNorDirectReadWithAddress(pxifs0Handle, commandId, address, size, output); }
Result PXIFS1_CardNorDirectReadWithAddress(u8 commandId, u32 address, u32 size, u8* output) { return FSPXI_CardNorDirectReadWithAddress(pxifs1Handle, commandId, address, size, output); }
Result PXIFSR_CardNorDirectReadWithAddress(u8 commandId, u32 address, u32 size, u8* output) { return FSPXI_CardNorDirectReadWithAddress(pxifsRHandle, commandId, address, size, output); }
Result PXIFSB_CardNorDirectReadWithAddress(u8 commandId, u32 address, u32 size, u8* output) { return FSPXI_CardNorDirectReadWithAddress(pxifsBHandle, commandId, address, size, output); }

Result PXIFS0_CardNorDirectWrite(u8 commandId, u32 size, u8* input) { return FSPXI_CardNorDirectWrite(pxifs0Handle, commandId, size, input); }
Result PXIFS1_CardNorDirectWrite(u8 commandId, u32 size, u8* input) { return FSPXI_CardNorDirectWrite(pxifs1Handle, commandId, size, input); }
Result PXIFSR_CardNorDirectWrite(u8 commandId, u32 size, u8* input) { return FSPXI_CardNorDirectWrite(pxifsRHandle, commandId, size, input); }
Result PXIFSB_CardNorDirectWrite(u8 commandId, u32 size, u8* input) { return FSPXI_CardNorDirectWrite(pxifsBHandle, commandId, size, input); }

Result PXIFS0_CardNorDirectWriteWithAddress(u8 commandId, u32 address, u32 size, u8* input) { return FSPXI_CardNorDirectWriteWithAddress(pxifs0Handle, commandId, address, size, input); }
Result PXIFS1_CardNorDirectWriteWithAddress(u8 commandId, u32 address, u32 size, u8* input) { return FSPXI_CardNorDirectWriteWithAddress(pxifs1Handle, commandId, address, size, input); }
Result PXIFSR_CardNorDirectWriteWithAddress(u8 commandId, u32 address, u32 size, u8* input) { return FSPXI_CardNorDirectWriteWithAddress(pxifsRHandle, commandId, address, size, input); }
Result PXIFSB_CardNorDirectWriteWithAddress(u8 commandId, u32 address, u32 size, u8* input) { return FSPXI_CardNorDirectWriteWithAddress(pxifsBHandle, commandId, address, size, input); }

Result PXIFS0_CardNorDirectRead_4xIO(u8 commandId, u32 address, u32 size, u8* output) { return FSPXI_CardNorDirectRead_4xIO(pxifs0Handle, commandId, address, size, output); }
Result PXIFS1_CardNorDirectRead_4xIO(u8 commandId, u32 address, u32 size, u8* output) { return FSPXI_CardNorDirectRead_4xIO(pxifs1Handle, commandId, address, size, output); }
Result PXIFSR_CardNorDirectRead_4xIO(u8 commandId, u32 address, u32 size, u8* output) { return FSPXI_CardNorDirectRead_4xIO(pxifsRHandle, commandId, address, size, output); }
Result PXIFSB_CardNorDirectRead_4xIO(u8 commandId, u32 address, u32 size, u8* output) { return FSPXI_CardNorDirectRead_4xIO(pxifsBHandle, commandId, address, size, output); }

Result PXIFS0_CardNorDirectCpuWriteWithoutVerify(u32 address, u32 size, u8* input) { return FSPXI_CardNorDirectCpuWriteWithoutVerify(pxifs0Handle, address, size, input); }
Result PXIFS1_CardNorDirectCpuWriteWithoutVerify(u32 address, u32 size, u8* input) { return FSPXI_CardNorDirectCpuWriteWithoutVerify(pxifs1Handle, address, size, input); }
Result PXIFSR_CardNorDirectCpuWriteWithoutVerify(u32 address, u32 size, u8* input) { return FSPXI_CardNorDirectCpuWriteWithoutVerify(pxifsRHandle, address, size, input); }
Result PXIFSB_CardNorDirectCpuWriteWithoutVerify(u32 address, u32 size, u8* input) { return FSPXI_CardNorDirectCpuWriteWithoutVerify(pxifsBHandle, address, size, input); }

Result PXIFS0_CardNorDirectSectorEraseWithoutVerify(u32 address) { return FSPXI_CardNorDirectSectorEraseWithoutVerify(pxifs0Handle, address); }
Result PXIFS1_CardNorDirectSectorEraseWithoutVerify(u32 address) { return FSPXI_CardNorDirectSectorEraseWithoutVerify(pxifs1Handle, address); }
Result PXIFSR_CardNorDirectSectorEraseWithoutVerify(u32 address) { return FSPXI_CardNorDirectSectorEraseWithoutVerify(pxifsRHandle, address); }
Result PXIFSB_CardNorDirectSectorEraseWithoutVerify(u32 address) { return FSPXI_CardNorDirectSectorEraseWithoutVerify(pxifsBHandle, address); }

Result PXIFS0_GetProductInfo(FS_ProductInfo* info, FSPXI_Archive archive) { return FSPXI_GetProductInfo(pxifs0Handle, info, archive); }
Result PXIFS1_GetProductInfo(FS_ProductInfo* info, FSPXI_Archive archive) { return FSPXI_GetProductInfo(pxifs1Handle, info, archive); }
Result PXIFSR_GetProductInfo(FS_ProductInfo* info, FSPXI_Archive archive) { return FSPXI_GetProductInfo(pxifsRHandle, info, archive); }
Result PXIFSB_GetProductInfo(FS_ProductInfo* info, FSPXI_Archive archive) { return FSPXI_GetProductInfo(pxifsBHandle, info, archive); }

Result PXIFS0_SetCardSpiBaudrate(FS_CardSpiBaudRate baudRate) { return FSPXI_SetCardSpiBaudrate(pxifs0Handle, baudRate); }
Result PXIFS1_SetCardSpiBaudrate(FS_CardSpiBaudRate baudRate) { return FSPXI_SetCardSpiBaudrate(pxifs1Handle, baudRate); }
Result PXIFSR_SetCardSpiBaudrate(FS_CardSpiBaudRate baudRate) { return FSPXI_SetCardSpiBaudrate(pxifsRHandle, baudRate); }
Result PXIFSB_SetCardSpiBaudrate(FS_CardSpiBaudRate baudRate) { return FSPXI_SetCardSpiBaudrate(pxifsBHandle, baudRate); }

Result PXIFS0_SetCardSpiBusMode(FS_CardSpiBusMode busMode) { return FSPXI_SetCardSpiBusMode(pxifs0Handle, busMode); }
Result PXIFS1_SetCardSpiBusMode(FS_CardSpiBusMode busMode) { return FSPXI_SetCardSpiBusMode(pxifs1Handle, busMode); }
Result PXIFSR_SetCardSpiBusMode(FS_CardSpiBusMode busMode) { return FSPXI_SetCardSpiBusMode(pxifsRHandle, busMode); }
Result PXIFSB_SetCardSpiBusMode(FS_CardSpiBusMode busMode) { return FSPXI_SetCardSpiBusMode(pxifsBHandle, busMode); }

Result PXIFS0_SendInitializeInfoTo9(u8 unk) { return FSPXI_SendInitializeInfoTo9(pxifs0Handle, unk); }
Result PXIFS1_SendInitializeInfoTo9(u8 unk) { return FSPXI_SendInitializeInfoTo9(pxifs1Handle, unk); }
Result PXIFSR_SendInitializeInfoTo9(u8 unk) { return FSPXI_SendInitializeInfoTo9(pxifsRHandle, unk); }
Result PXIFSB_SendInitializeInfoTo9(u8 unk) { return FSPXI_SendInitializeInfoTo9(pxifsBHandle, unk); }

Result PXIFS0_CreateExtSaveData(FS_ExtSaveDataInfo info) { return FSPXI_CreateExtSaveData(pxifs0Handle, info); }
Result PXIFS1_CreateExtSaveData(FS_ExtSaveDataInfo info) { return FSPXI_CreateExtSaveData(pxifs1Handle, info); }
Result PXIFSR_CreateExtSaveData(FS_ExtSaveDataInfo info) { return FSPXI_CreateExtSaveData(pxifsRHandle, info); }
Result PXIFSB_CreateExtSaveData(FS_ExtSaveDataInfo info) { return FSPXI_CreateExtSaveData(pxifsBHandle, info); }

Result PXIFS0_DeleteExtSaveData(FS_ExtSaveDataInfo info) { return FSPXI_DeleteExtSaveData(pxifs0Handle, info); }
Result PXIFS1_DeleteExtSaveData(FS_ExtSaveDataInfo info) { return FSPXI_DeleteExtSaveData(pxifs1Handle, info); }
Result PXIFSR_DeleteExtSaveData(FS_ExtSaveDataInfo info) { return FSPXI_DeleteExtSaveData(pxifsRHandle, info); }
Result PXIFSB_DeleteExtSaveData(FS_ExtSaveDataInfo info) { return FSPXI_DeleteExtSaveData(pxifsBHandle, info); }

Result PXIFS0_EnumerateExtSaveData(u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids) { return FSPXI_EnumerateExtSaveData(pxifs0Handle, idsWritten, idsSize, mediaType, idSize, shared, ids); }
Result PXIFS1_EnumerateExtSaveData(u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids) { return FSPXI_EnumerateExtSaveData(pxifs1Handle, idsWritten, idsSize, mediaType, idSize, shared, ids); }
Result PXIFSR_EnumerateExtSaveData(u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids) { return FSPXI_EnumerateExtSaveData(pxifsRHandle, idsWritten, idsSize, mediaType, idSize, shared, ids); }
Result PXIFSB_EnumerateExtSaveData(u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids) { return FSPXI_EnumerateExtSaveData(pxifsBHandle, idsWritten, idsSize, mediaType, idSize, shared, ids); }

Result PXIFS0_GetSpecialContentIndex(u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type) { return FSPXI_GetSpecialContentIndex(pxifs0Handle, index, mediaType, programId, type); }
Result PXIFS1_GetSpecialContentIndex(u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type) { return FSPXI_GetSpecialContentIndex(pxifs1Handle, index, mediaType, programId, type); }
Result PXIFSR_GetSpecialContentIndex(u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type) { return FSPXI_GetSpecialContentIndex(pxifsRHandle, index, mediaType, programId, type); }
Result PXIFSB_GetSpecialContentIndex(u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type) { return FSPXI_GetSpecialContentIndex(pxifsBHandle, index, mediaType, programId, type); }

Result PXIFS0_GetLegacyRomHeader(FS_MediaType mediaType, u64 programId, u8* header) { return FSPXI_GetLegacyRomHeader(pxifs0Handle, mediaType, programId, header); }
Result PXIFS1_GetLegacyRomHeader(FS_MediaType mediaType, u64 programId, u8* header) { return FSPXI_GetLegacyRomHeader(pxifs1Handle, mediaType, programId, header); }
Result PXIFSR_GetLegacyRomHeader(FS_MediaType mediaType, u64 programId, u8* header) { return FSPXI_GetLegacyRomHeader(pxifsRHandle, mediaType, programId, header); }
Result PXIFSB_GetLegacyRomHeader(FS_MediaType mediaType, u64 programId, u8* header) { return FSPXI_GetLegacyRomHeader(pxifsBHandle, mediaType, programId, header); }

Result PXIFS0_GetLegacyBannerData(FS_MediaType mediaType, u64 programId, u8* banner, u32 unk) { return FSPXI_GetLegacyBannerData(pxifs0Handle, mediaType, programId, banner, unk); }
Result PXIFS1_GetLegacyBannerData(FS_MediaType mediaType, u64 programId, u8* banner, u32 unk) { return FSPXI_GetLegacyBannerData(pxifs1Handle, mediaType, programId, banner, unk); }
Result PXIFSR_GetLegacyBannerData(FS_MediaType mediaType, u64 programId, u8* banner, u32 unk) { return FSPXI_GetLegacyBannerData(pxifsRHandle, mediaType, programId, banner, unk); }
Result PXIFSB_GetLegacyBannerData(FS_MediaType mediaType, u64 programId, u8* banner, u32 unk) { return FSPXI_GetLegacyBannerData(pxifsBHandle, mediaType, programId, banner, unk); }

Result PXIFS0_0x3D(u32 unk) { return FSPXI_0x3D(pxifs0Handle, unk); }
Result PXIFS1_0x3D(u32 unk) { return FSPXI_0x3D(pxifs1Handle, unk); }
Result PXIFSR_0x3D(u32 unk) { return FSPXI_0x3D(pxifsRHandle, unk); }
Result PXIFSB_0x3D(u32 unk) { return FSPXI_0x3D(pxifsBHandle, unk); }

Result PXIFS0_DeleteSdmcRoot(void) { return FSPXI_DeleteSdmcRoot(pxifs0Handle); }
Result PXIFS1_DeleteSdmcRoot(void) { return FSPXI_DeleteSdmcRoot(pxifs1Handle); }
Result PXIFSR_DeleteSdmcRoot(void) { return FSPXI_DeleteSdmcRoot(pxifsRHandle); }
Result PXIFSB_DeleteSdmcRoot(void) { return FSPXI_DeleteSdmcRoot(pxifsBHandle); }

Result PXIFS0_DeleteAllExtSaveDataOnNand(void) { return FSPXI_DeleteAllExtSaveDataOnNand(pxifs0Handle); }
Result PXIFS1_DeleteAllExtSaveDataOnNand(void) { return FSPXI_DeleteAllExtSaveDataOnNand(pxifs1Handle); }
Result PXIFSR_DeleteAllExtSaveDataOnNand(void) { return FSPXI_DeleteAllExtSaveDataOnNand(pxifsRHandle); }
Result PXIFSB_DeleteAllExtSaveDataOnNand(void) { return FSPXI_DeleteAllExtSaveDataOnNand(pxifsBHandle); }

Result PXIFS0_InitializeCtrFilesystem(void) { return FSPXI_InitializeCtrFilesystem(pxifs0Handle); }
Result PXIFS1_InitializeCtrFilesystem(void) { return FSPXI_InitializeCtrFilesystem(pxifs1Handle); }
Result PXIFSR_InitializeCtrFilesystem(void) { return FSPXI_InitializeCtrFilesystem(pxifsRHandle); }
Result PXIFSB_InitializeCtrFilesystem(void) { return FSPXI_InitializeCtrFilesystem(pxifsBHandle); }

Result PXIFS0_CreateSeed(void) { return FSPXI_CreateSeed(pxifs0Handle); }
Result PXIFS1_CreateSeed(void) { return FSPXI_CreateSeed(pxifs1Handle); }
Result PXIFSR_CreateSeed(void) { return FSPXI_CreateSeed(pxifsRHandle); }
Result PXIFSB_CreateSeed(void) { return FSPXI_CreateSeed(pxifsBHandle); }

Result PXIFS0_GetSdmcCtrRootPath(u8* out, u32 length) { return FSPXI_GetSdmcCtrRootPath(pxifs0Handle, out, length); }
Result PXIFS1_GetSdmcCtrRootPath(u8* out, u32 length) { return FSPXI_GetSdmcCtrRootPath(pxifs1Handle, out, length); }
Result PXIFSR_GetSdmcCtrRootPath(u8* out, u32 length) { return FSPXI_GetSdmcCtrRootPath(pxifsRHandle, out, length); }
Result PXIFSB_GetSdmcCtrRootPath(u8* out, u32 length) { return FSPXI_GetSdmcCtrRootPath(pxifsBHandle, out, length); }

Result PXIFS0_GetArchiveResource(FS_ArchiveResource* archiveResource, FS_SystemMediaType mediaType) { return FSPXI_GetArchiveResource(pxifs0Handle, archiveResource, mediaType); }
Result PXIFS1_GetArchiveResource(FS_ArchiveResource* archiveResource, FS_SystemMediaType mediaType) { return FSPXI_GetArchiveResource(pxifs1Handle, archiveResource, mediaType); }
Result PXIFSR_GetArchiveResource(FS_ArchiveResource* archiveResource, FS_SystemMediaType mediaType) { return FSPXI_GetArchiveResource(pxifsRHandle, archiveResource, mediaType); }
Result PXIFSB_GetArchiveResource(FS_ArchiveResource* archiveResource, FS_SystemMediaType mediaType) { return FSPXI_GetArchiveResource(pxifsBHandle, archiveResource, mediaType); }

Result PXIFS0_ExportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed) { return FSPXI_ExportIntegrityVerificationSeed(pxifs0Handle, seed); }
Result PXIFS1_ExportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed) { return FSPXI_ExportIntegrityVerificationSeed(pxifs1Handle, seed); }
Result PXIFSR_ExportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed) { return FSPXI_ExportIntegrityVerificationSeed(pxifsRHandle, seed); }
Result PXIFSB_ExportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed) { return FSPXI_ExportIntegrityVerificationSeed(pxifsBHandle, seed); }

Result PXIFS0_ImportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed) { return FSPXI_ImportIntegrityVerificationSeed(pxifs0Handle, seed); }
Result PXIFS1_ImportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed) { return FSPXI_ImportIntegrityVerificationSeed(pxifs1Handle, seed); }
Result PXIFSR_ImportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed) { return FSPXI_ImportIntegrityVerificationSeed(pxifsRHandle, seed); }
Result PXIFSB_ImportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed) { return FSPXI_ImportIntegrityVerificationSeed(pxifsBHandle, seed); }

Result PXIFS0_GetLegacySubBannerData(u32 bannerSize, FS_MediaType mediaType, u64 programId, u8* banner) { return FSPXI_GetLegacySubBannerData(pxifs0Handle, bannerSize, mediaType, programId, banner); }
Result PXIFS1_GetLegacySubBannerData(u32 bannerSize, FS_MediaType mediaType, u64 programId, u8* banner) { return FSPXI_GetLegacySubBannerData(pxifs1Handle, bannerSize, mediaType, programId, banner); }
Result PXIFSR_GetLegacySubBannerData(u32 bannerSize, FS_MediaType mediaType, u64 programId, u8* banner) { return FSPXI_GetLegacySubBannerData(pxifsRHandle, bannerSize, mediaType, programId, banner); }
Result PXIFSB_GetLegacySubBannerData(u32 bannerSize, FS_MediaType mediaType, u64 programId, u8* banner) { return FSPXI_GetLegacySubBannerData(pxifsBHandle, bannerSize, mediaType, programId, banner); }

Result PXIFS0_0x47(void* buf, u32 size) { return FSPXI_0x47(pxifs0Handle, buf, size); }
Result PXIFS1_0x47(void* buf, u32 size) { return FSPXI_0x47(pxifs1Handle, buf, size); }
Result PXIFSR_0x47(void* buf, u32 size) { return FSPXI_0x47(pxifsRHandle, buf, size); }
Result PXIFSB_0x47(void* buf, u32 size) { return FSPXI_0x47(pxifsBHandle, buf, size); }

Result PXIFS0_GetFileLastModified(FSPXI_Archive archive, u64* out, u16* path, u32 size) { return FSPXI_GetFileLastModified(pxifs0Handle, archive, out, path, size); }
Result PXIFS1_GetFileLastModified(FSPXI_Archive archive, u64* out, u16* path, u32 size) { return FSPXI_GetFileLastModified(pxifs1Handle, archive, out, path, size); }
Result PXIFSR_GetFileLastModified(FSPXI_Archive archive, u64* out, u16* path, u32 size) { return FSPXI_GetFileLastModified(pxifsRHandle, archive, out, path, size); }
Result PXIFSB_GetFileLastModified(FSPXI_Archive archive, u64* out, u16* path, u32 size) { return FSPXI_GetFileLastModified(pxifsBHandle, archive, out, path, size); }

Result PXIFS0_ReadSpecialFile(u32* bytesRead, u64 fileOffset, u32 size, u8* data) { return FSPXI_ReadSpecialFile(pxifs0Handle, bytesRead, fileOffset, size, data); }
Result PXIFS1_ReadSpecialFile(u32* bytesRead, u64 fileOffset, u32 size, u8* data) { return FSPXI_ReadSpecialFile(pxifs1Handle, bytesRead, fileOffset, size, data); }
Result PXIFSR_ReadSpecialFile(u32* bytesRead, u64 fileOffset, u32 size, u8* data) { return FSPXI_ReadSpecialFile(pxifsRHandle, bytesRead, fileOffset, size, data); }
Result PXIFSB_ReadSpecialFile(u32* bytesRead, u64 fileOffset, u32 size, u8* data) { return FSPXI_ReadSpecialFile(pxifsBHandle, bytesRead, fileOffset, size, data); }

Result PXIFS0_GetSpecialFileSize(u64* fileSize) { return FSPXI_GetSpecialFileSize(pxifs0Handle, fileSize); }
Result PXIFS1_GetSpecialFileSize(u64* fileSize) { return FSPXI_GetSpecialFileSize(pxifs1Handle, fileSize); }
Result PXIFSR_GetSpecialFileSize(u64* fileSize) { return FSPXI_GetSpecialFileSize(pxifsRHandle, fileSize); }
Result PXIFSB_GetSpecialFileSize(u64* fileSize) { return FSPXI_GetSpecialFileSize(pxifsBHandle, fileSize); }

Result PXIFS0_StartDeviceMoveAsSource(FS_DeviceMoveContext* context) { return FSPXI_StartDeviceMoveAsSource(pxifs0Handle, context); }
Result PXIFS1_StartDeviceMoveAsSource(FS_DeviceMoveContext* context) { return FSPXI_StartDeviceMoveAsSource(pxifs1Handle, context); }
Result PXIFSR_StartDeviceMoveAsSource(FS_DeviceMoveContext* context) { return FSPXI_StartDeviceMoveAsSource(pxifsRHandle, context); }
Result PXIFSB_StartDeviceMoveAsSource(FS_DeviceMoveContext* context) { return FSPXI_StartDeviceMoveAsSource(pxifsBHandle, context); }

Result PXIFS0_StartDeviceMoveAsDestination(FS_DeviceMoveContext context, bool clear) { return FSPXI_StartDeviceMoveAsDestination(pxifs0Handle, context, clear); }
Result PXIFS1_StartDeviceMoveAsDestination(FS_DeviceMoveContext context, bool clear) { return FSPXI_StartDeviceMoveAsDestination(pxifs1Handle, context, clear); }
Result PXIFSR_StartDeviceMoveAsDestination(FS_DeviceMoveContext context, bool clear) { return FSPXI_StartDeviceMoveAsDestination(pxifsRHandle, context, clear); }
Result PXIFSB_StartDeviceMoveAsDestination(FS_DeviceMoveContext context, bool clear) { return FSPXI_StartDeviceMoveAsDestination(pxifsBHandle, context, clear); }

Result PXIFS0_ReadFileSHA256(FSPXI_File file, u32* bytesRead, u64 offset, void* readBuffer, u32 readBufferSize, void* hashtable, u32 hashtableSize, u32 unk) { return FSPXI_ReadFileSHA256(pxifs0Handle, file, bytesRead, offset, readBuffer, readBufferSize, hashtable, hashtableSize, unk); }
Result PXIFS1_ReadFileSHA256(FSPXI_File file, u32* bytesRead, u64 offset, void* readBuffer, u32 readBufferSize, void* hashtable, u32 hashtableSize, u32 unk) { return FSPXI_ReadFileSHA256(pxifs1Handle, file, bytesRead, offset, readBuffer, readBufferSize, hashtable, hashtableSize, unk); }
Result PXIFSR_ReadFileSHA256(FSPXI_File file, u32* bytesRead, u64 offset, void* readBuffer, u32 readBufferSize, void* hashtable, u32 hashtableSize, u32 unk) { return FSPXI_ReadFileSHA256(pxifsRHandle, file, bytesRead, offset, readBuffer, readBufferSize, hashtable, hashtableSize, unk); }
Result PXIFSB_ReadFileSHA256(FSPXI_File file, u32* bytesRead, u64 offset, void* readBuffer, u32 readBufferSize, void* hashtable, u32 hashtableSize, u32 unk) { return FSPXI_ReadFileSHA256(pxifsBHandle, file, bytesRead, offset, readBuffer, readBufferSize, hashtable, hashtableSize, unk); }

Result PXIFS0_WriteFileSHA256(FSPXI_File file, u32* bytesWritten, u64 offset, void* writeBuffer, u32 writeBufferSize, void* hashtable, u32 hashtableSize, u32 unk1, u32 unk2) { return FSPXI_WriteFileSHA256(pxifs0Handle, file, bytesWritten, offset, writeBuffer, writeBufferSize, hashtable, hashtableSize, unk1, unk2); }
Result PXIFS1_WriteFileSHA256(FSPXI_File file, u32* bytesWritten, u64 offset, void* writeBuffer, u32 writeBufferSize, void* hashtable, u32 hashtableSize, u32 unk1, u32 unk2) { return FSPXI_WriteFileSHA256(pxifs1Handle, file, bytesWritten, offset, writeBuffer, writeBufferSize, hashtable, hashtableSize, unk1, unk2); }
Result PXIFSR_WriteFileSHA256(FSPXI_File file, u32* bytesWritten, u64 offset, void* writeBuffer, u32 writeBufferSize, void* hashtable, u32 hashtableSize, u32 unk1, u32 unk2) { return FSPXI_WriteFileSHA256(pxifsRHandle, file, bytesWritten, offset, writeBuffer, writeBufferSize, hashtable, hashtableSize, unk1, unk2); }
Result PXIFSB_WriteFileSHA256(FSPXI_File file, u32* bytesWritten, u64 offset, void* writeBuffer, u32 writeBufferSize, void* hashtable, u32 hashtableSize, u32 unk1, u32 unk2) { return FSPXI_WriteFileSHA256(pxifsBHandle, file, bytesWritten, offset, writeBuffer, writeBufferSize, hashtable, hashtableSize, unk1, unk2); }

Result PXIFS0_0x4F(u64 unk) { return FSPXI_0x4F(pxifs0Handle, unk); }
Result PXIFS1_0x4F(u64 unk) { return FSPXI_0x4F(pxifs1Handle, unk); }
Result PXIFSR_0x4F(u64 unk) { return FSPXI_0x4F(pxifsRHandle, unk); }
Result PXIFSB_0x4F(u64 unk) { return FSPXI_0x4F(pxifsBHandle, unk); }

Result PXIFS0_SetPriority(u32 priority) { return FSPXI_SetPriority(pxifs0Handle, priority); }
Result PXIFS1_SetPriority(u32 priority) { return FSPXI_SetPriority(pxifs1Handle, priority); }
Result PXIFSR_SetPriority(u32 priority) { return FSPXI_SetPriority(pxifsRHandle, priority); }
Result PXIFSB_SetPriority(u32 priority) { return FSPXI_SetPriority(pxifsBHandle, priority); }

Result PXIFS0_SwitchCleanupInvalidSaveData(bool enable) { return FSPXI_SwitchCleanupInvalidSaveData(pxifs0Handle, enable); }
Result PXIFS1_SwitchCleanupInvalidSaveData(bool enable) { return FSPXI_SwitchCleanupInvalidSaveData(pxifs1Handle, enable); }
Result PXIFSR_SwitchCleanupInvalidSaveData(bool enable) { return FSPXI_SwitchCleanupInvalidSaveData(pxifsRHandle, enable); }
Result PXIFSB_SwitchCleanupInvalidSaveData(bool enable) { return FSPXI_SwitchCleanupInvalidSaveData(pxifsBHandle, enable); }

Result PXIFS0_EnumerateSystemSaveData(u32* idsWritten, u32 idsSize, u32* ids) { return FSPXI_EnumerateSystemSaveData(pxifs0Handle, idsWritten, idsSize, ids); }
Result PXIFS1_EnumerateSystemSaveData(u32* idsWritten, u32 idsSize, u32* ids) { return FSPXI_EnumerateSystemSaveData(pxifs1Handle, idsWritten, idsSize, ids); }
Result PXIFSR_EnumerateSystemSaveData(u32* idsWritten, u32 idsSize, u32* ids) { return FSPXI_EnumerateSystemSaveData(pxifsRHandle, idsWritten, idsSize, ids); }
Result PXIFSB_EnumerateSystemSaveData(u32* idsWritten, u32 idsSize, u32* ids) { return FSPXI_EnumerateSystemSaveData(pxifsBHandle, idsWritten, idsSize, ids); }

Result PXIFS0_ReadNandReport(void* buffer, u32 size, u32 unk) { return FSPXI_ReadNandReport(pxifs0Handle, buffer, size, unk); }
Result PXIFS1_ReadNandReport(void* buffer, u32 size, u32 unk) { return FSPXI_ReadNandReport(pxifs1Handle, buffer, size, unk); }
Result PXIFSR_ReadNandReport(void* buffer, u32 size, u32 unk) { return FSPXI_ReadNandReport(pxifsRHandle, buffer, size, unk); }
Result PXIFSB_ReadNandReport(void* buffer, u32 size, u32 unk) { return FSPXI_ReadNandReport(pxifsBHandle, buffer, size, unk); }

Result PXIFS0_0x56(u32 (*out)[4], FS_Archive archive, FS_Path path) { return FSPXI_0x56(pxifs0Handle, out, archive, path); }
Result PXIFS1_0x56(u32 (*out)[4], FS_Archive archive, FS_Path path) { return FSPXI_0x56(pxifs1Handle, out, archive, path); }
Result PXIFSR_0x56(u32 (*out)[4], FS_Archive archive, FS_Path path) { return FSPXI_0x56(pxifsRHandle, out, archive, path); }
Result PXIFSB_0x56(u32 (*out)[4], FS_Archive archive, FS_Path path) { return FSPXI_0x56(pxifsBHandle, out, archive, path); }
