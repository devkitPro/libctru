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

#include "path_buf.h"

typedef struct romfs_mount
{
	devoptab_t         device;
	char               name[32];
	Handle             fd;
	time_t             mtime;
	u32                offset;
	romfs_header       header;
	romfs_dir          *cwd;
	u32                *dirHashTable, *fileHashTable;
	void               *dirTable, *fileTable;
	struct romfs_mount *next;
} romfs_mount;

extern int __system_argc;
extern char** __system_argv;

#define romFS_root(m)   ((romfs_dir*)(m)->dirTable)
#define romFS_dir(m,x)  ((romfs_dir*) ((u8*)(m)->dirTable  + (x)))
#define romFS_file(m,x) ((romfs_file*)((u8*)(m)->fileTable + (x)))
#define romFS_none      ((u32)~0)
#define romFS_dir_mode  (S_IFDIR | S_IRUSR | S_IRGRP | S_IROTH)
#define romFS_file_mode (S_IFREG | S_IRUSR | S_IRGRP | S_IROTH)

static ssize_t _romfs_read(romfs_mount *mount, u64 offset, void* buffer, u32 size)
{
	u64 pos = (u64)mount->offset + offset;
	u32 read = 0;
	Result rc = FSFILE_Read(mount->fd, &read, pos, buffer, size);
	if (R_FAILED(rc)) return -1;
	return read;
}

static bool _romfs_read_chk(romfs_mount *mount, u64 offset, void* buffer, u32 size)
{
	return _romfs_read(mount, offset, buffer, size) == size;
}

//-----------------------------------------------------------------------------

static int       romfs_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
static int       romfs_close(struct _reent *r, void *fd);
static ssize_t   romfs_read(struct _reent *r, void *fd, char *ptr, size_t len);
static off_t     romfs_seek(struct _reent *r, void *fd, off_t pos, int dir);
static int       romfs_fstat(struct _reent *r, void *fd, struct stat *st);
static int       romfs_stat(struct _reent *r, const char *path, struct stat *st);
static int       romfs_chdir(struct _reent *r, const char *path);
static DIR_ITER* romfs_diropen(struct _reent *r, DIR_ITER *dirState, const char *path);
static int       romfs_dirreset(struct _reent *r, DIR_ITER *dirState);
static int       romfs_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
static int       romfs_dirclose(struct _reent *r, DIR_ITER *dirState);

typedef struct
{
	romfs_mount *mount;
	romfs_file  *file;
	u64         offset, pos;
} romfs_fileobj;

typedef struct
{
	romfs_mount *mount;
	romfs_dir* dir;
	u32        state;
	u32        childDir;
	u32        childFile;
} romfs_diriter;

