/**
 * @file FS.h
 * @brief Filesystem Services
 */
#pragma once

#include <3ds/types.h>

///@name FS Open Flags
///@{
/// Open file for read.
#define FS_OPEN_READ   (1<<0)
/// Open file for write.
#define FS_OPEN_WRITE  (1<<1)
/// Create file if it doesn't exist.
#define FS_OPEN_CREATE (1<<2)
/// @}

///@name FS Create Attributes
///@{
/// No attributes.
#define FS_ATTRIBUTE_NONE      (0x00000000)
/// Create with read-only attribute.
#define FS_ATTRIBUTE_READONLY  (0x00000001)
/// Create with archive attribute.
#define FS_ATTRIBUTE_ARCHIVE   (0x00000100)
/// Create with hidden attribute.
#define FS_ATTRIBUTE_HIDDEN    (0x00010000)
/// Create with directory attribute.
#define FS_ATTRIBUTE_DIRECTORY (0x01000000)
/// @}

///@name FS Flush Flags
///@{
/// Don't flush
#define FS_WRITE_NOFLUSH (0x00000000)
/// Flush
#define FS_WRITE_FLUSH   (0x00010001)
/// @}

/// FS path type.
typedef enum
{
	PATH_INVALID = 0, ///< Specifies an invalid path.
	PATH_EMPTY   = 1, ///< Specifies an empty path.
	PATH_BINARY  = 2, ///< Specifies a binary path, which is non-text based.
	PATH_CHAR    = 3, ///< Specifies a text based path with a 8-bit byte per character.
	PATH_WCHAR   = 4, ///< Specifies a text based path with a 16-bit short per character.
} FS_pathType;

/// FS archive IDs.
typedef enum
{
	ARCH_ROMFS = 0x3,                       ///< RomFS archive.
	ARCH_SAVEDATA = 0x4,                    ///< Save data archive.
	ARCH_EXTDATA = 0x6,                     ///< Ext data archive.
	ARCH_SHARED_EXTDATA = 0x7,              ///< Shared ext data archive.
	ARCH_SYSTEM_SAVEDATA = 0x8,             ///< System save data archive.
	ARCH_SDMC = 0x9,                        ///< SDMC archive.
	ARCH_SDMC_WRITE_ONLY = 0xA,             ///< Write-only SDMC archive.
	ARCH_BOSS_EXTDATA = 0x12345678,         ///< BOSS ext data archive.
	ARCH_CARD_SPIFS = 0x12345679,           ///< Card SPIFS archive.
	ARCH_NAND_RW = 0x1234567D,              ///< Read-write NAND archive.
	ARCH_NAND_RO = 0x1234567E,              ///< Read-only NAND archive.
	ARCH_NAND_RO_WRITE_ACCESS = 0x1234567F, ///< Read-only write access NAND archive.
} FS_archiveIds;

/// FS path.
typedef struct
{
	FS_pathType type;  ///< FS path type.
	u32         size;  ///< FS path size.
	const u8    *data; ///< Pointer to FS path data.
} FS_path;

/// FS archive.
typedef struct
{
	u32     id;         ///< Archive ID.
	FS_path lowPath;    ///< FS path.
	Handle  handleLow;  ///< High word of handle.
	Handle  handleHigh; ///< Low word of handle.
} FS_archive;

/// Directory entry.
typedef struct
{
  // 0x00
  u16 name[0x106];     ///< UTF-16 encoded name
  // 0x20C
  u8  shortName[0x09]; ///< 8.3 File name
  // 0x215
  u8  unknown1;        ///< ???
  // 0x216
  u8  shortExt[0x04];  ///< 8.3 File extension (set to spaces for directories)
  // 0x21A
  u8  unknown2;        ///< ???
  // 0x21B
  u8  unknown3;        ///< ???
  // 0x21C
  u8  isDirectory;     ///< Directory bit
  // 0x21D
  u8  isHidden;        ///< Hidden bit
  // 0x21E
  u8  isArchive;       ///< Archive bit
  // 0x21F
  u8  isReadOnly;      ///< Read-only bit
  // 0x220
  u64 fileSize;        ///< File size
} FS_dirent;

