#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/iosupport.h>
#include <sys/param.h>

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <3ds.h>

/*! @internal
 *
 *  @file sdmc_dev.c
 *
 *  SDMC Device
 */

static int       sdmc_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
static int       sdmc_close(struct _reent *r, int fd);
static ssize_t   sdmc_write(struct _reent *r, int fd, const char *ptr, size_t len);
static ssize_t   sdmc_read(struct _reent *r, int fd, char *ptr, size_t len);
static off_t     sdmc_seek(struct _reent *r, int fd, off_t pos, int dir);
static int       sdmc_fstat(struct _reent *r, int fd, struct stat *st);
static int       sdmc_stat(struct _reent *r, const char *file, struct stat *st);
static int       sdmc_link(struct _reent *r, const char *existing, const char  *newLink);
static int       sdmc_unlink(struct _reent *r, const char *name);
static int       sdmc_chdir(struct _reent *r, const char *name);
static int       sdmc_rename(struct _reent *r, const char *oldName, const char *newName);
static int       sdmc_mkdir(struct _reent *r, const char *path, int mode);
static DIR_ITER* sdmc_diropen(struct _reent *r, DIR_ITER *dirState, const char *path);
static int       sdmc_dirreset(struct _reent *r, DIR_ITER *dirState);
static int       sdmc_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
static int       sdmc_dirclose(struct _reent *r, DIR_ITER *dirState);
static int       sdmc_statvfs(struct _reent *r, const char *path, struct statvfs *buf);
static int       sdmc_ftruncate(struct _reent *r, int fd, off_t len);
static int       sdmc_fsync(struct _reent *r, int fd);
static int       sdmc_chmod(struct _reent *r, const char *path, mode_t mode);
static int       sdmc_fchmod(struct _reent *r, int fd, mode_t mode);

/*! @cond INTERNAL */

#define SDMC_FILEBUFFSIZE   0x4000               /* Size of file data buffer */
#define SDMC_FILEBUFFMASK (-SDMC_FILEBUFFSIZE)   /* For calculating the start of the buffer's current chunk */
#define SDMC_FILEOFFMASK   (SDMC_FILEBUFFSIZE-1) /* For calculating the start offset within the buffer */

#define SDMC_BUFFINVALID   0xFFFFFFFF          /* To indicate that a buffer has not actually been loaded yet */

/*! Open file struct */
typedef struct
{
  Handle fd;     /*! CTRU handle */
  int    flags;  /*! Flags used in open(2) */
  u64    offset; /*! Current file offset */

  u8     buffer[SDMC_FILEBUFFSIZE]; /* File data buffer for speed-of-access */
  u64    boffset;                   /* File start offset of the current buffered chunk */
  u32    bsize;                     /* Amount of valid data in the buffer before EOF */

  u64    size;                      /* Size of the file in bytes*/

} sdmc_file_t;

/*! Open directory struct */
typedef struct
{
  Handle    fd;         /*! CTRU handle */
  FS_dirent entry_data; /*! Temporary storage for reading entries */
} sdmc_dir_t;

/*! SDMC devoptab */
static devoptab_t
sdmc_devoptab =
{
  .name         = "sdmc",
  .structSize   = sizeof(sdmc_file_t),
  .open_r       = sdmc_open,
  .close_r      = sdmc_close,
  .write_r      = sdmc_write,
  .read_r       = sdmc_read,
  .seek_r       = sdmc_seek,
  .fstat_r      = sdmc_fstat,
  .stat_r       = sdmc_stat,
  .link_r       = sdmc_link,
  .unlink_r     = sdmc_unlink,
  .chdir_r      = sdmc_chdir,
  .rename_r     = sdmc_rename,
  .mkdir_r      = sdmc_mkdir,
  .dirStateSize = sizeof(sdmc_dir_t),
  .diropen_r    = sdmc_diropen,
  .dirreset_r   = sdmc_dirreset,
  .dirnext_r    = sdmc_dirnext,
  .dirclose_r   = sdmc_dirclose,
  .statvfs_r    = sdmc_statvfs,
  .ftruncate_r  = sdmc_ftruncate,
  .fsync_r      = sdmc_fsync,
  .deviceData   = NULL,
  .chmod_r      = sdmc_chmod,
  .fchmod_r     = sdmc_fchmod,
};

/*! SDMC archive handle */
static FS_archive sdmcArchive =
{
  .id = ARCH_SDMC,
  .lowPath =
  {
    .type = PATH_EMPTY,
    .size = 1,
    .data = (u8*)"",
  },
};