static const devoptab_t romFS_devoptab =
{
	.structSize   = sizeof(romfs_fileobj),
	.open_r       = romfs_open,
	.close_r      = romfs_close,
	.read_r       = romfs_read,
	.seek_r       = romfs_seek,
	.fstat_r      = romfs_fstat,
	.stat_r       = romfs_stat,
	.lstat_r      = romfs_stat,
	.chdir_r      = romfs_chdir,
	.dirStateSize = sizeof(romfs_diriter),
	.diropen_r    = romfs_diropen,
	.dirreset_r   = romfs_dirreset,
	.dirnext_r    = romfs_dirnext,
	.dirclose_r   = romfs_dirclose,
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

__attribute__((weak)) const char* __romfs_path = NULL;

static romfs_mount *romfs_mount_list = NULL;

static void romfs_insert(romfs_mount *mount)
{
	mount->next      = romfs_mount_list;
	romfs_mount_list = mount;
}

static void romfs_remove(romfs_mount *mount)
{
	for(romfs_mount **it = &romfs_mount_list; *it; it = &(*it)->next)
	{
		if(*it == mount)
		{
			*it = mount->next;
			return;
		}
	}
}

static romfs_mount* romfs_alloc(void)
{
	romfs_mount *mount = (romfs_mount*)malloc(sizeof(romfs_mount));

	if (mount)
	{
		memset(mount, 0, sizeof(*mount));
		memcpy(&mount->device, &romFS_devoptab, sizeof(romFS_devoptab));
		mount->device.name = mount->name;
		mount->device.deviceData = mount;
		romfs_insert(mount);
	}

	return mount;
}

static void romfs_free(romfs_mount *mount)
{
	FSFILE_Close(mount->fd);
	romfs_remove(mount);
	free(mount->fileTable);
	free(mount->fileHashTable);
	free(mount->dirTable);
	free(mount->dirHashTable);
	free(mount);
}

Result romfsMountFromFile(Handle fd, u32 offset, const char *name)
{
	romfs_mount *mount = romfs_alloc();
	if (mount == NULL)
	{
		FSFILE_Close(fd);
		return MAKERESULT(RL_FATAL, RS_OUTOFRESOURCE, RM_ROMFS, RD_OUT_OF_MEMORY);
	}

	mount->fd     = fd;
	mount->offset = offset;
	mount->mtime  = time(NULL);
	strncpy(mount->name, name, sizeof(mount->name)-1);

	if (_romfs_read(mount, 0, &mount->header, sizeof(mount->header)) != sizeof(mount->header))
		goto fail_io;

	mount->dirHashTable = (u32*)malloc(mount->header.dirHashTableSize);
	if (!mount->dirHashTable)
		goto fail_oom;
	if (!_romfs_read_chk(mount, mount->header.dirHashTableOff, mount->dirHashTable, mount->header.dirHashTableSize))
		goto fail_io;

	mount->dirTable = malloc(mount->header.dirTableSize);
	if (!mount->dirTable)
		goto fail_oom;
	if (!_romfs_read_chk(mount, mount->header.dirTableOff, mount->dirTable, mount->header.dirTableSize))
		goto fail_io;

	mount->fileHashTable = (u32*)malloc(mount->header.fileHashTableSize);
	if (!mount->fileHashTable)
		goto fail_oom;
	if (!_romfs_read_chk(mount, mount->header.fileHashTableOff, mount->fileHashTable, mount->header.fileHashTableSize))
		goto fail_io;

	mount->fileTable = malloc(mount->header.fileTableSize);
	if (!mount->fileTable)
		goto fail_oom;
	if (!_romfs_read_chk(mount, mount->header.fileTableOff, mount->fileTable, mount->header.fileTableSize))
		goto fail_io;

	mount->cwd = romFS_root(mount);

	if (AddDevice(&mount->device) < 0)
		goto fail_oom;

	return 0;

fail_oom:
	romfs_free(mount);
	return MAKERESULT(RL_FATAL, RS_OUTOFRESOURCE, RM_ROMFS, RD_OUT_OF_MEMORY);

fail_io:
	romfs_free(mount);
	return MAKERESULT(RL_FATAL, RS_INVALIDSTATE, RM_ROMFS, RD_NOT_FOUND);
}

Result romfsMountSelf(const char* name)
{
	// If we are not 3DSX, we need to mount this process' real RomFS
	if (!envIsHomebrew())
		return romfsMountFromCurrentProcess(name);

	// Otherwise, we use the embedded RomFS inside the 3DSX file
    // Retrieve the filename of our 3DSX file
	const char* filename = __romfs_path;
	if (__system_argc > 0 && __system_argv[0])
		filename = __system_argv[0];
	if (!filename)
		return MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_ROMFS, RD_NO_DATA);

	// Adjust the path name
	if (strncmp(filename, "sdmc:/", 6) == 0)
		filename += 5;
	else if (strncmp(filename, "3dslink:/", 9) == 0)
	{
		strncpy(__ctru_dev_path_buf, "/3ds",     PATH_MAX);
		strncat(__ctru_dev_path_buf, filename+8, PATH_MAX);
		__ctru_dev_path_buf[PATH_MAX] = 0;
		filename = __ctru_dev_path_buf;
	}
	else
		return MAKERESULT(RL_USAGE, RS_NOTSUPPORTED, RM_ROMFS, RD_NOT_IMPLEMENTED);

	// Convert the path to UTF-16
	ssize_t units = utf8_to_utf16(__ctru_dev_utf16_buf, (const uint8_t*)filename, PATH_MAX);
	if (units < 0 || units > PATH_MAX)
		return MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_ROMFS, RD_OUT_OF_RANGE);
	__ctru_dev_utf16_buf[units] = 0;

	// Open the file directly
	Handle fd = 0;
	FS_Path archPath = { PATH_EMPTY, 1, "" };
	FS_Path filePath = { PATH_UTF16, (units+1)*2, __ctru_dev_utf16_buf };
	Result rc = FSUSER_OpenFileDirectly(&fd, ARCHIVE_SDMC, archPath, filePath, FS_OPEN_READ, 0);
	if (R_FAILED(rc))
		return rc;

	// Read and parse the header
	_3DSX_Header hdr;
	u32 bytesRead = 0;
	rc = FSFILE_Read(fd, &bytesRead, 0, &hdr, sizeof(hdr));
	if (R_FAILED(rc))
	{
		FSFILE_Close(fd);
		return rc;
	}

	// Validate the 3DSX header
	if (bytesRead != sizeof(hdr) || hdr.magic != _3DSX_MAGIC || hdr.headerSize < sizeof(hdr))
	{
		FSFILE_Close(fd);
		return MAKERESULT(RL_FATAL, RS_NOTFOUND, RM_ROMFS, RD_NOT_FOUND);
	}

	// Mount the file
	return romfsMountFromFile(fd, hdr.fsOffset, name);
}

