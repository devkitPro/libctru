#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dirent.h>
#include <sys/iosupport.h>
#include <sys/param.h>
#include <unistd.h>

#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/romfs.h>
#include <3ds/services/fs.h>
#include <3ds/util/utf.h>
#include <3ds/env.h>

static bool romFS_active;
static Handle romFS_file;
static time_t romFS_mtime;
static u32 romFS_offset;
static romfs_header romFS_header;
static romfs_dir* romFS_cwd;

static u32 *dirHashTable, *fileHashTable;
static void *dirTable, *fileTable;

extern int __system_argc;
extern char** __system_argv;

static char __component[PATH_MAX+1];
static uint16_t __utf16path[PATH_MAX+1];

#define romFS_root      ((romfs_dir*)dirTable)
#define romFS_dir(x)    ((romfs_dir*) ((u8*)dirTable  + (x)))
#define romFS_file(x)   ((romfs_file*)((u8*)fileTable + (x)))
#define romFS_none      ((u32)~0)
#define romFS_dir_mode  (S_IFDIR | S_IRUSR | S_IRGRP | S_IROTH)
#define romFS_file_mode (S_IFREG | S_IRUSR | S_IRGRP | S_IROTH)

static ssize_t _romfs_read(u64 offset, void* buffer, u32 size)
{
	u64 pos = (u64)romFS_offset + offset;
	u32 read = 0;
	Result rc = FSFILE_Read(romFS_file, &read, pos, buffer, size);
	if (R_FAILED(rc)) return -1;
	return read;
}

static bool _romfs_read_chk(u64 offset, void* buffer, u32 size)
{
	return _romfs_read(offset, buffer, size) == size;
}

//-----------------------------------------------------------------------------

static int       romfs_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
static int       romfs_close(struct _reent *r, int fd);
static ssize_t   romfs_read(struct _reent *r, int fd, char *ptr, size_t len);
static off_t     romfs_seek(struct _reent *r, int fd, off_t pos, int dir);
static int       romfs_fstat(struct _reent *r, int fd, struct stat *st);
static int       romfs_stat(struct _reent *r, const char *path, struct stat *st);
static int       romfs_chdir(struct _reent *r, const char *path);
static DIR_ITER* romfs_diropen(struct _reent *r, DIR_ITER *dirState, const char *path);
static int       romfs_dirreset(struct _reent *r, DIR_ITER *dirState);
static int       romfs_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
static int       romfs_dirclose(struct _reent *r, DIR_ITER *dirState);

typedef struct
{
	romfs_file *file;
	u64        offset, pos;
} romfs_fileobj;

typedef struct
{
	romfs_dir* dir;
	u32        state;
	u32        childDir;
	u32        childFile;
} romfs_diriter;

static devoptab_t romFS_devoptab =
{
	.name         = "romfs",
	.structSize   = sizeof(romfs_fileobj),
	.open_r       = romfs_open,
	.close_r      = romfs_close,
	.read_r       = romfs_read,
	.seek_r       = romfs_seek,
	.fstat_r      = romfs_fstat,
	.stat_r       = romfs_stat,
	.chdir_r      = romfs_chdir,
	.dirStateSize = sizeof(romfs_diriter),
	.diropen_r    = romfs_diropen,
	.dirreset_r   = romfs_dirreset,
	.dirnext_r    = romfs_dirnext,
	.dirclose_r   = romfs_dirclose,
	.deviceData   = NULL,
};

//-----------------------------------------------------------------------------

// File header
#define _3DSX_MAGIC 0x58534433 // '3DSX'
typedef struct
{
	u32 magic;
	u16 headerSize, relocHdrSize;
	u32 formatVer;
	u32 flags;

	// Sizes of the code, rodata and data segments +
	// size of the BSS section (uninitialized latter half of the data segment)
	u32 codeSegSize, rodataSegSize, dataSegSize, bssSize;
	// offset and size of smdh
	u32 smdhOffset, smdhSize;
	// offset to filesystem
	u32 fsOffset;
} _3DSX_Header;

static Result romfsInitCommon(void);
static void   romfsInitMtime(FS_ArchiveID archId, FS_Path archPath, FS_Path filePath);

__attribute__((weak)) const char* __romfs_path = NULL;