/*! @endcond */

static char __cwd[PATH_MAX+1] = "/";
static char __fixedpath[PATH_MAX+1];

static const char *sdmc_fixpath(const char *path)
{
  // Move the path pointer to the start of the actual path
  if (strchr (path, ':') != NULL)
  {
    path = strchr (path, ':') + 1;
  }

  if (strchr (path, ':') != NULL) return NULL;


  if (path[0]=='/') return path;

  strncpy(__fixedpath,__cwd,PATH_MAX);
  strncat(__fixedpath,path,PATH_MAX);
  __fixedpath[PATH_MAX] = 0;

  return __fixedpath;

}

extern int __system_argc;
extern char** __system_argv;

/*! Initialize SDMC device */
Result sdmcInit(void)
{
  Result rc;


  rc = FSUSER_OpenArchive(NULL, &sdmcArchive);


  if(rc == 0)
  {

    int dev = AddDevice(&sdmc_devoptab);

    if (__system_argc != 0 && __system_argv[0] != NULL)
    {
      if (FindDevice(__system_argv[0]) == dev)
      {
        strncpy(__fixedpath,__system_argv[0],PATH_MAX);
        char *last_slash = strrchr(__fixedpath,'/');
        if (last_slash != NULL) {
          last_slash[0] = 0;
          chdir(__fixedpath);
        }
      }
    }
  }

  return rc;
}

/*! Clean up SDMC device */
Result sdmcExit(void)
{
  Result rc;

  rc = FSUSER_CloseArchive(NULL, &sdmcArchive);
  if(rc == 0)
    RemoveDevice("sdmc");

  return rc;
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
sdmc_open(struct _reent *r,
          void          *fileStruct,
          const char    *path,
          int           flags,
          int           mode)
{
  Handle      fd;
  Result      rc;
  u32         sdmc_flags = 0;
  u32         attributes = FS_ATTRIBUTE_NONE;
  const char  *pathptr = NULL;

  pathptr = sdmc_fixpath(path);

  if(pathptr==NULL)
  {
    r->_errno=EINVAL;
    return -1;
  }

  /* get pointer to our data */
  sdmc_file_t *file = (sdmc_file_t*)fileStruct;

  /* check access mode */
  switch(flags & O_ACCMODE)
  {
    /* read-only: do not allow O_APPEND */
    case O_RDONLY:
      sdmc_flags |= FS_OPEN_READ;
      if(flags & O_APPEND)
      {
        r->_errno = EINVAL;
        return -1;
      }
      break;

    /* write-only */
    case O_WRONLY:
      sdmc_flags |= FS_OPEN_WRITE;
      break;

    /* read and write */
    case O_RDWR:
      sdmc_flags |= (FS_OPEN_READ | FS_OPEN_WRITE);
      break;

    /* an invalid option was supplied */
    default:
      r->_errno = EINVAL;
      return -1;
  }

  /* create file */
  if(flags & O_CREAT)
    sdmc_flags |= FS_OPEN_CREATE;

  /* TODO: Test O_EXCL. */

  /* set attributes */
  /*if(!(mode & S_IWUSR))
    attributes |= FS_ATTRIBUTE_READONLY;*/

  /* open the file */
  rc = FSUSER_OpenFile(NULL, &fd, sdmcArchive, FS_makePath(PATH_CHAR, pathptr),
                       sdmc_flags, attributes);
  if(rc == 0)
  {
    file->fd     = fd;
    file->flags  = (flags & (O_ACCMODE|O_APPEND|O_SYNC));
    file->offset = 0;

    /* initialize buffer offset to buffer on first-read/write */
    file->boffset = SDMC_BUFFINVALID;
    file->bsize   = 0;

    /* record the file's size */
    rc = FSFILE_GetSize(file->fd, &file->size);
    if(rc != 0)
    {
      r->_errno = rc;
      return -1;
    }

    return 0;
  }

  r->_errno = rc;
  return -1;
}

/*! Close an open file
 *
 *  @param[in,out] r  newlib reentrancy struct
 *  @param[in]     fd Pointer to sdmc_file_t
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
sdmc_close(struct _reent *r,
           int           fd)
{
  Result      rc;

  u32 bytes;

  /* get pointer to our data */
  sdmc_file_t *file = (sdmc_file_t*)fd;

  /* check if the file was opened with write access */
  if((file->flags & O_ACCMODE) != O_RDONLY)
  {
   /* flush the current buffer */
   if((rc = FSFILE_Write(file->fd, &bytes, file->boffset, file->buffer, file->bsize, FS_WRITE_FLUSH)) != 0)
   {
    r->_errno = rc;
    return -1;
   }
  }

  rc = FSFILE_Close(file->fd);
  if(rc == 0)
    return 0;

  r->_errno = rc;
  return -1;
}