Result romfsMountFromCurrentProcess(const char *name)
{
	// Set up FS_Path structures
	u8 zeros[0xC] = {0};
	FS_Path archPath = { PATH_EMPTY, 1, "" };
	FS_Path filePath = { PATH_BINARY, sizeof(zeros), zeros };

	// Open the RomFS file and mount it
	Handle fd = 0;
	Result rc = FSUSER_OpenFileDirectly(&fd, ARCHIVE_ROMFS, archPath, filePath, FS_OPEN_READ, 0);
	if (R_SUCCEEDED(rc))
		rc = romfsMountFromFile(fd, 0, name);

	return rc;
}

Result romfsMountFromTitle(u64 tid, FS_MediaType mediatype, const char* name)
{
	// Set up FS_Path structures
	u32 archPathData[4] = { (u32)tid, (u32)(tid>>32), (u8)mediatype, 0 };
	u32 filePathData[5] = { 0 };
	FS_Path archPath = { PATH_BINARY, sizeof(archPathData), archPathData };
	FS_Path filePath = { PATH_BINARY, sizeof(filePathData), filePathData };

	// Open the RomFS file and mount it
	Handle fd = 0;
	Result rc = FSUSER_OpenFileDirectly(&fd, ARCHIVE_SAVEDATA_AND_CONTENT, archPath, filePath, FS_OPEN_READ, 0);
	if (R_SUCCEEDED(rc))
		rc = romfsMountFromFile(fd, 0, name);

	return rc;
}

Result romfsUnmount(const char* name)
{
	// Find the mount
	romfs_mount* mount = romfs_mount_list;
	while (mount)
	{
		if (strncmp(mount->name, name, sizeof(mount->name)) == 0)
			break;
		mount = mount->next;
	}

	if (mount == NULL)
		return MAKERESULT(RL_STATUS, RS_NOTFOUND, RM_ROMFS, RD_NOT_FOUND);

	// Remove device
	char tmpname[34];
	unsigned len = strlen(name);
	memcpy(tmpname, mount->name, len);
	tmpname[len] = ':';
	tmpname[len+1] = 0;
	RemoveDevice(tmpname);

	// Free the mount
	romfs_free(mount);

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

static romfs_dir* searchForDir(romfs_mount *mount, romfs_dir* parent, u16* name, u32 namelen)
{
	u32 parentOff = (u32)parent - (u32)mount->dirTable;
	u32 hash = calcHash(parentOff, name, namelen, mount->header.dirHashTableSize/4);
	romfs_dir* curDir = NULL;
	u32 curOff;
	for (curOff = mount->dirHashTable[hash]; curOff != romFS_none; curOff = curDir->nextHash)
	{
		curDir = romFS_dir(mount, curOff);
		if (curDir->parent != parentOff) continue;
		if (curDir->nameLen != namelen*2) continue;
		if (memcmp(curDir->name, name, namelen*2) != 0) continue;
		return curDir;
	}
	return NULL;
}

static romfs_file* searchForFile(romfs_mount *mount, romfs_dir* parent, u16* name, u32 namelen)
{
	u32 parentOff = (u32)parent - (u32)mount->dirTable;
	u32 hash = calcHash(parentOff, name, namelen, mount->header.fileHashTableSize/4);
	romfs_file* curFile = NULL;
	u32 curOff;
	for (curOff = mount->fileHashTable[hash]; curOff != romFS_none; curOff = curFile->nextHash)
	{
		curFile = romFS_file(mount, curOff);
		if (curFile->parent != parentOff) continue;
		if (curFile->nameLen != namelen*2) continue;
		if (memcmp(curFile->name, name, namelen*2) != 0) continue;
		return curFile;
	}
	return NULL;
}

static int navigateToDir(romfs_mount *mount, romfs_dir** ppDir, const char** pPath, bool isDir)
{
	ssize_t units;

	char* colonPos = strchr(*pPath, ':');
	if (colonPos) *pPath = colonPos+1;
	if (!**pPath)
		return EILSEQ;

	*ppDir = mount->cwd;
	if (**pPath == '/')
	{
		*ppDir = romFS_root(mount);
		(*pPath)++;
	}

	while (**pPath)
	{
		char* slashPos = strchr(*pPath, '/');
		char* component = __ctru_dev_path_buf;

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
				*ppDir = romFS_dir(mount, (*ppDir)->parent);
				continue;
			}
		}

		units = utf8_to_utf16(__ctru_dev_utf16_buf, (const uint8_t*)component, PATH_MAX);
		if (units < 0)
			return EILSEQ;
		if (units > PATH_MAX)
			return ENAMETOOLONG;

		*ppDir = searchForDir(mount, *ppDir, __ctru_dev_utf16_buf, units);
		if (!*ppDir)
			return ENOENT;
	}

	return 0;
}