Result romfsInit(void)
{
	if (romFS_active) return 0;
	if (envIsHomebrew())
	{
		// RomFS appended to a 3DSX file
		const char* filename = __romfs_path;
		if (__system_argc > 0 && __system_argv[0])
			filename = __system_argv[0];
		if (!filename) return 1;

		if (strncmp(filename, "sdmc:/", 6) == 0)
			filename += 5;
		else if (strncmp(filename, "3dslink:/", 9) == 0)
		{
			strncpy(__component, "/3ds",     PATH_MAX);
			strncat(__component, filename+8, PATH_MAX);
			__component[PATH_MAX] = 0;
			filename = __component;
		} else
			return 2;

		ssize_t units = utf8_to_utf16(__utf16path, (const uint8_t*)filename, PATH_MAX);
		if (units < 0)         return 3;
		if (units >= PATH_MAX) return 4;
		__utf16path[units] = 0;

		FS_Path archPath = { PATH_EMPTY, 1, (u8*)"" };
		FS_Path filePath = { PATH_UTF16, (units+1)*2, (u8*)__utf16path };

		Result rc = FSUSER_OpenFileDirectly(&romFS_file, ARCHIVE_SDMC, archPath, filePath, FS_OPEN_READ, 0);
		if (R_FAILED(rc)) return rc;

		romfsInitMtime(ARCHIVE_SDMC, archPath, filePath);

		_3DSX_Header hdr;
		if (!_romfs_read_chk(0, &hdr, sizeof(hdr))) goto _fail0;
		if (hdr.magic != _3DSX_MAGIC) goto _fail0;
		if (hdr.headerSize < sizeof(hdr)) goto _fail0;
		romFS_offset = hdr.fsOffset;
		if (!romFS_offset) goto _fail0;
	} else
	{
		// Regular RomFS
		u8 zeros[0xC];
		memset(zeros, 0, sizeof(zeros));

		FS_Path archPath = { PATH_EMPTY, 1, (u8*)"" };
		FS_Path filePath = { PATH_BINARY, sizeof(zeros), zeros };

		Result rc = FSUSER_OpenFileDirectly(&romFS_file, ARCHIVE_ROMFS, archPath, filePath, FS_OPEN_READ, 0);
		if (R_FAILED(rc)) return rc;

		romfsInitMtime(ARCHIVE_ROMFS, archPath, filePath);
	}

	return romfsInitCommon();

_fail0:
	FSFILE_Close(romFS_file);
	return 10;
}

Result romfsInitFromFile(Handle file, u32 offset)
{
	if (romFS_active) return 0;
	romFS_file = file;
	romFS_offset = offset;
	return romfsInitCommon();
}

Result romfsInitCommon(void)
{
	if (_romfs_read(0, &romFS_header, sizeof(romFS_header)) != sizeof(romFS_header))
		goto _fail0;

	dirHashTable = (u32*)malloc(romFS_header.dirHashTableSize);
	if (!dirHashTable) goto _fail0;
	if (!_romfs_read_chk(romFS_header.dirHashTableOff, dirHashTable, romFS_header.dirHashTableSize)) goto _fail1;

	dirTable = malloc(romFS_header.dirTableSize);
	if (!dirTable) goto _fail1;
	if (!_romfs_read_chk(romFS_header.dirTableOff, dirTable, romFS_header.dirTableSize)) goto _fail2;

	fileHashTable = (u32*)malloc(romFS_header.fileHashTableSize);
	if (!fileHashTable) goto _fail2;
	if (!_romfs_read_chk(romFS_header.fileHashTableOff, fileHashTable, romFS_header.fileHashTableSize)) goto _fail3;

	fileTable = malloc(romFS_header.fileTableSize);
	if (!fileTable) goto _fail3;
	if (!_romfs_read_chk(romFS_header.fileTableOff, fileTable, romFS_header.fileTableSize)) goto _fail4;

	romFS_cwd = romFS_root;
	romFS_active = true;

	AddDevice(&romFS_devoptab);

	return 0;

_fail4:
	free(fileTable);
_fail3:
	free(fileHashTable);
_fail2:
	free(dirTable);
_fail1:
	free(dirHashTable);
_fail0:
	FSFILE_Close(romFS_file);
	return 10;
}

static void romfsInitMtime(FS_ArchiveID archId, FS_Path archPath, FS_Path filePath)
{
	u64 mtime;
	FS_Archive arch;
	Result rc;

	romFS_mtime = time(NULL);
	rc = FSUSER_OpenArchive(&arch, archId, archPath);
	if (R_FAILED(rc))
		return;

	rc = FSUSER_ControlArchive(arch, ARCHIVE_ACTION_GET_TIMESTAMP,
	                           (void*)filePath.data, filePath.size,
	                           &mtime, sizeof(mtime));
	FSUSER_CloseArchive(arch);
	if (R_FAILED(rc))
		return;

	/* convert from milliseconds to seconds */
	mtime /= 1000;
	/* convert from 2000-based timestamp to UNIX timestamp */
	mtime += 946684800;
	romFS_mtime = mtime;
}

