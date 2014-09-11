#include <string.h>
#include <3ds.h>

/*! @internal
 *
 *  @file fs.c
 * 
 *  Filesystem Services
 */

/*! FSUSER handle */
static Handle fsuHandle;

// used to determine whether or not we should do FSUSER_Initialize on fsuHandle
Handle __get_handle_from_list(char* name);

/*! Initialize FS service
 *
 *  @returns error
 */
Result
fsInit(void)
{
	Result ret;
	if((ret=srvGetServiceHandle(&fsuHandle, "fs:USER"))!=0)return ret;
	if(__get_handle_from_list("fs:USER")==0)ret=FSUSER_Initialize(NULL);
	return ret;
}

/*! Deinitialize FS service
 *
 *  @returns error
 */
Result
fsExit(void)
{
	return svcCloseHandle(fsuHandle);
}

/*! Initialize FS service handle
 *
 *  If @a handle is NULL, this initializes @ref fsuHandle.
 *
 *  @param[in] handle fs:USER service handle
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08010002]
 *  1          | 0x20 (ProcessID header)
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSUSER_Initialize(Handle* handle)
{
	if(!handle)
	{
		// don't run command if we got handle from the list
		handle = &fsuHandle;
		if(fsuHandle != 0 && __get_handle_from_list("fs:USER")!=0)
			return 0;
	}

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08010002;
	cmdbuf[1] = 0x20;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	return cmdbuf[1];
}

/*! Open a file
 *
 *  @param[in]  handle      fs:USER handle
 *  @param[out] out         Output handle
 *  @param[in]  archive     Open archive
 *  @param[in]  fileLowPath File path
 *  @param[in]  openFlags   Open flags
 *  @param[in]  attributes  Create attributes
 *
 *  @note This requires @a archive to have been opened
 *
 *  @returns error
 *
 *  @sa fs_open_flags
 *  @sa fs_create_attributes
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x080201C2]
 *  1          | Transaction (usually 0)
 *  2          | archive.handleLow
 *  3          | archive.handleHigh
 *  4          | fileLowPath.type
 *  5          | fileLowPath.size
 *  6          | openFlags
 *  7          | attributes
 *  8          | (fileLowPath.size << 14) \| 0x2
 *  9          | fileLowPath.data
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | ???
 *  3          | File handle
 */
Result
FSUSER_OpenFile(Handle     *handle,
                Handle     *out,
                FS_archive archive,
                FS_path    fileLowPath,
                u32        openFlags,
                u32        attributes)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x080201C2;
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = fileLowPath.type;
	cmdbuf[5] = fileLowPath.size;
	cmdbuf[6] = openFlags;
	cmdbuf[7] = attributes;
	cmdbuf[8] = (fileLowPath.size << 14) | 0x2;
	cmdbuf[9] = (u32)fileLowPath.data;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	if(out)
		*out = cmdbuf[3];

	return cmdbuf[1];
}

/*! Open a file
 *
 *  @param[in]  handle      fs:USER handle
 *  @param[out] out         Output handle
 *  @param[in]  archive     Open archive
 *  @param[in]  fileLowPath File path
 *  @param[in]  openFlags   Open flags
 *  @param[in]  attributes  Create attributes
 *
 *  @note This does not require @a archive to have been opened
 *
 *  @returns error
 *
 *  @sa fs_open_flags
 *  @sa fs_create_attributes
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *   0         | Header code [0x08030204]
 *   1         | Transaction (usually 0)
 *   2         | archive.id
 *   3         | archive.lowPath.type
 *   4         | archive.lowPath.Size
 *   5         | fileLowPath.type
 *   6         | fileLowPath.size
 *   7         | openFlags
 *   8         | attributes
 *   9         | (archive.lowPath.size << 14 \| 0x802
 *  10         | archive.lowPath.data
 *  11         | (fileLowPath.size << 14) \| 0x2
 *  12         | fileLowPath.data
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | ???
 *  3          | File handle
 */
Result
FSUSER_OpenFileDirectly(Handle     *handle,
                        Handle     *out,
                        FS_archive archive,
                        FS_path    fileLowPath,
                        u32        openFlags,
                        u32        attributes)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[ 0] = 0x08030204;
	cmdbuf[ 1] = 0;
	cmdbuf[ 2] = archive.id;
	cmdbuf[ 3] = archive.lowPath.type;
	cmdbuf[ 4] = archive.lowPath.size;
	cmdbuf[ 5] = fileLowPath.type;
	cmdbuf[ 6] = fileLowPath.size;
	cmdbuf[ 7] = openFlags;
	cmdbuf[ 8] = attributes;
	cmdbuf[ 9] = (archive.lowPath.size << 14) | 0x802;
	cmdbuf[10] = (u32)archive.lowPath.data;
	cmdbuf[11] = (fileLowPath.size << 14) | 0x2;
	cmdbuf[12] = (u32)fileLowPath.data;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	if(out)
		*out = cmdbuf[3];

	return cmdbuf[1];
}

