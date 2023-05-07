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
#include <3ds/archive.h>
#include <3ds/services/fs.h>
#include <3ds/util/utf.h>

#include "path_buf.h"

/*! @internal
 *
 *  @file archive_dev.c
 *
 *  Archive Device
 */

static int archive_translate_error(Result error);

static int       archive_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
static int       archive_close(struct _reent *r, void *fd);
static ssize_t   archive_write(struct _reent *r, void *fd, const char *ptr, size_t len);
static ssize_t   archive_read(struct _reent *r, void *fd, char *ptr, size_t len);
static off_t     archive_seek(struct _reent *r, void *fd, off_t pos, int dir);
static int       archive_fstat(struct _reent *r, void *fd, struct stat *st);
static int       archive_stat(struct _reent *r, const char *file, struct stat *st);
static int       archive_link(struct _reent *r, const char *existing, const char  *newLink);
static int       archive_unlink(struct _reent *r, const char *name);
static int       archive_chdir(struct _reent *r, const char *name);
static int       archive_rename(struct _reent *r, const char *oldName, const char *newName);
static int       archive_mkdir(struct _reent *r, const char *path, int mode);
static DIR_ITER* archive_diropen(struct _reent *r, DIR_ITER *dirState, const char *path);
static int       archive_dirreset(struct _reent *r, DIR_ITER *dirState);
static int       archive_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
static int       archive_dirclose(struct _reent *r, DIR_ITER *dirState);
static int       archive_statvfs(struct _reent *r, const char *path, struct statvfs *buf);
static int       archive_ftruncate(struct _reent *r, void *fd, off_t len);
static int       archive_fsync(struct _reent *r, void *fd);
static int       archive_chmod(struct _reent *r, const char *path, mode_t mode);
static int       archive_fchmod(struct _reent *r, void *fd, mode_t mode);
static int       archive_rmdir(struct _reent *r, const char *name);

/// Used for only the SD archive, about which information can actually be found
static int       sdmc_statvfs(struct _reent *r, const char *path, struct statvfs *buf);

/*! @cond INTERNAL */

/*! Open file struct */
typedef struct
{
  Handle fd;     /*! CTRU handle */
  int    flags;  /*! Flags used in open(2) */
  u64    offset; /*! Current file offset */
} archive_file_t;

/*! archive devoptab */
static devoptab_t
archive_devoptab =
{
  .name         = "archive",
  .structSize   = sizeof(archive_file_t),
  .open_r       = archive_open,
  .close_r      = archive_close,
  .write_r      = archive_write,
  .read_r       = archive_read,
  .seek_r       = archive_seek,
  .fstat_r      = archive_fstat,
  .stat_r       = archive_stat,
  .lstat_r      = archive_stat,
  .link_r       = archive_link,
  .unlink_r     = archive_unlink,
  .chdir_r      = archive_chdir,
  .rename_r     = archive_rename,
  .mkdir_r      = archive_mkdir,
  .dirStateSize = sizeof(archive_dir_t),
  .diropen_r    = archive_diropen,
  .dirreset_r   = archive_dirreset,
  .dirnext_r    = archive_dirnext,
  .dirclose_r   = archive_dirclose,
  .statvfs_r    = archive_statvfs,
  .ftruncate_r  = archive_ftruncate,
  .fsync_r      = archive_fsync,
  .deviceData   = 0,
  .chmod_r      = archive_chmod,
  .fchmod_r     = archive_fchmod,
  .rmdir_r      = archive_rmdir,
};

typedef struct
{
    bool setup;
    bool is_extdata;
    s32 id;
    devoptab_t device;
    FS_Archive archive;
    char* cwd;
    char name[32];
} archive_fsdevice;

static bool archive_initialized = false;
static s32 archive_device_cwd;
static archive_fsdevice archive_devices[8];

/*! @endcond */

static archive_fsdevice *archiveFindDevice(const char *name)
{
  u32 i;
  u32 total = sizeof(archive_devices) / sizeof(archive_fsdevice);
  archive_fsdevice *device = NULL;

  if(!archive_initialized)
    return NULL;

  for(i=0; i<total; i++)
  {
    device = &archive_devices[i];

    if(name==NULL) //Find an unused device entry.
    {
      if(!device->setup)
        return device;
    }
    else if(device->setup) //Find the device with the input name.
    {
      size_t devnamelen = strlen(device->name);
      if(strncmp(device->name, name, devnamelen)==0 && (name[devnamelen]=='\0' || name[devnamelen]==':'))
        return device;
    }
  }

  return NULL;
}

