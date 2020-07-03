/**
 * @file romfs.h
 * @brief RomFS driver.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/fs.h>

/// RomFS header.
typedef struct
{
	u32 headerSize;        ///< Size of the header.
	u32 dirHashTableOff;   ///< Offset of the directory hash table.
	u32 dirHashTableSize;  ///< Size of the directory hash table.
	u32 dirTableOff;       ///< Offset of the directory table.
	u32 dirTableSize;      ///< Size of the directory table.
	u32 fileHashTableOff;  ///< Offset of the file hash table.
	u32 fileHashTableSize; ///< Size of the file hash table.
	u32 fileTableOff;      ///< Offset of the file table.
	u32 fileTableSize;     ///< Size of the file table.
	u32 fileDataOff;       ///< Offset of the file data.
} romfs_header;

/// RomFS directory.
typedef struct
{
	u32 parent;    ///< Offset of the parent directory.
	u32 sibling;   ///< Offset of the next sibling directory.
	u32 childDir;  ///< Offset of the first child directory.
	u32 childFile; ///< Offset of the first file.
	u32 nextHash;  ///< Directory hash table pointer.
	u32 nameLen;   ///< Name length.
	u16 name[];    ///< Name. (UTF-16)
} romfs_dir;

/// RomFS file.
typedef struct
{
	u32 parent;   ///< Offset of the parent directory.
	u32 sibling;  ///< Offset of the next sibling file.
	u64 dataOff;  ///< Offset of the file's data.
	u64 dataSize; ///< Length of the file's data.
	u32 nextHash; ///< File hash table pointer.
	u32 nameLen;  ///< Name length.
	u16 name[];   ///< Name. (UTF-16)
} romfs_file;

/**
 * @brief Mounts the Application's RomFS.
 * @param name Device mount name.
 * @remark This function is intended to be used to access one's own RomFS.
 *         If the application is running as 3DSX, it mounts the embedded RomFS section inside the 3DSX.
 *         If on the other hand it's an NCCH, it behaves identically to \ref romfsMountFromCurrentProcess.
 */
Result romfsMountSelf(const char *name);

/**
 * @brief Mounts RomFS from an open file.
 * @param fd FSFILE handle of the RomFS image.
 * @param offset Offset of the RomFS within the file.
 * @param name Device mount name.
 */
Result romfsMountFromFile(Handle fd, u32 offset, const char *name);

/**
 * @brief Mounts RomFS using the current process host program RomFS.
 * @param name Device mount name.
 */
Result romfsMountFromCurrentProcess(const char *name);

/**
 * @brief Mounts RomFS from the specified title.
 * @param tid Title ID
 * @param mediatype Mediatype
 * @param name Device mount name.
 */
Result romfsMountFromTitle(u64 tid, FS_MediaType mediatype, const char* name);

/// Unmounts the RomFS device.
Result romfsUnmount(const char *name);

/// Wrapper for \ref romfsMountSelf with the default "romfs" device name.
static inline Result romfsInit(void)
{
	return romfsMountSelf("romfs");
}

/// Wrapper for \ref romfsUnmount with the default "romfs" device name.
static inline Result romfsExit(void)
{
	return romfsUnmount("romfs");
}
