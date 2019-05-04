#include <3ds/gdbhio_dev.h>
#include <3ds/gdbhio.h>

#include <sys/iosupport.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

static int _gdbHioGetFd(int fd)
{
	__handle *handle = __get_handle(fd);
	if (handle == NULL) {
		errno = EBADF;
		return -1;
	}

	if(strcmp(devoptab_list[handle->device]->name, "gdbhio") != 0) {
		errno = EBADF;
		return -1;
	}
	return *(int *)handle->fileStruct;
}

static inline int _gdbHioGetFdFromPtr(void *fdptr)
{
	return *(int *)fdptr;
}

static int _gdbHioDevOpen(struct _reent *r, void *fdptr, const char *pathname, int flags, int mode)
{
	(void)r;
	if (strncmp(pathname, "gdbhio:", 7) == 0) {
		pathname += 7;
	}

	int ret = gdbHioOpen(pathname, flags, mode);
	if (ret < 0) {
		return ret;
	} else {
		*(int *)fdptr = ret;
		return 0;
	}
}

static int _gdbHioDevClose(struct _reent *r, void *fdptr)
{
	(void)r;
	return gdbHioClose(_gdbHioGetFdFromPtr(fdptr));
}

static ssize_t _gdbHioDevRead(struct _reent *r, void *fdptr, char *buf, size_t count)
{
	(void)r;
	return gdbHioRead(_gdbHioGetFdFromPtr(fdptr), buf, (unsigned int)count);
}

static ssize_t _gdbHioDevWrite(struct _reent *r, void *fdptr, const char *buf, size_t count)
{
	(void)r;
	return gdbHioWrite(_gdbHioGetFdFromPtr(fdptr), buf, (unsigned int)count);
}

static off_t _gdbHioDevLseek(struct _reent *r, void *fdptr, off_t offset, int flag)
{
	(void)r;
	return gdbHioLseek(_gdbHioGetFdFromPtr(fdptr), offset, flag);
}

static int _gdbHioDevRename(struct _reent *r, const char *oldpath, const char *newpath)
{
	(void)r;
	return gdbHioRename(oldpath, newpath);
}

static int _gdbHioDevUnlink(struct _reent *r, const char *pathname)
{
	(void)r;
	return gdbHioUnlink(pathname);
}

static int _gdbHioDevStat(struct _reent *r, const char *pathname, struct stat *st)
{
	(void)r;
	return gdbHioStat(pathname, st);
}

static int _gdbHioDevFstat(struct _reent *r, void *fdptr, struct stat *st)
{
	(void)r;
	return gdbHioFstat(_gdbHioGetFdFromPtr(fdptr), st);
}

static int _gdbHioDevImportFd(int gdbFd)
{
	int fd, dev;

	dev = FindDevice("gdbhio:");
	if(dev == -1)
		return -1;

	fd = __alloc_handle(dev);
	if(fd == -1)
		return -1;

	*(int *)__get_handle(fd)->fileStruct = gdbFd;

	return fd;
}

static const devoptab_t g_gdbHioDevoptab = {
	.name = "gdbhio",
	.structSize   = sizeof(int),
	.open_r       = _gdbHioDevOpen,
	.close_r      = _gdbHioDevClose,
	.write_r      = _gdbHioDevWrite,
	.read_r       = _gdbHioDevRead,
	.seek_r       = _gdbHioDevLseek,
	.fstat_r      = _gdbHioDevFstat,
	.stat_r       = _gdbHioDevStat,
	.link_r       = NULL,
	.unlink_r     = _gdbHioDevUnlink,
	.chdir_r      = NULL,
	.rename_r     = _gdbHioDevRename,
	.mkdir_r      = NULL,
	.dirStateSize = 0,
	.diropen_r    = NULL,
	.dirreset_r   = NULL,
	.dirnext_r    = NULL,
	.dirclose_r   = NULL,
	.statvfs_r    = NULL,
	.ftruncate_r  = NULL,
	.fsync_r      = NULL,
	.deviceData   = 0,
	.chmod_r      = NULL,
	.fchmod_r     = NULL,
	.rmdir_r      = NULL,
};

int gdbHioDevGettimeofday(struct timeval *tv, void *tz)
{
	return gdbHioGettimeofday(tv, tz);
}

int gdbHioDevIsatty(int fd)
{
	return gdbHioIsatty(_gdbHioGetFd(fd));
}

int gdbHioDevSystem(const char *command)
{
	return gdbHioSystem(command);
}

int gdbHioDevInit(void)
{
	int dev = FindDevice("gdbhio:");
	if (dev >= 0) {
		return -1;
	}

	dev = AddDevice(&g_gdbHioDevoptab);
	return dev >= 0 ? 0 : -1;
}

void gdbHioDevExit(void)
{
	RemoveDevice("gdbhio:");
}

int gdbHioDevGetStdin(void)
{
	return _gdbHioDevImportFd(GDBHIO_STDIN_FILENO);
}

int gdbHioDevGetStdout(void)
{
	return _gdbHioDevImportFd(GDBHIO_STDOUT_FILENO);
}

int gdbHioDevGetStderr(void)
{
	return _gdbHioDevImportFd(GDBHIO_STDERR_FILENO);
}

int gdbHioDevRedirectStdStreams(bool in, bool out, bool err)
{
	int ret = 0;
	if (in && (ret = dup2(gdbHioDevGetStdin(), STDIN_FILENO) < 0)) return ret;
	if (out && (ret = dup2(gdbHioDevGetStdout(), STDOUT_FILENO) < 0)) return ret;
	if (err && (ret = dup2(gdbHioDevGetStderr(), STDERR_FILENO) < 0)) return ret;
	return ret;
}