/// Initializes FS.
Result fsInit(void);

/// Exits FS.
void fsExit(void);

/**
 * @brief Gets the current FS session handle.
 * @return The current FS session handle.
 */
Handle *fsGetSessionHandle(void);

/**
 * Creates an FS_path instance.
 * @param type Type of path.
 * @param path Path to use.
 * @return The created FS_path instance.
 */
FS_path FS_makePath(FS_pathType type, const char *path);

/**
 * @brief Initializes FSUSER.
 * @param handle FS:USER service handle to use.
 */
Result FSUSER_Initialize(Handle handle);

/**
 * @brief Opens an archive.
 * @param archive Archive to open.
 */
Result FSUSER_OpenArchive(FS_archive* archive);

/**
 * @brief Opens a directory.
 * @param out Pointer to output the directory handle to.
 * @param archive Archive to open the directory from.
 * @param dirLowPath Path of the directory.
 */
Result FSUSER_OpenDirectory(Handle* out, FS_archive archive, FS_path dirLowPath);

/**
 * @brief Opens a file.
 * @param out Pointer to output the file handle to.
 * @param archive Archive to open the file from.
 * @param fileLowPath Path of the file.
 * @param openflags Open flags to use.
 * @param attributes Attributes to use.
 */
Result FSUSER_OpenFile(Handle* out, FS_archive archive, FS_path fileLowPath, u32 openflags, u32 attributes);

/**
 * @brief Opens a file directly.
 * @param out Pointer to output the file handle to.
 * @param archive Archive to open the file from.
 * @param fileLowPath Path of the file.
 * @param openflags Open flags to use.
 * @param attributes Attributes to use.
 */
Result FSUSER_OpenFileDirectly(Handle* out, FS_archive archive, FS_path fileLowPath, u32 openflags, u32 attributes);

/**
 * @brief Closes an archive.
 * @param archive Archive to close.
 */
Result FSUSER_CloseArchive(FS_archive* archive);

/**
 * @brief Creates a file.
 * @param archive Archive to use.
 * @param fileLowPath Path of the file.
 * @param fileSize Initial size of the file.
 */
Result FSUSER_CreateFile(FS_archive archive, FS_path fileLowPath, u32 fileSize);

/**
 * @brief Creates a directory.
 * @param archive Archive to use.
 * @param dirLowPath Path of the directory.
 */
Result FSUSER_CreateDirectory(FS_archive archive, FS_path dirLowPath);

/**
 * @brief Deletes a file.
 * @param archive Archive to use.
 * @param fileLowPath Path of the file.
 */
Result FSUSER_DeleteFile(FS_archive archive, FS_path fileLowPath);

/**
 * @brief Deletes a directory.
 * @param archive Archive to use.
 * @param dirLowPath Path of the directory.
 */
Result FSUSER_DeleteDirectory(FS_archive archive, FS_path dirLowPath);

/**
 * @brief Deletes a directory recursively.
 * @param archive Archive to use.
 * @param dirLowPath Path of the directory.
 */
Result FSUSER_DeleteDirectoryRecursively(FS_archive archive, FS_path dirLowPath);

/**
 * @brief Renames a file.
 * @param srcArchive Source archive.
 * @param srcFileLowPath Source file.
 * @param destArchive Destination archive.
 * @param destFileLowPath Destination file.
 */
Result FSUSER_RenameFile(FS_archive srcArchive, FS_path srcFileLowPath, FS_archive destArchive, FS_path destFileLowPath);

/**
 * @brief Renames a directory.
 * @param srcArchive Source archive.
 * @param srcDirLowPath Source directory.
 * @param destArchive Destination archive.
 * @param destDirLowPath Destination directory.
 */
