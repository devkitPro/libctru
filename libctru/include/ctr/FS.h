#ifndef FS_H
#define FS_H

#define FS_OPEN_READ (1<<0)
#define FS_OPEN_WRITE (1<<1)
#define FS_OPEN_CREATE (1<<2)

#define FS_ATTRIBUTE_NONE (0x00000000)
#define FS_ATTRIBUTE_READONLY (0x00000001)
#define FS_ATTRIBUTE_ARCHIVE (0x00000100)
#define FS_ATTRIBUTE_HIDDEN (0x00010000)
#define FS_ATTRIBUTE_DIRECTORY (0x01000000)

typedef enum{
	PATH_INVALID = 0,	// Specifies an invalid path.
	PATH_EMPTY = 1,		// Specifies an empty path.
	PATH_BINARY = 2,	// Specifies a binary path, which is non-text based.
	PATH_CHAR = 3,		// Specifies a text based path with a 8-bit byte per character.
	PATH_WCHAR = 4,		// Specifies a text based path with a 16-bit short per character.
}FS_pathType;

typedef struct{
	FS_pathType type;
	u32 size;
	u8* data;
}FS_path;

typedef struct{
	u32 id;
	FS_path lowPath;
	Handle handleLow, handleHigh;
}FS_archive;

static inline FS_path FS_makePath(FS_pathType type, char* path)
{
	return (FS_path){type, strlen(path)+1, (u8*)path};
}

Result FSUSER_Initialize(Handle handle);
Result FSUSER_OpenArchive(Handle handle, FS_archive* archive);
Result FSUSER_OpenDirectory(Handle handle, Handle* out, FS_archive archive, FS_path dirLowPath);
Result FSUSER_OpenFile(Handle handle, Handle* out, FS_archive archive, FS_path fileLowPath, u32 openflags, u32 attributes);
Result FSUSER_OpenFileDirectly(Handle handle, Handle* out, FS_archive archive, FS_path fileLowPath, u32 openflags, u32 attributes);
Result FSUSER_CloseArchive(Handle handle, FS_archive* archive);

Result FSFILE_Close(Handle handle);
Result FSFILE_Read(Handle handle, u32 *bytesRead, u64 offset, u32 *buffer, u32 size);
Result FSFILE_Write(Handle handle, u32 *bytesWritten, u64 offset, u32 *buffer, u32 size, u32 flushFlags);
Result FSFILE_GetSize(Handle handle, u64 *size);
Result FSFILE_SetSize(Handle handle, u64 size);

Result FSDIR_Read(Handle handle, u32 *entriesRead, u32 entrycount, u16 *buffer);
Result FSDIR_Close(Handle handle);

#endif