/*! Delete a file
 *
 *  @param[in] handle      fs:USER handle
 *  @param[in] archive     Open archive
 *  @param[in] fileLowPath File path
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08040142]
 *  1          | 0
 *  2          | archive.handleLow
 *  3          | archive.handleHigh
 *  4          | fileLowPath.type
 *  5          | fileLowPath.size
 *  6          | (fileLowPath.size << 14) \| 0x2
 *  7          | fileLowPath.data
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSUSER_DeleteFile(Handle     *handle,
                  FS_archive archive,
                  FS_path    fileLowPath)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08040142;
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = fileLowPath.type;
	cmdbuf[5] = fileLowPath.size;
	cmdbuf[6] = (fileLowPath.size << 14) | 0x2;
	cmdbuf[7] = (u32)fileLowPath.data;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	return cmdbuf[1];
}

/* stub */
Result
FSUSER_RenameFile(void)
{
	return -1;
}

/*! Delete a directory
 *
 *  @param[in] handle     fs:USER handle
 *  @param[in] archive    Open archive
 *  @param[in] dirLowPath Directory path
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08060142]
 *  1          | 0
 *  2          | archive.handleLow
 *  3          | archive.handleHigh
 *  4          | dirLowPath.type
 *  5          | dirLowPath.size
 *  6          | (dirLowPath.size << 14) \| 0x2
 *  7          | dirLowPath.data
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSUSER_DeleteDirectory(Handle     *handle,
                       FS_archive archive,
                       FS_path    dirLowPath)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08060142;
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = dirLowPath.type;
	cmdbuf[5] = dirLowPath.size;
	cmdbuf[6] = (dirLowPath.size << 14) | 0x2;
	cmdbuf[7] = (u32)dirLowPath.data;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	return cmdbuf[1];
}

/* stub */
Result
FSUSER_DeleteDirectoryRecursively(void)
{
	return -1;
}

/* stub */
Result
FSUSER_CreateFile(void)
{
	return -1;
}

/*! Create a directory
 *
 *  @param[in] handle     fs:USER handle
 *  @param[in] archive    Open archive
 *  @param[in] dirLowPath Directory path to create
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08090182]
 *  1          | 0
 *  2          | archive.handleLow
 *  3          | archive.handleHigh
 *  4          | dirLowPath.type
 *  5          | dirLowPath.size
 *  6          | 0
 *  7          | (dirLowPath.size << 14) \| 0x2
 *  8          | dirLowPath.data
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSUSER_CreateDirectory(Handle     *handle,
                       FS_archive archive,
                       FS_path    dirLowPath)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08090182;
	cmdbuf[1] = 0;
	cmdbuf[2] = archive.handleLow;
	cmdbuf[3] = archive.handleHigh;
	cmdbuf[4] = dirLowPath.type;
	cmdbuf[5] = dirLowPath.size;
	cmdbuf[6] = 0;
	cmdbuf[7] = (dirLowPath.size << 14) | 0x2;
	cmdbuf[8] = (u32)dirLowPath.data;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	return cmdbuf[1];
}

/* stub */
Result
FSUSER_RenameDirectory(void)
{
	return -1;
}

/*! Open a directory
 *
 *  @param[in]  handle     fs:USER handle
 *  @param[out] out        Output handle
 *  @param[in]  archive    Open archive
 *  @param[in]  dirLowPath Directory path
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x080B0102]
 *  1          | archive.handleLow
 *  2          | archive.handleHigh
 *  3          | dirLowPath.type
 *  4          | dirLowPath.size
 *  5          | (dirLowPath.size << 14) \| 0x2
 *  6          | dirLowPath.data
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | Directory handle
 */
Result
FSUSER_OpenDirectory(Handle     *handle,
                     Handle     *out,
                     FS_archive archive,
                     FS_path    dirLowPath)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x080B0102;
	cmdbuf[1] = archive.handleLow;
	cmdbuf[2] = archive.handleHigh;
	cmdbuf[3] = dirLowPath.type;
	cmdbuf[4] = dirLowPath.size;
	cmdbuf[5] = (dirLowPath.size << 14) | 0x2;
	cmdbuf[6] = (u32)dirLowPath.data;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	if(out)
		*out = cmdbuf[3];

	return cmdbuf[1];
}