/*! Write to an open file
 *
 *  @param[in,out] r   newlib reentrancy struct
 *  @param[in,out] fd  Pointer to sdmc_file_t
 *  @param[in]     ptr Pointer to data to write
 *  @param[in]     len Length of data to write
 *
 *  @returns number of bytes written
 *  @returns -1 for error
 */
static ssize_t
sdmc_write(struct _reent *r,
           int           fd,
           const char    *ptr,
           size_t        len)
{
  Result      rc;
  u32         bytes;
  u32         sync = 0;
  u64         offset;

  u32         start;
  u32         curlen;
  u32         total;

  /* get pointer to our data */
  sdmc_file_t *file = (sdmc_file_t*)fd;

  /* check that the file was opened with write access */
  if((file->flags & O_ACCMODE) == O_RDONLY)
  {
    r->_errno = EBADF;
    return -1;
  }

  /* check if this is synchronous or not */
  if(file->flags & O_SYNC)
    sync = FS_WRITE_FLUSH;

  /* initialize offset */
  offset = file->offset;
  if(file->flags & O_APPEND)
  {
    /* append means write from the end of the file */
    rc = FSFILE_GetSize(file->fd, &offset);
    if(rc != 0)
    {
      r->_errno = rc;
      return -1;
    }
  }

  /* initialize buffer start position, copy length, and total copied count */
  start  = offset & SDMC_FILEOFFMASK;
  curlen = SDMC_FILEBUFFSIZE-start;
  total  = 0;

  /* if the target location is not in the currently-buffered chunk, save the current buffer and load the target chunk */
  if(file->boffset != (offset & SDMC_FILEBUFFMASK))
  {

   /* if there is a valid buffer */
   if(file->boffset != SDMC_BUFFINVALID)
   {
buffernext:

    /* flush the current buffer */
    if((rc = FSFILE_Write(file->fd, &bytes, file->boffset, file->buffer, file->bsize, sync)) != 0)
    {
     r->_errno = rc;
     return -1;
    }
   }

   /* buffer the target chunk */
   file->boffset = offset & SDMC_FILEBUFFMASK;
   if((rc = FSFILE_Read(file->fd, &file->bsize, file->boffset, file->buffer, SDMC_FILEBUFFSIZE)) != 0)
   {
    r->_errno = rc;
    return -1;
   }
  }
//   else
//    bytes = file->boffset + start;

  if(curlen > len)
   curlen = len;

  memcpy(file->buffer+start, ptr, curlen);
  total += curlen;
  offset += curlen;
  if((offset - file->boffset) > file->bsize)
   file->bsize = offset - file->boffset;
  if((len -= curlen) > 0)
  {
   ptr += curlen;
   start = 0;
   if(len > SDMC_FILEBUFFSIZE)
    curlen = SDMC_FILEBUFFSIZE;
   else
    curlen = len;
   goto buffernext;
  }

  file->offset = offset;

  /* return the total number of bytes that were actually copied */
  return (ssize_t)total;
}

/*! Read from an open file
 *
 *  @param[in,out] r   newlib reentrancy struct
 *  @param[in,out] fd  Pointer to sdmc_file_t
 *  @param[out]    ptr Pointer to buffer to read into
 *  @param[in]     len Length of data to read
 *
 *  @returns number of bytes read
 *  @returns -1 for error
 */