static const char*
archive_fixpath(struct _reent    *r,
                const char       *path,
                archive_fsdevice **device)
{
  ssize_t       units;
  uint32_t      code;
  const uint8_t *p = (const uint8_t*)path;
  const char *device_path = path;

  // Move the path pointer to the start of the actual path
  do
  {
    units = decode_utf8(&code, p);
    if(units < 0)
    {
      r->_errno = EILSEQ;
      return NULL;
    }

    p += units;
  } while(code != ':' && code != 0);

  // We found a colon; p points to the actual path
  if(code == ':')
    path = (const char*)p;

  // Make sure there are no more colons and that the
  // remainder of the filename is valid UTF-8
  p = (const uint8_t*)path;
  do
  {
    units = decode_utf8(&code, p);
    if(units < 0)
    {
      r->_errno = EILSEQ;
      return NULL;
    }

    if(code == ':')
    {
      r->_errno = EINVAL;
      return NULL;
    }

    p += units;
  } while(code != 0);

  archive_fsdevice *dev = NULL;
  if(device != NULL && *device != NULL)
    dev = *device;
  else if(path != device_path)
    dev = archiveFindDevice(device_path);
  else if(archive_device_cwd != -1)
    dev = &archive_devices[archive_device_cwd];
  if(dev == NULL)
  {
    r->_errno = ENODEV;
    return NULL;
  }

  if(path[0] == '/')
    strncpy(__ctru_dev_path_buf, path, PATH_MAX);
  else
  {
    size_t cwdlen = strlen(dev->cwd);
    strncpy(__ctru_dev_path_buf, dev->cwd, PATH_MAX);
    __ctru_dev_path_buf[PATH_MAX] = '\0';
    strncat(__ctru_dev_path_buf, "/", PATH_MAX - cwdlen);
    strncat(__ctru_dev_path_buf, path, PATH_MAX - cwdlen - 1);
  }

  if(__ctru_dev_path_buf[PATH_MAX] != 0)
  {
    __ctru_dev_path_buf[PATH_MAX] = 0;
    r->_errno = ENAMETOOLONG;
    return NULL;
  }

  if(device)
    *device = dev;

  return __ctru_dev_path_buf;
}

static const FS_Path
archive_utf16path(struct _reent    *r,
                  const char       *path,
                  archive_fsdevice **device)
{
  ssize_t units;
  FS_Path fspath;

  fspath.data = NULL;

  if(archive_fixpath(r, path, device) == NULL)
    return fspath;

  units = utf8_to_utf16(__ctru_dev_utf16_buf, (const uint8_t*)__ctru_dev_path_buf, PATH_MAX);
  if(units < 0)
  {
    r->_errno = EILSEQ;
    return fspath;
  }
  if(units > PATH_MAX)
  {
    r->_errno = ENAMETOOLONG;
    return fspath;
  }

  __ctru_dev_utf16_buf[units] = 0;

  fspath.type = PATH_UTF16;
  fspath.size = (units+1)*sizeof(uint16_t);
  fspath.data = __ctru_dev_utf16_buf;

  return fspath;
}

extern int __system_argc;
extern char** __system_argv;

static void _archiveInit(void)
{
  u32 total = sizeof(archive_devices) / sizeof(archive_fsdevice);
  if (!archive_initialized)
  {
    memset(archive_devices, 0, sizeof(archive_devices));

    for (u32 i=0; i<total; i++)
    {
      memcpy(&archive_devices[i].device, &archive_devoptab, sizeof(archive_devoptab));
      archive_devices[i].device.name = archive_devices[i].name;
      archive_devices[i].device.deviceData = &archive_devices[i];
      archive_devices[i].id = i;
    }

    archive_device_cwd = -1;
    archive_initialized = true;
  }
}

static int _archiveMountDevice(FS_Archive       archive,
                               const char       *deviceName,
                               archive_fsdevice **out_device)
{
  archive_fsdevice *device = NULL;

  if (archiveFindDevice(deviceName)) //Device is already mounted with the same name
    goto _fail;

  _archiveInit(); //Ensure driver is initialized

  device = archiveFindDevice(NULL);
  if(device==NULL)
    goto _fail;

  device->archive = archive;
  memset(device->name, 0, sizeof(device->name));
  strncpy(device->name, deviceName, sizeof(device->name)-1);

  int dev = AddDevice(&device->device);
  if(dev==-1)
    goto _fail;

  device->setup = 1;
  device->cwd = malloc(PATH_MAX+1);
  device->cwd[0] = '/';
  device->cwd[1] = '\0';

  if (archive_device_cwd==-1)
    archive_device_cwd = device->id;

  const devoptab_t *default_dev = GetDeviceOpTab("");
  if(default_dev==NULL || strcmp(default_dev->name, "stdnull")==0)
    setDefaultDevice(dev);

  if(out_device != NULL)
    *out_device=device;

  return dev;

_fail:
  FSUSER_CloseArchive(archive);
  return -1;
}

