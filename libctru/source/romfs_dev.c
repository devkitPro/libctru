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
#include <3ds/svc.h>
#include <3ds/romfs.h>
#include <3ds/services/fs.h>
#include <3ds/util/utf.h>

static bool romFS_active;
static Handle romFS_file;
static u32 romFS_offset;
static romfs_header romFS_header;
static romfs_dir* romFS_cwd;

static u32 *dirHashTable, *fileHashTable;
static void *dirTable, *fileTable;

extern u32 __service_ptr;
extern int __system_argc;
extern char** __system_argv;

static char __component[PATH_MAX+1];
static uint16_t __utf16path[PATH_MAX+1];

#define romFS_root    ((romfs_dir*)dirTable)
#define romFS_dir(x)  ((romfs_dir*) ((u8*)dirTable  + (x)))
#define romFS_file(x) ((romfs_file*)((u8*)fileTable + (x)))
#define romFS_none    ((u32)~0)

static ssize_t _romfs_read(u64 offset, void* buffer, u32 size)
{
	u64 pos = (u64)romFS_offset + offset;
	u32 read = 0;
	Result rc = FSFILE_Read(romFS_file, &read, pos, buffer, size);
	if (rc) return -1;
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
static int       romfs_stat(struct _reent *r, const char *file, struct stat *st);
static int       romfs_chdir(struct _reent *r, const char *name);
static DIR_ITER* romfs_diropen(struct _reent *r, DIR_ITER *dirState, const char *path);
static int       romfs_dirreset(struct _reent *r, DIR_ITER *dirState);
static int       romfs_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
static int       romfs_dirclose(struct _reent *r, DIR_ITER *dirState);

typedef struct
{
	u64 offset, size, pos;
} romfs_fileobj;

typedef struct
{
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

Result romfsInit(void)
{
	if (romFS_active) return 0;
	if (__service_ptr)
	{
		// RomFS appended to a 3DSX file
		if (__system_argc == 0 || !__system_argv[0]) return 1;
		const char* filename = __system_argv[0];
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

		size_t units = utf8_to_utf16(__utf16path, (const uint8_t*)filename, PATH_MAX+1);
		if (units == (size_t)-1) return 3;
		__utf16path[units] = 0;

		FS_archive arch = { ARCH_SDMC, { PATH_EMPTY, 1, (u8*)"" }, 0, 0 };
		FS_path path = { PATH_WCHAR, units+1, (u8*)__utf16path };

		Result rc = FSUSER_OpenFileDirectly(NULL, &romFS_file, arch, path, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
		if (rc) return rc;

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

		FS_archive arch = { ARCH_ROMFS, { PATH_EMPTY, 1, (u8*)"" }, 0, 0 };
		FS_path path = { PATH_BINARY, sizeof(zeros), zeros };

		Result rc = FSUSER_OpenFileDirectly(NULL, &romFS_file, arch, path, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
		if (rc) return rc;
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
	chdir("romfs:/");

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

Result romfsExit(void)
{
	if (!romFS_active) return 0;
	romFS_active = false;

	RemoveDevice("romfs");
	FSFILE_Close(romFS_file);
	free(dirHashTable);
	free(fileHashTable);
	free(dirTable);
	free(fileTable);

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
	size_t units;

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

		units = utf8_to_utf16(__utf16path, (const uint8_t*)component, PATH_MAX+1);
		if (units == (size_t)-1)
			return EILSEQ;

		*ppDir = searchForDir(*ppDir, __utf16path, units);
		if (!*ppDir)
			return EEXIST;
	}

	if (!isDir && !**pPath)
		return EILSEQ;

	return 0;
}

//-----------------------------------------------------------------------------

int romfs_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode)
{
	romfs_fileobj* fileobj = (romfs_fileobj*)fileStruct;

	if ((flags & O_ACCMODE) != O_RDONLY)
	{
		r->_errno = EINVAL;
		return -1;
	}

	romfs_dir* curDir = NULL;
	r->_errno = navigateToDir(&curDir, &path, false);
	if (r->_errno != 0)
		return -1;

	size_t units = utf8_to_utf16(__utf16path, (const uint8_t*)path, PATH_MAX+1);
	if (!units || units == (size_t)-1)
	{
		r->_errno = EILSEQ;
		return -1;
	}

	romfs_file* file = searchForFile(curDir, __utf16path, units);
	if (!file)
	{
		r->_errno = EEXIST;
		return -1;
	}

	fileobj->offset = (u64)romFS_header.fileDataOff + file->dataOff;
	fileobj->size   = file->dataSize;
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
	if (endPos > file->size)
		endPos = file->size;
	len = endPos - file->pos;

	ssize_t adv = _romfs_read(file->offset + file->pos, ptr, len);
	if (adv >= 0)
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
	switch (dir)
	{
		case SEEK_SET:
			file->pos = pos;
			break;
		case SEEK_CUR:
			file->pos += pos;
			break;
		case SEEK_END:
			file->pos = file->size + pos;
			break;
		default:
			r->_errno = EINVAL;
			return -1;
	}
	if (file->pos > file->size)
		file->pos = file->size;
	return file->pos;
}

int romfs_fstat(struct _reent *r, int fd, struct stat *st)
{
	romfs_fileobj* file = (romfs_fileobj*)fd;
	memset(st, 0, sizeof(struct stat));
	st->st_size  = (off_t)file->size;
	st->st_nlink = 1;
	st->st_mode  = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	return 0;
}

int romfs_stat(struct _reent *r, const char *file, struct stat *st)
{
	r->_errno = ENOTSUP;
	return 1;
}

int romfs_chdir(struct _reent *r, const char *name)
{
	romfs_dir* curDir = NULL;
	r->_errno = navigateToDir(&curDir, &name, true);
	if (r->_errno != 0)
		return -1;

	romFS_cwd = curDir;
	return 0;
}

DIR_ITER* romfs_diropen(struct _reent *r, DIR_ITER *dirState, const char *path)
{
	//romfs_diriter* dir = (romfs_diriter*)(dirState->dirStruct);
	r->_errno = ENOTSUP;
	return NULL;
}

int romfs_dirreset(struct _reent *r, DIR_ITER *dirState)
{
	//romfs_diriter* dir = (romfs_diriter*)(dirState->dirStruct);
	r->_errno = ENOTSUP;
	return 1;
}

int romfs_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat)
{
	//romfs_diriter* dir = (romfs_diriter*)(dirState->dirStruct);
	r->_errno = ENOTSUP;
	return 1;
}

int romfs_dirclose(struct _reent *r, DIR_ITER *dirState)
{
	return 0;
}