Result romfsExit(void)
{
	if (!romFS_active) return 0;
	romFS_active = false;

	RemoveDevice("romfs:");
	FSFILE_Close(romFS_file);
	free(dirHashTable);
	free(fileHashTable);
	free(dirTable);
	free(fileTable);
	romFS_offset = 0;

	return 0;
}

//-----------------------------------------------------------------------------

static u32 calcHash(u32 parent, u16* name, u32 namelen, u32 total)
{
	u32 hash = parent ^ 123456789;
	u32 i;
	for (i = 0; i < namelen; i ++)
	{
		hash = (hash >> 5) | (hash << 27);
		hash ^= name[i];
	}
	return hash % total;
}

static romfs_dir* searchForDir(romfs_dir* parent, u16* name, u32 namelen)
{
	u32 parentOff = (u32)parent - (u32)dirTable;
	u32 hash = calcHash(parentOff, name, namelen, romFS_header.dirHashTableSize/4);
	romfs_dir* curDir = NULL;
	u32 curOff;
	for (curOff = dirHashTable[hash]; curOff != romFS_none; curOff = curDir->nextHash)
	{
		curDir = romFS_dir(curOff);
		if (curDir->parent != parentOff) continue;
		if (curDir->nameLen != namelen*2) continue;
		if (memcmp(curDir->name, name, namelen*2) != 0) continue;
		return curDir;
	}
	return NULL;
}

static romfs_file* searchForFile(romfs_dir* parent, u16* name, u32 namelen)
{
	u32 parentOff = (u32)parent - (u32)dirTable;
	u32 hash = calcHash(parentOff, name, namelen, romFS_header.fileHashTableSize/4);
	romfs_file* curFile = NULL;
	u32 curOff;
	for (curOff = fileHashTable[hash]; curOff != romFS_none; curOff = curFile->nextHash)
	{
		curFile = romFS_file(curOff);
		if (curFile->parent != parentOff) continue;
		if (curFile->nameLen != namelen*2) continue;
		if (memcmp(curFile->name, name, namelen*2) != 0) continue;
		return curFile;
	}
	return NULL;
}

static int navigateToDir(romfs_dir** ppDir, const char** pPath, bool isDir)
{
	ssize_t units;

	char* colonPos = strchr(*pPath, ':');
	if (colonPos) *pPath = colonPos+1;
	if (!**pPath)
		return EILSEQ;

	*ppDir = romFS_cwd;
	if (**pPath == '/')
	{
		*ppDir = romFS_root;
		(*pPath)++;
	}

	while (**pPath)
	{
		char* slashPos = strchr(*pPath, '/');
		char* component = __component;

		if (slashPos)
		{
			u32 len = slashPos - *pPath;
			if (!len)
				return EILSEQ;
			if (len > PATH_MAX)
				return ENAMETOOLONG;

			memcpy(component, *pPath, len);
			component[len] = 0;
			*pPath = slashPos+1;
		} else if (isDir)
		{
			component = (char*)*pPath;
			*pPath += strlen(component);
		} else
			return 0;

		if (component[0]=='.')
		{
			if (!component[1]) continue;
			if (component[1]=='.' && !component[2])
			{
				*ppDir = romFS_dir((*ppDir)->parent);
				continue;
			}
		}

		units = utf8_to_utf16(__utf16path, (const uint8_t*)component, PATH_MAX);
		if (units < 0)
			return EILSEQ;
		if (units >= PATH_MAX)
			return ENAMETOOLONG;

		*ppDir = searchForDir(*ppDir, __utf16path, units);
		if (!*ppDir)
			return EEXIST;
	}

	if (!isDir && !**pPath)
		return EILSEQ;

	return 0;
}

static ino_t dir_inode(romfs_dir *dir)
{
	return (uint32_t*)dir - (uint32_t*)dirTable;
}

static off_t dir_size(romfs_dir *dir)
{
	return sizeof(romfs_dir) + (dir->nameLen+3)/4;
}

static nlink_t dir_nlink(romfs_dir *dir)
{
	nlink_t count = 2; // one for self, one for parent
	u32     offset = dir->childDir;

	while(offset != romFS_none)
	{
		romfs_dir *tmp = romFS_dir(offset);
		++count;
		offset = tmp->sibling;
	}

	offset = dir->childFile;
	while(offset != romFS_none)
	{
		romfs_file *tmp = romFS_file(offset);
		++count;
		offset = tmp->sibling;
	}

	return count;
}