/*! Open an archive
 *
 *  @param[in]     handle  fs:USER handle
 *  @param[in,out] archive Archive to open
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x080C00C2]
 *  1          | archive->id
 *  2          | archive->lowPath.type
 *  3          | archive->lowPath.size
 *  4          | (archive->lowPath.size << 14) \| 0x2
 *  5          | archive->lowPath.data
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | archive->handleLow
 *  3          | archive->handleHigh
 */
Result
FSUSER_OpenArchive(Handle     *handle,
                   FS_archive *archive)
{
	if(!archive)
		return -2;

	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x080C00C2;
	cmdbuf[1] = archive->id;
	cmdbuf[2] = archive->lowPath.type;
	cmdbuf[3] = archive->lowPath.size;
	cmdbuf[4] = (archive->lowPath.size << 14) | 0x2;
	cmdbuf[5] = (u32)archive->lowPath.data;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	archive->handleLow  = cmdbuf[2];
	archive->handleHigh = cmdbuf[3];

	return cmdbuf[1];
}


/*! Close an open archive
 *
 *  @param[in]     handle  fs:USER handle
 *  @param[in,out] archive Archive to close
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x080B0102]
 *  1          | archive->handleLow
 *  2          | archive->handleHigh
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSUSER_CloseArchive(Handle     *handle,
                    FS_archive *archive)
{
	if(!archive)
		return -2;

	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x080E0080;
	cmdbuf[1] = archive->handleLow;
	cmdbuf[2] = archive->handleHigh;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	return cmdbuf[1];
}

/*! Get SD FAT information
 *
 *  @param[in]  handle       fs:USER handle
 *  @param[out] sectorSize   Sector size (bytes)
 *  @param[out] clusterSize  Cluster size (bytes)
 *  @param[out] numClusters  Total number of clusters
 *  @param[out] freeClusters Number of free clusters
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08140000]
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | Sector (bytes)
 *  3          | Cluster (bytes)
 *  4          | Partition capacity (clusters)
 *  5          | Free space (clusters)
 */
Result
FSUSER_GetSdmcArchiveResource(Handle *handle,
                              u32    *sectorSize,
                              u32    *clusterSize,
                              u32    *numClusters,
                              u32    *freeClusters)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08140000;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	if(sectorSize)
		*sectorSize = cmdbuf[2];

	if(clusterSize)
		*clusterSize = cmdbuf[3];

	if(numClusters)
		*numClusters = cmdbuf[4];

	if(freeClusters)
		*freeClusters = cmdbuf[5];

	return cmdbuf[1];
}

/*! Check if SD card is detected
 *
 *  @param[in]  handle   fs:USER handle
 *  @param[out] detected Output detected state
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08170000]
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | Whether SD is detected
 */
Result
FSUSER_IsSdmcDetected(Handle *handle,
                      u32    *detected)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08170000;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	if(detected)
		*detected = cmdbuf[2];

	return cmdbuf[1];
}

/*! Check if SD card is writable
 *
 *  @param[in]  handle   fs:USER handle
 *  @param[out] writable Output writable state
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08180000]
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | Whether SD is writable
 */
Result
FSUSER_IsSdmcWritable(Handle *handle,
                      u32    *writable)
{
	if(!handle)
		handle = &fsuHandle;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08180000;

	Result ret = 0;
	if((ret = svcSendSyncRequest(*handle)))
		return ret;

	if(writable)
		*writable = cmdbuf[2];

	return cmdbuf[1];
}

/*! Close an open file
 *
 *  @param[in] handle Open file handle
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08080000]
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSFILE_Close(Handle handle)
{
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08080000;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	return cmdbuf[1];
}

/*! Read data from an open file
 *
 *  @param[in]  handle    Open file handle
 *  @param[out] bytesRead Number of bytes read
 *  @param[in]  offset    File offset to read from
 *  @param[out] buffer    Buffer to read into
 *  @param[in]  size      Number of bytes to read
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x080200C2]
 *  1          | offset (low word)
 *  2          | offset (high word)
 *  3          | size
 *  4          | (size << 4) \| 0xC
 *  5          | buffer
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | Number of bytes read
 */