Result archiveMount(FS_ArchiveID archiveID,
                    FS_Path      archivePath,
                    const char   *deviceName)
{
  FS_Archive ar;
  Result rc = 0;
  rc = FSUSER_OpenArchive(&ar, archiveID, archivePath);
  if (R_SUCCEEDED(rc))
  {
    archive_fsdevice* device = NULL;
    rc = _archiveMountDevice(ar, deviceName, &device);
    if (R_SUCCEEDED(rc))
    {
      /* Special handling for ExtData is necessary */
      if (archiveID == ARCHIVE_EXTDATA || archiveID == ARCHIVE_SHARED_EXTDATA || archiveID == ARCHIVE_BOSS_EXTDATA || archiveID == ARCHIVE_EXTDATA_AND_BOSS_EXTDATA)
      {
        device->is_extdata = true;
      }
    }
  }
  return rc;
}

/*! Initialize SDMC device */
Result archiveMountSdmc(void)
{
  ssize_t  units;
  uint32_t code;
  char     *p;
  FS_Path sdmcPath = { PATH_EMPTY, 1, "" };
  FS_Archive sdmcArchive;
  Result   rc = 0;

  _archiveInit();

  rc = FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, sdmcPath);
  if(R_SUCCEEDED(rc))
  {
    fsExemptFromSession(sdmcArchive);
    archive_fsdevice* device;
    rc = _archiveMountDevice(sdmcArchive, "sdmc", &device);

    if (rc != -1)
    {
      if (device)
      {
        device->device.statvfs_r = sdmc_statvfs; // set SDMC's statvfs to the device
      }
      if(__system_argc != 0 && __system_argv[0] != NULL)
      {
        if(FindDevice(__system_argv[0]) == rc)
        {
          strncpy(__ctru_dev_path_buf,__system_argv[0],PATH_MAX);
          if(__ctru_dev_path_buf[PATH_MAX] != 0)
          {
            __ctru_dev_path_buf[PATH_MAX] = 0;
          }
          else
          {
            char *last_slash = NULL;
            p = __ctru_dev_path_buf;
            do
            {
              units = decode_utf8(&code, (const uint8_t*)p);
              if(units < 0)
              {
                last_slash = NULL;
                break;
              }

              if(code == '/')
                last_slash = p;

              p += units;
            } while(code != 0);

            if(last_slash != NULL)
            {
              last_slash[0] = 0;
              chdir(__ctru_dev_path_buf);
            }
          }
        }
      }
    }
  }

  return rc;
}

