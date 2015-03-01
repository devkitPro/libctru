#pragma once
#include <3ds/types.h>

/*! @file FS.h
 *
 *  Filesystem Services
 */

/*! @defgroup fs_open_flags FS Open Flags
 *
 *  @sa FSUSER_OpenFile
 *  @sa FSUSER_OpenFileDirectly
 *
 *  @{
 */

/*! Open file for read. */
#define FS_OPEN_READ   (1<<0)
/*! Open file for write. */
#define FS_OPEN_WRITE  (1<<1)
/*! Create file if it doesn't exist. */
#define FS_OPEN_CREATE (1<<2)
/* @} */

/*! @defgroup fs_create_attributes FS Create Attributes
 *
 *  @sa FSUSER_OpenFile
 *  @sa FSUSER_OpenFileDirectly
 *
 *  @{
 */

/*! No attributes. */
#define FS_ATTRIBUTE_NONE      (0x00000000)
/*! Create with read-only attribute. */
#define FS_ATTRIBUTE_READONLY  (0x00000001)
/*! Create with archive attribute. */
#define FS_ATTRIBUTE_ARCHIVE   (0x00000100)
/*! Create with hidden attribute. */
#define FS_ATTRIBUTE_HIDDEN    (0x00010000)
/*! Create with directory attribute. */
#define FS_ATTRIBUTE_DIRECTORY (0x01000000)
/*! @} */

/*! @defgroup fs_write_flush_flags FS Flush Flags
 *
 *  @sa FSFILE_Write
 *
 *  @{
 */

/*! Don't flush */
#define FS_WRITE_NOFLUSH (0x00000000)
/*! Flush */
#define FS_WRITE_FLUSH   (0x00010001)

/* @} */

/*! FS path type */
typedef enum
{
	PATH_INVALID = 0, //!< Specifies an invalid path.
	PATH_EMPTY   = 1, //!< Specifies an empty path.
	PATH_BINARY  = 2, //!< Specifies a binary path, which is non-text based.
	PATH_CHAR    = 3, //!< Specifies a text based path with a 8-bit byte per character.
	PATH_WCHAR   = 4, //!< Specifies a text based path with a 16-bit short per character.
} FS_pathType;

/*! FS archive ids */
typedef enum
{
	ARCH_ROMFS = 0x3,
	ARCH_SAVEDATA = 0x4,
	ARCH_EXTDATA = 0x6,
	ARCH_SHARED_EXTDATA = 0x7,
	ARCH_SYSTEM_SAVEDATA = 0x8,
	ARCH_SDMC = 0x9,
	ARCH_SDMC_WRITE_ONLY = 0xA,
	ARCH_BOSS_EXTDATA = 0x12345678,
	ARCH_CARD_SPIFS = 0x12345679,
	ARCH_NAND_RW = 0x1234567D,
	ARCH_NAND_RO = 0x1234567E,
	ARCH_NAND_RO_WRITE_ACCESS = 0x1234567F,
} FS_archiveIds;

/*! FS path */
typedef struct
{
	FS_pathType type;  //!< FS path type.
	u32         size;  //!< FS path size.
	const u8    *data; //!< Pointer to FS path data.
} FS_path;

/*! FS archive */
typedef struct
{
	u32     id;         //!< Archive ID.
	FS_path lowPath;    //!< FS path.
	Handle  handleLow;  //!< High word of handle.
	Handle  handleHigh; //!< Low word of handle.
} FS_archive;

/*! Directory entry */
typedef struct
{
  // 0x00
  u16 name[0x106];     //!< UTF-16 encoded name
  // 0x20C
  u8  shortName[0x09]; //!< 8.3 file name
  // 0x215
  u8  unknown1;        //!< ???
  // 0x216
  u8  shortExt[0x04];  //!< 8.3 file extension (set to spaces for directories)
  // 0x21A
  u8  unknown2;        //!< ???
  // 0x21B
  u8  unknown3;        //!< ???
  // 0x21C
  u8  isDirectory;     //!< directory bit
  // 0x21D
  u8  isHidden;        //!< hidden bit
  // 0x21E
  u8  isArchive;       //!< archive bit
  // 0x21F
  u8  isReadOnly;      //!< read-only bit
  // 0x220
  u64 fileSize;        //!< file size
} FS_dirent;

Result fsInit(void);
Result fsExit(void);

FS_path FS_makePath(FS_pathType type, const char  *path);

Result FSUSER_Initialize(Handle* handle);
Result FSUSER_OpenArchive(Handle* handle, FS_archive* archive);
Result FSUSER_OpenDirectory(Handle* handle, Handle* out, FS_archive archive, FS_path dirLowPath);
Result FSUSER_OpenFile(Handle* handle, Handle* out, FS_archive archive, FS_path fileLowPath, u32 openflags, u32 attributes);
Result FSUSER_OpenFileDirectly(Handle* handle, Handle* out, FS_archive archive, FS_path fileLowPath, u32 openflags, u32 attributes);
Result FSUSER_CloseArchive(Handle* handle, FS_archive* archive);
Result FSUSER_CreateFile(Handle* handle, FS_archive archive, FS_path fileLowPath, u32 fileSize);
Result FSUSER_CreateDirectory(Handle* handle, FS_archive archive, FS_path dirLowPath);
Result FSUSER_DeleteFile(Handle *handle, FS_archive archive, FS_path fileLowPath);
Result FSUSER_DeleteDirectory(Handle *handle, FS_archive archive, FS_path dirLowPath);
Result FSUSER_RenameFile(Handle *handle, FS_archive srcArchive, FS_path srcFileLowPath, FS_archive destArchive, FS_path destFileLowPath);
Result FSUSER_RenameDirectory(Handle *handle, FS_archive srcArchive, FS_path srcDirLowPath, FS_archive destArchive, FS_path destDirLowPath);
Result FSUSER_GetSdmcArchiveResource(Handle *handle, u32 *sectorSize, u32 *clusterSize, u32 *numClusters, u32 *freeClusters);
Result FSUSER_GetNandArchiveResource(Handle *handle, u32 *sectorSize, u32 *clusterSize, u32 *numClusters, u32 *freeClusters);
Result FSUSER_IsSdmcDetected(Handle *handle, u8 *detected);
Result FSUSER_IsSdmcWritable(Handle *handle, u8 *writable);

Result FSFILE_Close(Handle handle);
Result FSFILE_Read(Handle handle, u32 *bytesRead, u64 offset, void *buffer, u32 size);
Result FSFILE_Write(Handle handle, u32 *bytesWritten, u64 offset, const void *buffer, u32 size, u32 flushFlags);
Result FSFILE_GetSize(Handle handle, u64 *size);
Result FSFILE_SetSize(Handle handle, u64 size);
Result FSFILE_GetAttributes(Handle handle, u32 *attributes);
Result FSFILE_SetAttributes(Handle handle, u32 attributes);
Result FSFILE_Flush(Handle handle);

Result FSDIR_Read(Handle handle, u32 *entriesRead, u32 entrycount, FS_dirent *buffer);
Result FSDIR_Close(Handle handle);
