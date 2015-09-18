#pragma once

#include <3ds/types.h>

typedef struct
{
	u32 headerSize;
	u32 dirHashTableOff, dirHashTableSize;
	u32 dirTableOff, dirTableSize;
	u32 fileHashTableOff, fileHashTableSize;
	u32 fileTableOff, fileTableSize;
	u32 fileDataOff;
} romfs_header;

typedef struct
{
	u32 parent, sibling;
	u32 childDir, childFile;
	u32 nextHash;
	u32 nameLen;
	u16 name[];
} romfs_dir;

typedef struct
{
	u32 parent, sibling;
	u64 dataOff, dataSize;
	u32 nextHash;
	u32 nameLen;
	u16 name[];
} romfs_file;

Result romfsInit(void);
Result romfsInitFromFile(Handle file, u32 offset);
Result romfsExit(void);
