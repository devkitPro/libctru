/**
 * @file romfs.h
 * @brief RomFS driver.
 */
#pragma once

#include <3ds/types.h>

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

struct romfs_mount;

/**
 * @brief Mounts the Application's RomFS.
 * @param mount Output mount handle
 */
Result romfsMount(struct romfs_mount **mount);
static inline Result romfsInit(void)
{
	return romfsMount(NULL);
}

/**
 * @brief Mounts RomFS from an open file.
 * @param file Handle of the RomFS file.
 * @param offset Offset of the RomFS within the file.
 * @param mount Output mount handle
 */
Result romfsMountFromFile(Handle file, u32 offset, struct romfs_mount **mount);
static inline Result romfsInitFromFile(Handle file, u32 offset)
{
	return romfsMountFromFile(file, offset, NULL);
}

/// Bind the RomFS mount
Result romfsBind(struct romfs_mount *mount);

/// Unmounts the RomFS device.
Result romfsUnmount(struct romfs_mount *mount);
static inline Result romfsExit(void)
{
	return romfsUnmount(NULL);
}
