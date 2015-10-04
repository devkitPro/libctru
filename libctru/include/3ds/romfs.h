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

/// Initializes the RomFS driver.
Result romfsInit(void);

/**
 * @brief Initializes the RomFS driver from a RomFS file.
 * @param file Handle of the RomFS file.
 * @param offset Offset of the RomFS within the file.
 */
Result romfsInitFromFile(Handle file, u32 offset);

/// Exits the RomFS driver.
Result romfsExit(void);