static ino_t dir_inode(romfs_mount *mount, romfs_dir *dir)
{
	return (uint32_t*)dir - (uint32_t*)mount->dirTable;
}

static off_t dir_size(romfs_dir *dir)
{
	return sizeof(romfs_dir) + (dir->nameLen+3)/4;
}

static nlink_t dir_nlink(romfs_mount *mount, romfs_dir *dir)
{
	nlink_t count = 2; // one for self, one for parent
	u32     offset = dir->childDir;

	while(offset != romFS_none)
	{
		romfs_dir *tmp = romFS_dir(mount, offset);
		++count;
		offset = tmp->sibling;
	}

	offset = dir->childFile;
	while(offset != romFS_none)
	{
		romfs_file *tmp = romFS_file(mount, offset);
		++count;
		offset = tmp->sibling;
	}

	return count;
}

static ino_t file_inode(romfs_mount *mount, romfs_file *file)
{
	return ((uint32_t*)file - (uint32_t*)mount->fileTable) + mount->header.dirTableSize/4;
}

static void fill_dir(struct stat *st, romfs_mount *mount, romfs_dir *dir)
{
	memset(st, 0, sizeof(*st));
	st->st_ino     = dir_inode(mount, dir);
	st->st_mode    = romFS_dir_mode;
	st->st_nlink   = dir_nlink(mount, dir);
	st->st_size    = dir_size(dir);
	st->st_blksize = 512;
	st->st_blocks  = (st->st_blksize + 511) / 512;
	st->st_atime   = st->st_mtime = st->st_ctime = mount->mtime;
}

static void fill_file(struct stat *st, romfs_mount *mount, romfs_file *file)
{
	memset(st, 0, sizeof(*st));
	st->st_ino     = file_inode(mount, file);
	st->st_mode    = romFS_file_mode;
	st->st_nlink   = 1;
	st->st_size    = (off_t)file->dataSize;
	st->st_blksize = 512;
	st->st_blocks  = (st->st_blksize + 511) / 512;
	st->st_atime   = st->st_mtime = st->st_ctime = mount->mtime;
}

//-----------------------------------------------------------------------------

int romfs_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode)
{
	romfs_fileobj* fileobj = (romfs_fileobj*)fileStruct;

	fileobj->mount = (romfs_mount*)r->deviceData;

	if ((flags & O_ACCMODE) != O_RDONLY)
	{
		r->_errno = EROFS;
		return -1;
	}

	romfs_dir* curDir = NULL;
	r->_errno = navigateToDir(fileobj->mount, &curDir, &path, false);
	if (r->_errno != 0)
		return -1;

	ssize_t units = utf8_to_utf16(__ctru_dev_utf16_buf, (const uint8_t*)path, PATH_MAX);
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

	romfs_file* file = searchForFile(fileobj->mount, curDir, __ctru_dev_utf16_buf, units);
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
	fileobj->offset = (u64)fileobj->mount->header.fileDataOff + file->dataOff;
	fileobj->pos    = 0;

	return 0;
}

int romfs_close(struct _reent *r, void *fd)
{
	return 0;
}

ssize_t romfs_read(struct _reent *r, void *fd, char *ptr, size_t len)
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

	ssize_t adv = _romfs_read(file->mount, file->offset + file->pos, ptr, len);
	if(adv >= 0)
	{
		file->pos += adv;
		return adv;
	}

	r->_errno = EIO;
	return -1;
}