Result archiveCommitSaveData(const char *deviceName)
{
  archive_fsdevice* device = archiveFindDevice(deviceName);
  if(device!=NULL)
  {
    return FSUSER_ControlArchive(device->archive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
  }

  return -1;
}

Result _archiveUnmountDeviceStruct(archive_fsdevice *device)
{
  char name[34];
  if(!device->setup)
    return 0;

  memset(name, 0, sizeof(name));
  strncpy(name, device->name, sizeof(name)-2);
  strncat(name, ":", sizeof(name)-strlen(name)-1);

  RemoveDevice(name);
  free(device->cwd);
  FSUSER_CloseArchive(device->archive);
  fsUnexemptFromSession(device->archive);

  if(device->id == archive_device_cwd)
    archive_device_cwd = -1;

  device->setup = 0;
  memset(device->name, 0, sizeof(device->name));

  return 0;
}

Result archiveUnmount(const char *deviceName)
{
  archive_fsdevice *device = archiveFindDevice(deviceName);

  if(device==NULL)
    return -1;

  return _archiveUnmountDeviceStruct(device);
}

Result archiveUnmountAll(void)
{
  u32 total = sizeof(archive_devices) / sizeof(archive_fsdevice);

  if(!archive_initialized) return 0;

  for(u32 i = 0; i < total; i++)
  {
    _archiveUnmountDeviceStruct(&archive_devices[i]);
  }

  archive_initialized = false;

  return 0;
}

/*! Open a file
 *
 *  @param[in,out] r          newlib reentrancy struct
 *  @param[out]    fileStruct Pointer to file struct to fill in
 *  @param[in]     path       Path to open
 *  @param[in]     flags      Open flags from open(2)
 *  @param[in]     mode       Permissions to set on create
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_open(struct _reent *r,
             void          *fileStruct,
             const char    *path,
             int           flags,
             int           mode)
{
  Handle        fd;
  Result        rc;
  u32           archive_flags = 0;
  u32           attributes = 0;
  FS_Path       fs_path;
  archive_fsdevice *device = r->deviceData;

  fs_path = archive_utf16path(r, path, &device);
  if(fs_path.data == NULL)
    return -1;

  /* get pointer to our data */
  archive_file_t *file = (archive_file_t*)fileStruct;

  /* check access mode */
  switch(flags & O_ACCMODE)
  {
    /* read-only: do not allow O_APPEND */
    case O_RDONLY:
      archive_flags |= FS_OPEN_READ;
      if(flags & O_APPEND)
      {
        r->_errno = EINVAL;
        return -1;
      }
      break;

    /* write-only */
    case O_WRONLY:
      archive_flags |= FS_OPEN_WRITE;
      break;

    /* read and write */
    case O_RDWR:
      archive_flags |= (FS_OPEN_READ | FS_OPEN_WRITE);
      break;

    /* an invalid option was supplied */
    default:
      r->_errno = EINVAL;
      return -1;
  }

  /* ExtData is weird and there's no good way to support writing to it through standard interfaces */
  if (device->is_extdata && ((flags & O_ACCMODE) != O_RDONLY || (flags & O_CREAT)))
  {
    r->_errno = ENOTSUP;
    return -1;
  }

  /* create file */
  if((flags & O_CREAT))
    archive_flags |= FS_OPEN_CREATE;

  /* Test O_EXCL. */
  if((flags & O_CREAT) && (flags & O_EXCL))
  {
    rc = FSUSER_CreateFile(device->archive, fs_path, attributes, 0);
    if(R_FAILED(rc))
    {
      r->_errno = archive_translate_error(rc);
      return -1;
    }
  }

  /* set attributes */
  /*if(!(mode & S_IWUSR))
    attributes |= FS_ATTRIBUTE_READONLY;*/

  /* open the file */
  rc = FSUSER_OpenFile(&fd, device->archive, fs_path,
                       archive_flags, attributes);
  if(R_SUCCEEDED(rc))
  {
    if((flags & O_ACCMODE) != O_RDONLY && (flags & O_TRUNC))
    {
      rc = FSFILE_SetSize(fd, 0);
      if(R_FAILED(rc))
      {
        FSFILE_Close(fd);
        r->_errno = archive_translate_error(rc);
        return -1;
      }
    }

    file->fd     = fd;
    file->flags  = (flags & (O_ACCMODE|O_APPEND|O_SYNC));
    file->offset = 0;
    return 0;
  }

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Close an open file
 *
 *  @param[in,out] r  newlib reentrancy struct
 *  @param[in]     fd Pointer to archive_file_t
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_close(struct _reent *r,
              void          *fd)
{
  Result      rc;

  /* get pointer to our data */
  archive_file_t *file = (archive_file_t*)fd;

  rc = FSFILE_Close(file->fd);
  if(R_SUCCEEDED(rc))
    return 0;

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Write to an open file
 *
 *  @param[in,out] r   newlib reentrancy struct
 *  @param[in,out] fd  Pointer to archive_file_t
 *  @param[in]     ptr Pointer to data to write
 *  @param[in]     len Length of data to write
 *
 *  @returns number of bytes written
 *  @returns -1 for error
 */
static ssize_t
archive_write(struct _reent *r,
              void          *fd,
              const char    *ptr,
              size_t        len)
{
  Result      rc;
  u32         bytes;
  u32         sync = 0;

  /* get pointer to our data */
  archive_file_t *file = (archive_file_t*)fd;

  /* check that the file was opened with write access */
  if((file->flags & O_ACCMODE) == O_RDONLY)
  {
    r->_errno = EBADF;
    return -1;
  }

  /* check if this is synchronous or not */
  if(file->flags & O_SYNC)
    sync = FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME;

  if(file->flags & O_APPEND)
  {
    /* append means write from the end of the file */
    rc = FSFILE_GetSize(file->fd, &file->offset);
    if(R_FAILED(rc))
    {
      r->_errno = archive_translate_error(rc);
      return -1;
    }
  }

  rc = FSFILE_Write(file->fd, &bytes, file->offset,
                    (u32*)ptr, len, sync);
  if(R_FAILED(rc))
  {
    r->_errno = archive_translate_error(rc);
    return -1;
  }

  file->offset += bytes;

  return bytes;
}

/*! Read from an open file
 *
 *  @param[in,out] r   newlib reentrancy struct
 *  @param[in,out] fd  Pointer to archive_file_t
 *  @param[out]    ptr Pointer to buffer to read into
 *  @param[in]     len Length of data to read
 *
 *  @returns number of bytes read
 *  @returns -1 for error
 */
static ssize_t
archive_read(struct _reent *r,
             void          *fd,
             char          *ptr,
             size_t         len)
{
  Result      rc;
  u32         bytes;

  /* get pointer to our data */
  archive_file_t *file = (archive_file_t*)fd;

  /* check that the file was opened with read access */
  if((file->flags & O_ACCMODE) == O_WRONLY)
  {
    r->_errno = EBADF;
    return -1;
  }

  /* read the data */
  rc = FSFILE_Read(file->fd, &bytes, file->offset, (u32*)ptr, (u32)len);
  if(R_SUCCEEDED(rc))
  {
    /* update current file offset */
    file->offset += bytes;
    return (ssize_t)bytes;
  }

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Update an open file's current offset
 *
 *  @param[in,out] r      newlib reentrancy struct
 *  @param[in,out] fd     Pointer to archive_file_t
 *  @param[in]     pos    Offset to seek to
 *  @param[in]     whence Where to seek from
 *
 *  @returns new offset for success
 *  @returns -1 for error
 */
static off_t
archive_seek(struct _reent *r,
             void          *fd,
             off_t         pos,
             int           whence)
{
  Result      rc;
  u64         offset;

  /* get pointer to our data */
  archive_file_t *file = (archive_file_t*)fd;

  /* find the offset to see from */
  switch(whence)
  {
    /* set absolute position; start offset is 0 */
    case SEEK_SET:
      offset = 0;
      break;

    /* set position relative to the current position */
    case SEEK_CUR:
      offset = file->offset;
      break;

    /* set position relative to the end of the file */
    case SEEK_END:
      rc = FSFILE_GetSize(file->fd, &offset);
      if(R_FAILED(rc))
      {
        r->_errno = archive_translate_error(rc);
        return -1;
      }
      break;

    /* an invalid option was provided */
    default:
      r->_errno = EINVAL;
      return -1;
  }

  /* TODO: A better check that prevents overflow. */
  if(pos < 0 && offset < -pos)
  {
    /* don't allow seek to before the beginning of the file */
    r->_errno = EINVAL;
    return -1;
  }

  /* update the current offset */
  file->offset = offset + pos;
  return file->offset;
}

/*! Get file stats from an open file
 *
 *  @param[in,out] r  newlib reentrancy struct
 *  @param[in]     fd Pointer to archive_file_t
 *  @param[out]    st Pointer to file stats to fill
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_fstat(struct _reent *r,
              void          *fd,
              struct stat   *st)
{
  Result      rc;
  u64         size;
  archive_file_t *file = (archive_file_t*)fd;

  rc = FSFILE_GetSize(file->fd, &size);
  if(R_SUCCEEDED(rc))
  {
    memset(st, 0, sizeof(struct stat));
    st->st_size = (off_t)size;
    st->st_nlink = 1;
    st->st_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    return 0;
  }

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Get file stats
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     file Path to file
 *  @param[out]    st   Pointer to file stats to fill
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_stat(struct _reent *r,
             const char    *file,
             struct stat   *st)
{
  Handle  fd;
  Result  rc;
  FS_Path fs_path;
  archive_fsdevice *device = r->deviceData;

  fs_path = archive_utf16path(r, file, &device);
  if(fs_path.data == NULL)
    return -1;

  if(R_SUCCEEDED(rc = FSUSER_OpenFile(&fd, device->archive, fs_path, FS_OPEN_READ, 0)))
  {
    archive_file_t tmpfd = { .fd = fd };
    rc = archive_fstat(r, &tmpfd, st);
    FSFILE_Close(fd);

    return rc;
  }
  else if(R_SUCCEEDED(rc = FSUSER_OpenDirectory(&fd, device->archive, fs_path)))
  {
    memset(st, 0, sizeof(struct stat));
    st->st_nlink = 1;
    st->st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
    FSDIR_Close(fd);
    return 0;
  }

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Hard link a file
 *
 *  @param[in,out] r        newlib reentrancy struct
 *  @param[in]     existing Path of file to link
 *  @param[in]     newLink  Path of new link
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_link(struct _reent *r,
             const char    *existing,
             const char    *newLink)
{
  r->_errno = ENOSYS;
  return -1;
}

/*! Unlink a file
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     name Path of file to unlink
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_unlink(struct _reent *r,
               const char    *name)
{
  Result  rc;
  FS_Path fs_path;
  archive_fsdevice *device = r->deviceData;

  fs_path = archive_utf16path(r, name, &device);
  if(fs_path.data == NULL)
    return -1;

  rc = FSUSER_DeleteFile(device->archive, fs_path);
  if(R_SUCCEEDED(rc))
    return 0;

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Change current working directory
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     name Path to new working directory
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_chdir(struct _reent *r,
              const char    *name)
{
  Handle  fd;
  Result  rc;
  FS_Path fs_path;
  archive_fsdevice *device = r->deviceData;

  fs_path = archive_utf16path(r, name, &device);
  if(fs_path.data == NULL)
    return -1;

  rc = FSUSER_OpenDirectory(&fd, device->archive, fs_path);
  if(R_SUCCEEDED(rc))
  {
    FSDIR_Close(fd);
    strncpy(device->cwd, __ctru_dev_path_buf, PATH_MAX + 1);
    device->cwd[PATH_MAX] = '\0';
    return 0;
  }

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Rename a file
 *
 *  @param[in,out] r       newlib reentrancy struct
 *  @param[in]     oldName Path to rename from
 *  @param[in]     newName Path to rename to
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_rename(struct _reent *r,
               const char    *oldName,
               const char    *newName)
{
  Result  rc;
  FS_Path fs_path_old, fs_path_new;
  archive_fsdevice *sourceDevice = r->deviceData;
  archive_fsdevice *destDevice = NULL;

  /* treat extdata as read-only */
  if(sourceDevice->is_extdata)
  {
    r->_errno = ENOTSUP;
    return -1;
  }

  fs_path_old = archive_utf16path(r, oldName, &sourceDevice);
  if(fs_path_old.data == NULL)
    return -1;

  uint8_t old_path_copy[fs_path_old.size];
  memcpy(old_path_copy, fs_path_old.data, fs_path_old.size);
  fs_path_old.data = old_path_copy;

  fs_path_new = archive_utf16path(r, newName, &destDevice);
  if(fs_path_new.data == NULL)
    return -1;

  /* moving between archives is not supported */
  if(sourceDevice->archive != destDevice->archive)
  {
    r->_errno = ENOTSUP;
    return -1;
  }

  rc = FSUSER_RenameFile(sourceDevice->archive, fs_path_old, sourceDevice->archive, fs_path_new);
  /* if the file at the target destination exists, overwrite it */
  if(R_FAILED(rc) && R_DESCRIPTION(rc) == RD_ALREADY_EXISTS) {
    rc = FSUSER_DeleteFile(sourceDevice->archive, fs_path_new);
    if(R_FAILED(rc)) {
      r->_errno = archive_translate_error(rc);
      return -1;
    }
    rc = FSUSER_RenameFile(sourceDevice->archive, fs_path_old, sourceDevice->archive, fs_path_new);
    if(R_SUCCEEDED(rc)) return 0;
  } else if(R_SUCCEEDED(rc)) return 0;

  rc = FSUSER_RenameDirectory(sourceDevice->archive, fs_path_old, sourceDevice->archive, fs_path_new);
  /* if the directory at the target destination exists, overwrite it */
  if(R_FAILED(rc) && R_DESCRIPTION(rc) == RD_ALREADY_EXISTS) {
    /* only overwrite empty directories */
    rc = FSUSER_DeleteDirectory(sourceDevice-> archive, fs_path_new);
    if(R_FAILED(rc)) {
      r->_errno = archive_translate_error(rc);
      return -1;
    }
    rc = FSUSER_RenameDirectory(sourceDevice->archive, fs_path_old, sourceDevice->archive, fs_path_new);
    if(R_SUCCEEDED(rc)) return 0;
  } else if(R_SUCCEEDED(rc)) return 0;

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Create a directory
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     path Path of directory to create
 *  @param[in]     mode Permissions of created directory
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_mkdir(struct _reent *r,
              const char    *path,
              int           mode)
{
  Result  rc;
  FS_Path fs_path;
  archive_fsdevice *device = r->deviceData;

  /* treat extdata as read-only */
  if(device->is_extdata)
  {
    r->_errno = ENOTSUP;
    return -1;
  }

  fs_path = archive_utf16path(r, path, &device);
  if(fs_path.data == NULL)
    return -1;

  /* TODO: Use mode to set directory attributes. */

  rc = FSUSER_CreateDirectory(device->archive, fs_path, 0);
  if(rc == 0xC82044BE)
  {
    r->_errno = EEXIST;
    return -1;
  }
  else if(R_SUCCEEDED(rc))
    return 0;

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Open a directory
 *
 *  @param[in,out] r        newlib reentrancy struct
 *  @param[in]     dirState Pointer to open directory state
 *  @param[in]     path     Path of directory to open
 *
 *  @returns dirState for success
 *  @returns NULL for error
 */
static DIR_ITER*
archive_diropen(struct _reent *r,
                DIR_ITER      *dirState,
                const char    *path)
{
  Handle  fd;
  Result  rc;
  FS_Path fs_path;
  archive_fsdevice *device = r->deviceData;

  fs_path = archive_utf16path(r, path, &device);

  if(fs_path.data == NULL)
    return NULL;

  /* get pointer to our data */
  archive_dir_t *dir = (archive_dir_t*)(dirState->dirStruct);

  /* open the directory */
  rc = FSUSER_OpenDirectory(&fd, device->archive, fs_path);
  if(R_SUCCEEDED(rc))
  {
    dir->magic = ARCHIVE_DIRITER_MAGIC;
    dir->fd    = fd;
    dir->index = -1;
    dir->size  = 0;
    memset(&dir->entry_data, 0, sizeof(dir->entry_data));
    return dirState;
  }

  r->_errno = archive_translate_error(rc);
  return NULL;
}

/*! Reset an open directory to its intial state
 *
 *  @param[in,out] r        newlib reentrancy struct
 *  @param[in]     dirState Pointer to open directory state
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_dirreset(struct _reent *r,
                 DIR_ITER      *dirState)
{
  r->_errno = ENOSYS;
  return -1;
}

/*! Fetch the next entry of an open directory
 *
 *  @param[in,out] r        newlib reentrancy struct
 *  @param[in]     dirState Pointer to open directory state
 *  @param[out]    filename Buffer to store entry name
 *  @param[out]    filestat Buffer to store entry attributes
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_dirnext(struct _reent *r,
                DIR_ITER      *dirState,
                char          *filename,
                struct stat   *filestat)
{
  Result              rc;
  u32                 entries;
  ssize_t             units;
  FS_DirectoryEntry   *entry;

  /* get pointer to our data */
  archive_dir_t *dir = (archive_dir_t*)(dirState->dirStruct);

  static const size_t max_entries = sizeof(dir->entry_data) / sizeof(dir->entry_data[0]);

  /* check if it's in the batch already */
  if(++dir->index < dir->size)
  {
    rc = 0;
  }
  else
  {
    /* reset batch info */
    dir->index = -1;
    dir->size  = 0;

    /* fetch the next batch */
    memset(dir->entry_data, 0, sizeof(dir->entry_data));
    rc = FSDIR_Read(dir->fd, &entries, max_entries, dir->entry_data);
    if(R_SUCCEEDED(rc))
    {
      if(entries == 0)
      {
        /* there are no more entries; ENOENT signals end-of-directory */
        r->_errno = ENOENT;
        return -1;
      }

      dir->index = 0;
      dir->size  = entries;
    }
  }

  if(R_SUCCEEDED(rc))
  {
    entry = &dir->entry_data[dir->index];

    /* fill in the stat info */
    filestat->st_ino = 0;
    if(entry->attributes & FS_ATTRIBUTE_DIRECTORY)
      filestat->st_mode = S_IFDIR;
    else
      filestat->st_mode = S_IFREG;

    /* convert name from UTF-16 to UTF-8 */
    memset(filename, 0, NAME_MAX);
    units = utf16_to_utf8((uint8_t*)filename, entry->name, NAME_MAX);
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

    return 0;
  }

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Close an open directory
 *
 *  @param[in,out] r        newlib reentrancy struct
 *  @param[in]     dirState Pointer to open directory state
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_dirclose(struct _reent *r,
                 DIR_ITER      *dirState)
{
  Result         rc;

  /* get pointer to our data */
  archive_dir_t *dir = (archive_dir_t*)(dirState->dirStruct);

  /* close the directory */
  rc = FSDIR_Close(dir->fd);
  if(R_SUCCEEDED(rc))
    return 0;

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Get filesystem statistics for an archive
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     path Path to filesystem to get statistics of
 *  @param[out]    buf  Buffer to fill
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_statvfs(struct _reent  *r,
                const char     *path,
                struct statvfs *buf)
{
  // Result rc;
  // u32 totalSize, directories, files;
  // bool duplicateData;

  // rc = FSUSER_GetFormatInfo(&totalSize, &directories, &files, &duplicateData, )

  r->_errno = ENOSYS;

  return -1;

  // Result rc;
  // FS_ArchiveResource resource;
  // bool writable = false;

  // rc = FSUSER_GetArchiveResource(&resource);

  // if(R_SUCCEEDED(rc))
  // {
  //   buf->f_bsize   = resource.clusterSize;
  //   buf->f_frsize  = resource.clusterSize;
  //   buf->f_blocks  = resource.totalClusters;
  //   buf->f_bfree   = resource.freeClusters;
  //   buf->f_bavail  = resource.freeClusters;
  //   buf->f_files   = 0; //??? how to get
  //   buf->f_ffree   = resource.freeClusters;
  //   buf->f_favail  = resource.freeClusters;
  //   buf->f_fsid    = 0; //??? how to get
  //   buf->f_flag    = ST_NOSUID;
  //   buf->f_namemax = 0; //??? how to get

  //   rc = FSUSER_IsarchiveWritable(&writable);
  //   if(R_FAILED(rc) || !writable)
  //     buf->f_flag |= ST_RDONLY;

  //   return 0;
  // }

  // r->_errno = archive_translate_error(rc);
  // return -1;
}

/*! Get filesystem statistics for the SD card
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     path Path to filesystem to get statistics of
 *  @param[out]    buf  Buffer to fill
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
sdmc_statvfs(struct _reent  *r,
             const char     *path,
             struct statvfs *buf)
{
  Result rc;
  FS_ArchiveResource resource;
  bool writable = false;

  rc = FSUSER_GetSdmcArchiveResource(&resource);

  if(R_SUCCEEDED(rc))
  {
    buf->f_bsize   = resource.clusterSize;
    buf->f_frsize  = resource.clusterSize;
    buf->f_blocks  = resource.totalClusters;
    buf->f_bfree   = resource.freeClusters;
    buf->f_bavail  = resource.freeClusters;
    buf->f_files   = 0; //??? how to get
    buf->f_ffree   = resource.freeClusters;
    buf->f_favail  = resource.freeClusters;
    buf->f_fsid    = 0; //??? how to get
    buf->f_flag    = ST_NOSUID;
    buf->f_namemax = 0; //??? how to get

    rc = FSUSER_IsSdmcWritable(&writable);
    if(R_FAILED(rc) || !writable)
      buf->f_flag |= ST_RDONLY;

    return 0;
  }

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Truncate an open file
 *
 *  @param[in,out] r   newlib reentrancy struct
 *  @param[in]     fd  Pointer to archive_file_t
 *  @param[in]     len Length to truncate file to
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_ftruncate(struct _reent *r,
                  void          *fd,
                  off_t         len)
{
  Result      rc;

  /* get pointer to our data */
  archive_file_t *file = (archive_file_t*)fd;

  /* make sure length is non-negative */
  if(len < 0)
  {
    r->_errno = EINVAL;
    return -1;
  }

  /* set the new file size */
  rc = FSFILE_SetSize(file->fd, len);
  if(R_SUCCEEDED(rc))
    return 0;

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Synchronize a file to media
 *
 *  @param[in,out] r  newlib reentrancy struct
 *  @param[in]     fd Pointer to archive_file_t
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_fsync(struct _reent *r,
              void          *fd)
{
  Result rc;

  /* get pointer to our data */
  archive_file_t *file = (archive_file_t*)fd;

  rc = FSFILE_Flush(file->fd);
  if(R_SUCCEEDED(rc))
    return 0;

  r->_errno = archive_translate_error(rc);
  return -1;
}

/*! Change a file's mode
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     path Path to file to update
 *  @param[in]     mode New mode to set
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_chmod(struct _reent *r,
              const char    *path,
              mode_t        mode)
{
  r->_errno = ENOSYS;
  return -1;
}

/*! Change an open file's mode
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     fd   Pointer to archive_file_t
 *  @param[in]     mode New mode to set
 *
 *  @returns 0 for success
 *  @returns -1 for failure
 */
static int
archive_fchmod(struct _reent *r,
               void          *fd,
               mode_t        mode)
{
  r->_errno = ENOSYS;
  return -1;
}

/*! Remove a directory
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     name Path of directory to remove
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
archive_rmdir(struct _reent *r,
              const char    *name)
{
  Result  rc;
  FS_Path fs_path;
  archive_fsdevice *device = r->deviceData;

  /* treat extdata as read-only */
  if(device->is_extdata)
  {
    r->_errno = ENOTSUP;
    return -1;
  }

  fs_path = archive_utf16path(r, name, &device);
  if(fs_path.data == NULL)
    return -1;

  rc = FSUSER_DeleteDirectory(device->archive, fs_path);
  if(R_SUCCEEDED(rc))
    return 0;

  r->_errno = archive_translate_error(rc);
  return -1;
}

Result
archive_getmtime(const char *name,
                 u64        *mtime)
{
  Result        rc;
  FS_Path       fs_path;
  struct _reent r;
  archive_fsdevice *device = NULL;

  r._errno = 0;

  fs_path = archive_utf16path(&r, name, &device);
  if(r._errno != 0)
    errno = r._errno;

  if(fs_path.data == NULL)
    return -1;

  rc = FSUSER_ControlArchive(device->archive, ARCHIVE_ACTION_GET_TIMESTAMP,
                             (void*)fs_path.data, fs_path.size,
                             mtime, sizeof(*mtime));
  if(rc == 0)
  {
    /* convert from milliseconds to seconds */
    *mtime /= 1000;
    /* convert from 2000-based timestamp to UNIX timestamp */
    *mtime += 946684800;
  }

  return rc;
}

/*! Error map */
typedef struct
{
  Result fs_error; //!< Error from FS service
  int    error;    //!< POSIX errno
} error_map_t;

/*! Error table */
static const error_map_t error_table[] =
{
  /* keep this list sorted! */
  { 0x082044BE, EEXIST,       },
  { 0x086044D2, ENOSPC,       },
  { 0xC8804478, ENOENT,       },
  { 0xC92044FA, ENOENT,       },
  { 0xE0E046BE, EINVAL,       },
  { 0xE0E046BF, ENAMETOOLONG, },
};
static const size_t num_errors = sizeof(error_table)/sizeof(error_table[0]);

/*! Comparison function for bsearch on error_table
 *
 *  @param[in] p1 Left side of comparison
 *  @param[in] p2 Right side of comparison
 *
 *  @returns <0 if lhs < rhs
 *  @returns >0 if lhs > rhs
 *  @returns 0  if lhs == rhs
 */
static int
error_cmp(const void *p1, const void *p2)
{
  const error_map_t *lhs = (const error_map_t*)p1;
  const error_map_t *rhs = (const error_map_t*)p2;

  if((u32)lhs->fs_error < (u32)rhs->fs_error)
    return -1;
  else if((u32)lhs->fs_error > (u32)rhs->fs_error)
    return 1;
  return 0;
}

/*! Translate FS service error to errno
 *
 *  @param[in] error FS service error
 *
 *  @returns errno
 */
static int
archive_translate_error(Result error)
{
  error_map_t key = { .fs_error = error };
  const error_map_t *rc = bsearch(&key, error_table, num_errors,
                                  sizeof(error_map_t), error_cmp);

  if(rc != NULL)
    return rc->error;

  return (int)error;
}