Result
FSFILE_Read(Handle handle,
            u32    *bytesRead,
            u64    offset,
            void   *buffer,
            u32    size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x080200C2;
	cmdbuf[1] = (u32)offset;
	cmdbuf[2] = (u32)(offset >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = (size << 4) | 0xC;
	cmdbuf[5] = (u32)buffer;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	if(bytesRead)
		*bytesRead = cmdbuf[2];

	return cmdbuf[1];
}

/*! Write data to an open file
 *
 *  @param[in]  handle       Open file handle
 *  @param[out] bytesWritten Number of bytes read
 *  @param[in]  offset       File offset to write to
 *  @param[in]  buffer       Buffer to write from
 *  @param[in]  size         Number of bytes to write
 *  @param[in]  flushFlags   Flush flags
 *
 *  @returns error
 *
 *  @sa fs_write_flush_flags
 *
 *  @warning
 *    Using invalid flushFlags can corrupt the archive you're writing to.
 *
 *  @warning
 *    Data should not be in read-only memory.
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08030102]
 *  1          | offset (low word)
 *  2          | offset (high word)
 *  3          | size
 *  4          | flushFlags
 *  5          | (size << 4) \| 0xA
 *  6          | buffer
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | Number of bytes written
 */
Result
FSFILE_Write(Handle     handle,
             u32        *bytesWritten,
             u64        offset,
             const void *buffer,
             u32        size,
             u32        flushFlags)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08030102;
	cmdbuf[1] = (u32)offset;
	cmdbuf[2] = (u32)(offset >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = flushFlags;
	cmdbuf[5] = (size << 4) | 0xA;
	cmdbuf[6] = (u32)buffer;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	if(bytesWritten)
		*bytesWritten = cmdbuf[2];

	return cmdbuf[1];
}

/*! Get the size of an open file
 *
 *  @param[in]  handle Open file handle
 *  @param[out] size   Output size
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08040000]
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | File size (lower word)
 *  3          | File size (upper word)
 */
Result
FSFILE_GetSize(Handle handle,
               u64    *size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08040000;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	if(size)
		*size = (u64)cmdbuf[2] | ((u64)cmdbuf[3] << 32);

	return cmdbuf[1];
}

/*! Set the size of an open file
 *
 *  @param[in] handle Open file handle
 *  @param[in] size   Size to set
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08050080]
 *  1          | size (lower word)
 *  2          | size (upper word)
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSFILE_SetSize(Handle handle,
               u64    size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08050080;
	cmdbuf[1] = (u32)size;
	cmdbuf[2] = (u32)(size >> 32);

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;


	return cmdbuf[1];
}

/*! Get attributes for an open file
 *
 *  @param[in]  handle     Open file handle
 *  @param[out] attributes Output attributes
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08060000]
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | Attributes
 */
Result
FSFILE_GetAttributes(Handle handle,
                     u32    *attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08060000;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	if(attributes)
		*attributes = cmdbuf[2];

	return cmdbuf[1];
}

/*! Set attributes for an open file
 *
 *  @param[in] handle     Open file handle
 *  @param[in] attributes Attributes to set
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08070040]
 *  1          | Attributes
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSFILE_SetAttributes(Handle handle,
                     u32    attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08070040;
	cmdbuf[1] = attributes;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	return cmdbuf[1];
}

/*! Flush an open file
 *
 *  @param[in] handle Open file handle
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08090000]
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSFILE_Flush(Handle handle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08090000;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	return cmdbuf[1];
}

/*! Read a directory entry from an open directory
 *
 *  @param[in]  handle      Open directory handle
 *  @param[out] entriesRead Output number of entries read
 *  @param[in]  entryCount  Number of entries to read
 *  @param[out] buffer      Output buffer
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08010042]
 *  1          | entryCount
 *  2          | ((entrycount*0x228) << 4) \| 0xC
 *  3          | buffer
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 *  2          | Number of entries read
 */
Result
FSDIR_Read(Handle    handle,
           u32       *entriesRead,
           u32       entryCount,
           FS_dirent *buffer)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08010042;
	cmdbuf[1] = entryCount;
	cmdbuf[2] = ((entryCount*0x228) << 4) | 0xC;
	cmdbuf[3] = (u32)buffer;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	if(entriesRead)
		*entriesRead = cmdbuf[2];

	return cmdbuf[1];
}

/*! Close an open directory
 *
 *  @param[in] handle Open directory handle
 *
 *  @returns error
 *
 *  @internal
 *
 *  #### Request
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code [0x08020000]
 *
 *  #### Response
 *
 *  Index Word | Description
 *  -----------|-------------------------
 *  0          | Header code
 *  1          | Result code
 */
Result
FSDIR_Close(Handle handle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x08020000;

	Result ret = 0;
	if((ret = svcSendSyncRequest(handle)))
		return ret;

	return cmdbuf[1];
}
