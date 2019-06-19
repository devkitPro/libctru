#include <3ds/gdbhio_dev.h>
#include <3ds/gdbhio.h>

#include <sys/iosupport.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

static int g_gdbHioStdinFd = -1, g_gdbHioStdoutFd = -1, g_gdbHioStderrFd = -1;

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

static inline const char *_gdbHioSkipMountpoint(const char *pathname)
{
	return strncmp(pathname, "gdbhio:", 7) == 0 ? pathname + 7 : pathname;
}

static int _gdbHioDevOpen(struct _reent *r, void *fdptr, const char *pathname, int flags, int mode)
{
	(void)r;

	pathname = _gdbHioSkipMountpoint(pathname);
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

	int fd = _gdbHioGetFdFromPtr(fdptr);
	if (fd == g_gdbHioStdinFd)
		g_gdbHioStdinFd = -1;
	else if (fd == g_gdbHioStdoutFd)
		g_gdbHioStdoutFd = -1;
	else if (fd == g_gdbHioStderrFd)
		g_gdbHioStderrFd = -1;
	else
		return gdbHioClose(fd);

	return 0;
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
	return gdbHioRename(_gdbHioSkipMountpoint(oldpath), _gdbHioSkipMountpoint(newpath));
}

static int _gdbHioDevUnlink(struct _reent *r, const char *pathname)
{
	(void)r;
	return gdbHioUnlink(_gdbHioSkipMountpoint(pathname));
}

static int _gdbHioDevStat(struct _reent *r, const char *pathname, struct stat *st)
{
	(void)r;
	return gdbHioStat(_gdbHioSkipMountpoint(pathname), st);
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
	if (dev < 0) return -1;

	return 0;
}

void gdbHioDevExit(void)
{
	close(g_gdbHioStdinFd);
	close(g_gdbHioStdoutFd);
	close(g_gdbHioStderrFd);
	RemoveDevice("gdbhio:");
}

int gdbHioDevGetStdin(void)
{
	if (g_gdbHioStdinFd < 0)
		g_gdbHioStdinFd = _gdbHioDevImportFd(GDBHIO_STDIN_FILENO);

	return g_gdbHioStdinFd;
}

int gdbHioDevGetStdout(void)
{
	if (g_gdbHioStdoutFd < 0)
		g_gdbHioStdoutFd = _gdbHioDevImportFd(GDBHIO_STDOUT_FILENO);

	return g_gdbHioStdoutFd;
}

int gdbHioDevGetStderr(void)
{
	if (g_gdbHioStderrFd < 0)
		g_gdbHioStderrFd = _gdbHioDevImportFd(GDBHIO_STDERR_FILENO);

	return g_gdbHioStderrFd;
}

int gdbHioDevRedirectStdStreams(bool in, bool out, bool err)
{
	int ret = 0;
	if (in) {
		if (gdbHioDevGetStdin() < 0) return -1;
		ret = dup2(g_gdbHioStdinFd, STDIN_FILENO);
		if (ret < 0) return -1;
		if (ret != g_gdbHioStdinFd) {
			close(g_gdbHioStdinFd);
			g_gdbHioStdinFd = STDIN_FILENO;
		}
	}

	if (out) {
		if (gdbHioDevGetStdout() < 0) return -2;
		ret = dup2(g_gdbHioStdoutFd, STDOUT_FILENO);
		if (ret < 0) return -2;
		if (ret != g_gdbHioStdoutFd) {
			close(g_gdbHioStdoutFd);
			g_gdbHioStdoutFd = STDOUT_FILENO;
		}
	}

	if (err) {
		if (gdbHioDevGetStderr() < 0) return -3;
		ret = dup2(g_gdbHioStderrFd, STDERR_FILENO);
		if (ret < 0) return -3;
		if (ret != g_gdbHioStderrFd) {
			close(g_gdbHioStderrFd);
			g_gdbHioStderrFd = STDERR_FILENO;
		}
	}

	return 0;
}