off_t romfs_seek(struct _reent *r, void *fd, off_t pos, int dir)
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

int romfs_fstat(struct _reent *r, void *fd, struct stat *st)
{
	romfs_fileobj* fileobj = (romfs_fileobj*)fd;
	fill_file(st, fileobj->mount, fileobj->file);
	return 0;
}

int romfs_stat(struct _reent *r, const char *path, struct stat *st)
{
	romfs_mount* mount = (romfs_mount*)r->deviceData;
	romfs_dir* curDir = NULL;
	r->_errno = navigateToDir(mount, &curDir, &path, false);
	if(r->_errno != 0)
		return -1;

	if (!*path)
	{
		fill_dir(st, mount, curDir);
		return 0;
	}

	ssize_t units = utf8_to_utf16(__ctru_dev_utf16_buf, (const uint8_t*)path, PATH_MAX);
	if (units <= 0)
	{
		r->_errno = EILSEQ;
		return -1;
	}
	if (units > PATH_MAX)
	{
		r->_errno = ENAMETOOLONG;
		return -1;
	}

	romfs_dir* dir = searchForDir(mount, curDir, __ctru_dev_utf16_buf, units);
	if(dir)
	{
		fill_dir(st, mount, dir);
		return 0;
	}

	romfs_file* file = searchForFile(mount, curDir, __ctru_dev_utf16_buf, units);
	if(file)
	{
		fill_file(st, mount, file);
		return 0;
	}

	r->_errno = ENOENT;
	return -1;
}

int romfs_chdir(struct _reent *r, const char *path)
{
	romfs_mount* mount = (romfs_mount*)r->deviceData;
	romfs_dir* curDir = NULL;
	r->_errno = navigateToDir(mount, &curDir, &path, true);
	if (r->_errno != 0)
		return -1;

	mount->cwd = curDir;
	return 0;
}

DIR_ITER* romfs_diropen(struct _reent *r, DIR_ITER *dirState, const char *path)
{
	romfs_diriter* iter = (romfs_diriter*)(dirState->dirStruct);
	romfs_dir* curDir = NULL;
	iter->mount = (romfs_mount*)r->deviceData;

	r->_errno = navigateToDir(iter->mount, &curDir, &path, true);
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
		filestat->st_ino  = dir_inode(iter->mount, iter->dir);
		filestat->st_mode = romFS_dir_mode;

		strcpy(filename, ".");
		iter->state = 1;
		return 0;
	}
	else if(iter->state == 1)
	{
		/* '..' entry */
		romfs_dir* dir = romFS_dir(iter->mount, iter->dir->parent);

		memset(filestat, 0, sizeof(*filestat));
		filestat->st_ino = dir_inode(iter->mount, dir);
		filestat->st_mode = romFS_dir_mode;

		strcpy(filename, "..");
		iter->state = 2;
		return 0;
	}

	if(iter->childDir != romFS_none)
	{
		romfs_dir* dir = romFS_dir(iter->mount, iter->childDir);
		iter->childDir = dir->sibling;

		memset(filestat, 0, sizeof(*filestat));
		filestat->st_ino = dir_inode(iter->mount, dir);
		filestat->st_mode = romFS_dir_mode;

		/* convert name from UTF-16 to UTF-8 */
		memset(filename, 0, NAME_MAX);
		memcpy(__ctru_dev_utf16_buf, dir->name, dir->nameLen*sizeof(uint16_t));
		__ctru_dev_utf16_buf[dir->nameLen/sizeof(uint16_t)] = 0;
		units = utf16_to_utf8((uint8_t*)filename, __ctru_dev_utf16_buf, NAME_MAX);

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
		romfs_file* file = romFS_file(iter->mount, iter->childFile);
		iter->childFile = file->sibling;

		memset(filestat, 0, sizeof(*filestat));
		filestat->st_ino = file_inode(iter->mount, file);
		filestat->st_mode = romFS_file_mode;

		/* convert name from UTF-16 to UTF-8 */
		memset(filename, 0, NAME_MAX);
		memcpy(__ctru_dev_utf16_buf, file->name, file->nameLen*sizeof(uint16_t));
		__ctru_dev_utf16_buf[file->nameLen/sizeof(uint16_t)] = 0;
		units = utf16_to_utf8((uint8_t*)filename, __ctru_dev_utf16_buf, NAME_MAX);

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