static ino_t file_inode(romfs_file *file)
{
	return ((uint32_t*)file - (uint32_t*)fileTable) + romFS_header.dirTableSize/4;
}

//-----------------------------------------------------------------------------

int romfs_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode)
{
	romfs_fileobj* fileobj = (romfs_fileobj*)fileStruct;

	if ((flags & O_ACCMODE) != O_RDONLY)
	{
		r->_errno = EROFS;
		return -1;
	}

	romfs_dir* curDir = NULL;
	r->_errno = navigateToDir(&curDir, &path, false);
	if (r->_errno != 0)
		return -1;

	ssize_t units = utf8_to_utf16(__utf16path, (const uint8_t*)path, PATH_MAX);
	if (units <= 0)
	{
		r->_errno = EILSEQ;
		return -1;
	}
	if (units >= PATH_MAX)
	{
		r->_errno = ENAMETOOLONG;
		return -1;
	}

	romfs_file* file = searchForFile(curDir, __utf16path, units);
	if (!file)
	{
		if(flags & O_CREAT)
			r->_errno = EROFS;
		else
			r->_errno = ENOENT;
		return -1;
	}
	else if((flags & O_CREAT) && (flags & O_EXCL))
	{
		r->_errno = EEXIST;
		return -1;
	}

	fileobj->file   = file;
	fileobj->offset = (u64)romFS_header.fileDataOff + file->dataOff;
	fileobj->pos    = 0;

	return 0;
}

int romfs_close(struct _reent *r, int fd)
{
	return 0;
}

ssize_t romfs_read(struct _reent *r, int fd, char *ptr, size_t len)
{
	romfs_fileobj* file = (romfs_fileobj*)fd;
	u64 endPos = file->pos + len;

	/* check if past end-of-file */
	if(file->pos >= file->file->dataSize)
		return 0;

	/* truncate the read to end-of-file */
	if(endPos > file->file->dataSize)
		endPos = file->file->dataSize;
	len = endPos - file->pos;

	ssize_t adv = _romfs_read(file->offset + file->pos, ptr, len);
	if(adv >= 0)
	{
		file->pos += adv;
		return adv;
	}

	r->_errno = EIO;
	return -1;
}

off_t romfs_seek(struct _reent *r, int fd, off_t pos, int dir)
{
	romfs_fileobj* file = (romfs_fileobj*)fd;
	off_t          start;
	switch (dir)
	{
		case SEEK_SET:
			start = 0;
			break;

		case SEEK_CUR:
			start = file->pos;
			break;

		case SEEK_END:
			start = file->file->dataSize;
			break;

		default:
			r->_errno = EINVAL;
			return -1;
	}

	/* don't allow negative position */
	if(pos < 0)
	{
 		if(start + pos < 0)
		{
			r->_errno = EINVAL;
			return -1;
		}
	}
	/* check for overflow */
	else if(INT64_MAX - pos < start)
	{
		r->_errno = EOVERFLOW;
		return -1;
	}

	file->pos = start + pos;
	return file->pos;
}

int romfs_fstat(struct _reent *r, int fd, struct stat *st)
{
	romfs_fileobj* file = (romfs_fileobj*)fd;
	memset(st, 0, sizeof(struct stat));
	st->st_ino   = file_inode(file->file);
	st->st_mode  = romFS_file_mode;
	st->st_nlink = 1;
	st->st_size  = (off_t)file->file->dataSize;
	st->st_blksize = 512;
	st->st_blocks  = (st->st_blksize + 511) / 512;
	st->st_atime = st->st_mtime = st->st_ctime = romFS_mtime;

	return 0;
}

int romfs_stat(struct _reent *r, const char *path, struct stat *st)
{
	romfs_dir* curDir = NULL;
	r->_errno = navigateToDir(&curDir, &path, false);
	if(r->_errno != 0)
		return -1;

	ssize_t units = utf8_to_utf16(__utf16path, (const uint8_t*)path, PATH_MAX);
	if (units <= 0)
	{
		r->_errno = EILSEQ;
		return -1;
	}
	if (units >= PATH_MAX)
	{
		r->_errno = ENAMETOOLONG;
		return -1;
	}

	romfs_dir* dir = searchForDir(curDir, __utf16path, units);
	if(dir)
	{
		memset(st, 0, sizeof(*st));
		st->st_ino     = dir_inode(dir);
		st->st_mode    = romFS_dir_mode;
		st->st_nlink   = dir_nlink(dir);
		st->st_size    = dir_size(dir);
		st->st_blksize = 512;
		st->st_blocks  = (st->st_blksize + 511) / 512;
		st->st_atime = st->st_mtime = st->st_ctime = romFS_mtime;

		return 0;
	}

	romfs_file* file = searchForFile(curDir, __utf16path, units);
	if(file)
	{
		memset(st, 0, sizeof(*st));
		st->st_ino   = file_inode(file);
		st->st_mode  = romFS_file_mode;
		st->st_nlink = 1;
		st->st_size  = file->dataSize;
		st->st_blksize = 512;
		st->st_blocks  = (st->st_blksize + 511) / 512;
		st->st_atime = st->st_mtime = st->st_ctime = romFS_mtime;

		return 0;
	}

	r->_errno = ENOENT;
	return 1;
}