static ssize_t
sdmc_read(struct _reent *r,
          int           fd,
          char          *ptr,
          size_t         len)
{
  Result      rc;
//   u32         bytes;

  u32         start;
  u32         curlen;
  u32         total;

  /* get pointer to our data */
  sdmc_file_t *file = (sdmc_file_t*)fd;

  /* check that the file was opened with read access */
  if((file->flags & O_ACCMODE) == O_WRONLY)
  {
    r->_errno = EBADF;
    return -1;
  }

  if((file->offset + len) > file->size)
   if((len = (file->size - file->offset)) <= 0)
    return(0);

  /* initialize buffer start position, copy length, and total copied count */
  start  = file->offset & SDMC_FILEOFFMASK;
  curlen = SDMC_FILEBUFFSIZE-start;
  total  = 0;

  /* if the requested data does not reside in the currently-buffered chunk, load the buffer */
  if(file->boffset != (file->offset & SDMC_FILEBUFFMASK))
  {
buffernext:
   file->boffset = file->offset & SDMC_FILEBUFFMASK;
   /* read the data */
   if((rc = FSFILE_Read(file->fd, &file->bsize, file->boffset, file->buffer, SDMC_FILEBUFFSIZE)) != 0)
   {
    r->_errno = rc;
    return -1;
   }
  }
//   else
//    bytes = file->boffset + start;

  if(curlen > len)
   curlen = len;
  if(curlen > file->bsize)
   curlen = file->bsize;

  memcpy(ptr, file->buffer+start, curlen);
  total += curlen;
  file->offset += curlen;
  if((len -= curlen) > 0)
  {
   if(curlen + start == SDMC_FILEBUFFSIZE)
   {
    ptr += curlen;
    start = 0;
    if(len > SDMC_FILEBUFFSIZE)
     curlen = SDMC_FILEBUFFSIZE;
    else
     curlen = len;
    goto buffernext;
   }
  }

  /* return the total number of bytes that were actually copied */
  return (ssize_t)total;

}

/*! Update an open file's current offset
 *
 *  @param[in,out] r      newlib reentrancy struct
 *  @param[in,out] fd     Pointer to sdmc_file_t
 *  @param[in]     pos    Offset to seek to
 *  @param[in]     whence Where to seek from
 *
 *  @returns new offset for success
 *  @returns -1 for error
 */