Result FSUSER_RenameDirectory(FS_archive srcArchive, FS_path srcDirLowPath, FS_archive destArchive, FS_path destDirLowPath);

/**
 * @brief Gets the SDMC resource info.
 * @param sectorSize Pointer to output the sector size to.
 * @param sectorSize Pointer to output the cluster size to.
 * @param sectorSize Pointer to output the total number of clusters to.
 * @param sectorSize Pointer to output the number of free clusters to.
 */
Result FSUSER_GetSdmcArchiveResource(u32 *sectorSize, u32 *clusterSize, u32 *numClusters, u32 *freeClusters);

/**
 * @brief Gets the NAND resource info.
 * @param sectorSize Pointer to output the sector size to.
 * @param sectorSize Pointer to output the cluster size to.
 * @param sectorSize Pointer to output the total number of clusters to.
 * @param sectorSize Pointer to output the number of free clusters to.
 */
Result FSUSER_GetNandArchiveResource(u32 *sectorSize, u32 *clusterSize, u32 *numClusters, u32 *freeClusters);

/**
 * @brief Gets whether an SD card is detected.
 * @param detected Pointer to output the SD detection state to.
 */
Result FSUSER_IsSdmcDetected(u8 *detected);

/**
 * @brief Gets whether the SD card is writable.
 * @param detected Pointer to output the SD writable state to.
 */
Result FSUSER_IsSdmcWritable(u8 *writable);

/**
 * @brief Gets the media type of the current application.
 * @param mediatype Pointer to output the media type to.
 */
Result FSUSER_GetMediaType(u8* mediatype);

/**
 * @brief Closes a file handle.
 * @param handle File handle to close.
 */
Result FSFILE_Close(Handle handle);

/**
 * @brief Reads from a file.
 * @param handle File handle to use.
 * @param bytesRead Pointer to output the number of bytes read to.
 * @param offset Offset to read from.
 * @param buffer Buffer to read to.
 * @param size Size of the buffer.
 */
Result FSFILE_Read(Handle handle, u32 *bytesRead, u64 offset, void *buffer, u32 size);

/**
 * @brief Writes to a file.
 * @param handle File handle to use.
 * @param bytesRead Pointer to output the number of bytes written to.
 * @param offset Offset to write to.
 * @param buffer Buffer to write from.
 * @param size Size of the buffer.
 * @param flushFlags Flush flags to apply after writing.
 */
Result FSFILE_Write(Handle handle, u32 *bytesWritten, u64 offset, const void *buffer, u32 size, u32 flushFlags);

/**
 * @brief Gets a file's size.
 * @param handle File handle to use.
 * @param size Pointer to output the size to.
 */
Result FSFILE_GetSize(Handle handle, u64 *size);

/**
 * @brief Sets a file's size.
 * @param handle File handle to use.
 * @param size Size to set.
 */
Result FSFILE_SetSize(Handle handle, u64 size);

/**
 * @brief Gets a file's attributes.
 * @param handle File handle to use.
 * @param attributes Pointer to output the attributes to.
 */
Result FSFILE_GetAttributes(Handle handle, u32 *attributes);

/**
 * @brief Sets a file's attributes.
 * @param handle File handle to use.
 * @param attributes Attributes to set.
 */
Result FSFILE_SetAttributes(Handle handle, u32 attributes);

/**
 * @brief Flushes a file to disk.
 * @param handle File handle to flush.
 */
Result FSFILE_Flush(Handle handle);

/**
 * @brief Reads one or more directory entries.
 * @param handle Directory handle to read from.
 * @param entriesRead Pointer to output the current number of read entries to.
 * @param entrycount Number of entries to read.
 * @param buffer Buffer to output directory entries to.
 */
Result FSDIR_Read(Handle handle, u32 *entriesRead, u32 entrycount, FS_dirent *buffer);

/**
 * @brief Closes a directory handle.
 * @param handle Directory handle to close.
 */
Result FSDIR_Close(Handle handle);