int romfs_chdir(struct _reent *r, const char *path)
{
	romfs_dir* curDir = NULL;
	r->_errno = navigateToDir(&curDir, &path, true);
	if (r->_errno != 0)
		return -1;

	romFS_cwd = curDir;
	return 0;
}

DIR_ITER* romfs_diropen(struct _reent *r, DIR_ITER *dirState, const char *path)
{
	romfs_diriter* iter = (romfs_diriter*)(dirState->dirStruct);
	romfs_dir* curDir = NULL;

	r->_errno = navigateToDir(&curDir, &path, true);
	if(r->_errno != 0)
		return NULL;

	iter->dir       = curDir;
	iter->state     = 0;
	iter->childDir  = curDir->childDir;
	iter->childFile = curDir->childFile;

	return dirState;
}

int romfs_dirreset(struct _reent *r, DIR_ITER *dirState)
{
	romfs_diriter* iter = (romfs_diriter*)(dirState->dirStruct);

	iter->state     = 0;
	iter->childDir  = iter->dir->childDir;
	iter->childFile = iter->dir->childFile;

	return 0;
}

int romfs_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat)
{
	ssize_t        units;
	romfs_diriter* iter = (romfs_diriter*)(dirState->dirStruct);

	if(iter->state == 0)
	{
		/* '.' entry */
		memset(filestat, 0, sizeof(*filestat));
		filestat->st_ino  = dir_inode(iter->dir);
		filestat->st_mode = romFS_dir_mode;

		strcpy(filename, ".");
		iter->state = 1;
		return 0;
	}
	else if(iter->state == 1)
	{
		/* '..' entry */
		romfs_dir* dir = romFS_dir(iter->dir->parent);

		memset(filestat, 0, sizeof(*filestat));
		filestat->st_ino = dir_inode(dir);
		filestat->st_mode = romFS_dir_mode;

		strcpy(filename, "..");
		iter->state = 2;
		return 0;
	}

	if(iter->childDir != romFS_none)
	{
		romfs_dir* dir = romFS_dir(iter->childDir);
		iter->childDir = dir->sibling;

		memset(filestat, 0, sizeof(*filestat));
		filestat->st_ino = dir_inode(dir);
		filestat->st_mode = romFS_dir_mode;

		/* convert name from UTF-16 to UTF-8 */
		memset(filename, 0, NAME_MAX);
		memcpy(__utf16path, dir->name, dir->nameLen*sizeof(uint16_t));
		__utf16path[dir->nameLen/sizeof(uint16_t)] = 0;
		units = utf16_to_utf8((uint8_t*)filename, __utf16path, NAME_MAX);

		if(units < 0)
		{
			r->_errno = EILSEQ;
			return -1;
		}

		if(units >= NAME_MAX)
		{
			r->_errno = ENAMETOOLONG;
			return -1;
		}

		filename[units] = 0;

		return 0;
	}
	else if(iter->childFile != romFS_none)
	{
		romfs_file* file = romFS_file(iter->childFile);
		iter->childFile = file->sibling;

		memset(filestat, 0, sizeof(*filestat));
		filestat->st_ino = file_inode(file);
		filestat->st_mode = romFS_file_mode;

		/* convert name from UTF-16 to UTF-8 */
		memset(filename, 0, NAME_MAX);
		memcpy(__utf16path, file->name, file->nameLen*sizeof(uint16_t));
		__utf16path[file->nameLen/sizeof(uint16_t)] = 0;
		units = utf16_to_utf8((uint8_t*)filename, __utf16path, NAME_MAX);

		if(units < 0)
		{
			r->_errno = EILSEQ;
			return -1;
		}

		if(units >= NAME_MAX)
		{
			r->_errno = ENAMETOOLONG;
			return -1;
		}

		filename[units] = 0;

		return 0;
	}

	r->_errno = ENOENT;
	return -1;
}

int romfs_dirclose(struct _reent *r, DIR_ITER *dirState)
{
	return 0;
}