static off_t
sdmc_seek(struct _reent *r,
          int           fd,
          off_t         pos,
          int           whence)
{
  Result      rc;
  u64         offset;

  /* get pointer to our data */
  sdmc_file_t *file = (sdmc_file_t*)fd;

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
      if(rc != 0)
      {
        r->_errno = rc;
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
 *  @param[in]     fd Pointer to sdmc_file_t
 *  @param[out]    st Pointer to file stats to fill
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
sdmc_fstat(struct _reent *r,
           int           fd,
           struct stat   *st)
{
  r->_errno = ENOSYS;
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
sdmc_stat(struct _reent *r,
          const char    *file,
          struct stat   *st)
{
  r->_errno = ENOSYS;
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
sdmc_link(struct _reent *r,
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
sdmc_unlink(struct _reent *r,
            const char    *name)
{
  r->_errno = ENOSYS;
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
sdmc_chdir(struct _reent *r,
           const char    *name)
{
  Handle fd;
  Result rc;
  const char     *pathptr = NULL;

  pathptr = sdmc_fixpath(name);

  if(pathptr==NULL)
  {
    r->_errno=EINVAL;
    return -1;
  }

  rc = FSUSER_OpenDirectory(NULL, &fd, sdmcArchive, FS_makePath(PATH_CHAR, pathptr));
  if(rc == 0)
  {
    FSDIR_Close(fd);
    strncpy(__cwd,pathptr,PATH_MAX);
  }
  else
  {
    r->_errno=EINVAL;
    return -1;
  }

  return 0;

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
sdmc_rename(struct _reent *r,
            const char    *oldName,
            const char    *newName)
{
  r->_errno = ENOSYS;
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
sdmc_mkdir(struct _reent *r,
           const char    *path,
           int           mode)
{
  Result rc;
  const char *pathptr = NULL;

  pathptr = sdmc_fixpath(path);

  if(pathptr==NULL)
  {
    r->_errno=EINVAL;
    return -1;
  }

  /* TODO: Use mode to set directory attributes. */

  rc = FSUSER_CreateDirectory(NULL, sdmcArchive, FS_makePath(PATH_CHAR, pathptr));
  if(rc == 0)
    return 0;

  r->_errno = ENOSYS;
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
sdmc_diropen(struct _reent *r,
             DIR_ITER      *dirState,
             const char    *path)
{
  Handle         fd;
  Result         rc;
  const char     *pathptr = NULL;

  pathptr = sdmc_fixpath(path);

  if(pathptr==NULL)
  {
    r->_errno=EINVAL;
    return NULL;
  }

  /* get pointer to our data */
  sdmc_dir_t *dir = (sdmc_dir_t*)(dirState->dirStruct);

  /* open the directory */
  rc = FSUSER_OpenDirectory(NULL, &fd, sdmcArchive, FS_makePath(PATH_CHAR, pathptr));
  if(rc == 0)
  {
    dir->fd = fd;
    memset(&dir->entry_data, 0, sizeof(dir->entry_data));
    return dirState;
  }

  r->_errno = rc;
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
sdmc_dirreset(struct _reent *r,
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
sdmc_dirnext(struct _reent *r,
             DIR_ITER      *dirState,
             char          *filename,
             struct stat   *filestat)
{
  Result         rc;
  u32            entries;
  u16            *name;

  /* get pointer to our data */
  sdmc_dir_t *dir = (sdmc_dir_t*)(dirState->dirStruct);

  /* fetch the next entry */
  rc = FSDIR_Read(dir->fd, &entries, 1, &dir->entry_data);
  if(rc == 0)
  {
    if(entries == 0)
    {
      /* there are no more entries; ENOENT signals end-of-directory */
      r->_errno = ENOENT;
      return -1;
    }

    /* fill in the stat info */
    filestat->st_ino = 0;
    if(dir->entry_data.isDirectory)
      filestat->st_mode = S_IFDIR;
    else
      filestat->st_mode = S_IFREG;

    /* copy the name */
    name = dir->entry_data.name;
    while(*name)
      *filename++ = *name++;
    *filename = 0;

    return 0;
  }

  r->_errno = rc;
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
sdmc_dirclose(struct _reent *r,
              DIR_ITER      *dirState)
{
  Result         rc;

  /* get pointer to our data */
  sdmc_dir_t *dir = (sdmc_dir_t*)(dirState->dirStruct);

  /* close the directory */
  rc = FSDIR_Close(dir->fd);
  if(rc == 0)
    return 0;

  r->_errno = rc;
  return -1;
}

/*! Get filesystem statistics
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
  u32    clusterSize, numClusters, freeClusters;
  u8    writable = 0;

  rc = FSUSER_GetSdmcArchiveResource(NULL,
                                     NULL,
                                     &clusterSize,
                                     &numClusters,
                                     &freeClusters);

  if(rc == 0)
  {
    buf->f_bsize   = clusterSize;
    buf->f_frsize  = clusterSize;
    buf->f_blocks  = numClusters;
    buf->f_bfree   = freeClusters;
    buf->f_bavail  = freeClusters;
    buf->f_files   = 0; //??? how to get
    buf->f_ffree   = freeClusters;
    buf->f_favail  = freeClusters;
    buf->f_fsid    = 0; //??? how to get
    buf->f_flag    = ST_NOSUID;
    buf->f_namemax = 0; //??? how to get

    rc = FSUSER_IsSdmcWritable(NULL, &writable);
    if(rc != 0 || !writable)
      buf->f_flag |= ST_RDONLY;

    return 0;
  }

  r->_errno = rc;
  return -1;
}

/*! Truncate an open file
 *
 *  @param[in,out] r   newlib reentrancy struct
 *  @param[in]     fd  Pointer to sdmc_file_t
 *  @param[in]     len Length to truncate file to
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
sdmc_ftruncate(struct _reent *r,
               int           fd,
               off_t         len)
{
  Result      rc;

  /* get pointer to our data */
  sdmc_file_t *file = (sdmc_file_t*)fd;

  /* make sure length is non-negative */
  if(len < 0)
  {
    r->_errno = EINVAL;
    return -1;
  }

  /* set the new file size */
  rc = FSFILE_SetSize(file->fd, len);
  if(rc == 0)
    return 0;

  r->_errno = rc;
  return -1;
}

/*! Synchronize a file to media
 *
 *  @param[in,out] r  newlib reentrancy struct
 *  @param[in]     fd Pointer to sdmc_file_t
 *
 *  @returns 0 for success
 *  @returns -1 for error
 */
static int
sdmc_fsync(struct _reent *r,
           int           fd)
{
  Result rc;

  u32 bytes;

  /* get pointer to our data */
  sdmc_file_t *file = (sdmc_file_t*)fd;

  /* flush the current buffer */
  if((rc = FSFILE_Write(file->fd, &bytes, file->boffset, file->buffer, file->bsize, FS_WRITE_FLUSH)) != 0)
  {
   r->_errno = rc;
   return -1;
  }

  rc = FSFILE_Flush(file->fd);
  if(rc == 0)
    return 0;

  r->_errno = rc;
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
sdmc_chmod(struct _reent *r,
          const char    *path,
          mode_t        mode)
{
  r->_errno = ENOSYS;
  return -1;
}

/*! Change an open file's mode
 *
 *  @param[in,out] r    newlib reentrancy struct
 *  @param[in]     fd   Pointer to sdmc_file_t
 *  @param[in]     mode New mode to set
 *
 *  @returns 0 for success
 *  @returns -1 for failure
 */
static int
sdmc_fchmod(struct _reent *r,
            int           fd,
            mode_t        mode)
{
  r->_errno = ENOSYS;
  return -1;
}
