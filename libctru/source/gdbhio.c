#include <3ds/gdbhio.h>

#include <3ds/svc.h>
#include <3ds/result.h>

#include <string.h>
#include <errno.h>

#include <stdarg.h>
#include <stdio.h>

#define GDBHIO_O_RDONLY           0x0
#define GDBHIO_O_WRONLY           0x1
#define GDBHIO_O_RDWR             0x2
#define GDBHIO_O_ACCMODE          0x3
#define GDBHIO_O_APPEND           0x8
#define GDBHIO_O_CREAT          0x200
#define GDBHIO_O_TRUNC          0x400
#define GDBHIO_O_EXCL           0x800
#define GDBHIO_O_SUPPORTED  (GDBHIO_O_RDONLY | GDBHIO_O_WRONLY| \
                             GDBHIO_O_RDWR   | GDBHIO_O_APPEND| \
                             GDBHIO_O_CREAT  | GDBHIO_O_TRUNC| \
                             GDBHIO_O_EXCL)

#define GDBHIO_S_IFREG        0100000
#define GDBHIO_S_IFDIR         040000
#define GDBHIO_S_IFCHR         020000
#define GDBHIO_S_IRUSR           0400
#define GDBHIO_S_IWUSR           0200
#define GDBHIO_S_IXUSR           0100
#define GDBHIO_S_IRWXU           (GDBHIO_S_IRUSR | GDBHIO_S_IWUSR | GDBHIO_S_IXUSR)
#define GDBHIO_S_IRGRP            040
#define GDBHIO_S_IWGRP            020
#define GDBHIO_S_IXGRP            010
#define GDBHIO_S_IRWXG            (GDBHIO_S_IRGRP | GDBHIO_S_IWGRP | GDBHIO_S_IXGRP)
#define GDBHIO_S_IROTH             04
#define GDBHIO_S_IWOTH             02
#define GDBHIO_S_IXOTH             01
#define GDBHIO_S_IRWXO             (GDBHIO_S_IROTH | GDBHIO_S_IWOTH | GDBHIO_S_IXOTH)
#define GDBHIO_S_SUPPORTED         (GDBHIO_S_IFREG|GDBHIO_S_IFDIR|  \
                                    GDBHIO_S_IRWXU|GDBHIO_S_IRWXG|  \
                                    GDBHIO_S_IRWXO)

#define GDBHIO_SEEK_SET             0
#define GDBHIO_SEEK_CUR             1
#define GDBHIO_SEEK_END             2

#define GDBHIO_EPERM                1
#define GDBHIO_ENOENT               2
#define GDBHIO_EINTR                4
#define GDBHIO_EIO                  5
#define GDBHIO_EBADF                9
#define GDBHIO_EACCES              13
#define GDBHIO_EFAULT              14
#define GDBHIO_EBUSY               16
#define GDBHIO_EEXIST              17
#define GDBHIO_ENODEV              19
#define GDBHIO_ENOTDIR             20
#define GDBHIO_EISDIR              21
#define GDBHIO_EINVAL              22
#define GDBHIO_ENFILE              23
#define GDBHIO_EMFILE              24
#define GDBHIO_EFBIG               27
#define GDBHIO_ENOSPC              28
#define GDBHIO_ESPIPE              29
#define GDBHIO_EROFS               30
#define GDBHIO_ENOSYS              88
#define GDBHIO_ENAMETOOLONG        91
#define GDBHIO_EUNKNOWN          9999

typedef struct PackedGdbHioRequest
{
	char magic[4]; // "GDB\x00"
	u32 version;

	// Request
	char functionName[16+1];
	char paramFormat[8+1];

	u64 parameters[8];
	size_t stringLengths[8];

	// Return
	s64 retval;
	int gdbErrno;
	bool ctrlC;
} PackedGdbHioRequest;

static __thread bool g_gdbHioWasInterruptedByCtrlC = false;

bool gdbHioWasInterruptedByCtrlC(void)
{
	return g_gdbHioWasInterruptedByCtrlC;
}

static int _gdbHioImportErrno(int errnum)
{
	switch (errnum) {
		case GDBHIO_EPERM: return EPERM;
		case GDBHIO_ENOENT: return ENOENT;
		case GDBHIO_EINTR: return EINTR;
		case GDBHIO_EIO: return EIO;
		case GDBHIO_EBADF: return EBADF;
		case GDBHIO_EACCES: return EACCES;
		case GDBHIO_EFAULT: return EFAULT;
		case GDBHIO_EBUSY: return EBUSY;
		case GDBHIO_EEXIST: return EEXIST;
		case GDBHIO_ENODEV: return ENODEV;
		case GDBHIO_ENOTDIR: return ENOTDIR;
		case GDBHIO_EISDIR: return EISDIR;
		case GDBHIO_EINVAL: return EINVAL;
		case GDBHIO_ENFILE: return ENFILE;
		case GDBHIO_EMFILE: return EMFILE;
		case GDBHIO_EFBIG: return EFBIG;
		case GDBHIO_ENOSPC: return ENOSPC;
		case GDBHIO_ESPIPE: return ESPIPE;
		case GDBHIO_EROFS: return EROFS;
		case GDBHIO_ENOSYS: return ENOSYS;
		case GDBHIO_ENAMETOOLONG: return ENAMETOOLONG;
		default: return EPIPE;
	}
}

static int _gdbHioExportOpenFlags(int flags)
{
	int outflags = 0;
	if (flags & O_CREAT) outflags |= GDBHIO_O_CREAT;
	if (flags & O_EXCL) outflags |= GDBHIO_O_EXCL;
	if (flags & O_TRUNC) outflags |= GDBHIO_O_TRUNC;
	if (flags & O_APPEND) outflags |= GDBHIO_O_APPEND;

	switch (flags & O_ACCMODE) {
	case O_RDONLY:
		outflags |= GDBHIO_O_RDONLY;
		break;
	case O_WRONLY:
		outflags |= GDBHIO_O_WRONLY;
		break;
	case O_RDWR:
		outflags |= GDBHIO_O_RDWR;
		break;
	default:
		break;
	}

	// Note: O_BINARY is implicit if the host supports it

	return outflags;
}

typedef s32 gdbhio_mode_t;

static mode_t _gdbHioImportFileMode(gdbhio_mode_t gdbMode)
{
	mode_t mode = 0;
	if (mode & ~GDBHIO_S_SUPPORTED) return -1;

	if (gdbMode & GDBHIO_S_IFREG) mode |= S_IFREG;
	if (gdbMode & GDBHIO_S_IFDIR) mode |= S_IFDIR;
	if (gdbMode & GDBHIO_S_IFCHR) mode |= S_IFCHR;
	if (gdbMode & GDBHIO_S_IRUSR) mode |= S_IRUSR;
	if (gdbMode & GDBHIO_S_IWUSR) mode |= S_IWUSR;
	if (gdbMode & GDBHIO_S_IXUSR) mode |= S_IXUSR;
	if (gdbMode & GDBHIO_S_IRGRP) mode |= S_IRGRP;
	if (gdbMode & GDBHIO_S_IWGRP) mode |= S_IWGRP;
	if (gdbMode & GDBHIO_S_IXGRP) mode |= S_IXGRP;
	if (gdbMode & GDBHIO_S_IROTH) mode |= S_IROTH;
	if (gdbMode & GDBHIO_S_IWOTH) mode |= S_IWOTH;
	if (gdbMode & GDBHIO_S_IXOTH) mode |= S_IXOTH;

  return mode;
}

static int _gdbHioExportFileMode(mode_t mode)
{
	gdbhio_mode_t gdbMode = 0;
	if (mode & ~GDBHIO_S_SUPPORTED) return -1;

	if (mode & S_IFREG) gdbMode |= GDBHIO_S_IFREG;
	if (mode & S_IFDIR) gdbMode |= GDBHIO_S_IFDIR;
	if (mode & S_IFCHR) gdbMode |= GDBHIO_S_IFCHR;
	if (mode & S_IRUSR) gdbMode |= GDBHIO_S_IRUSR;
	if (mode & S_IWUSR) gdbMode |= GDBHIO_S_IWUSR;
	if (mode & S_IXUSR) gdbMode |= GDBHIO_S_IXUSR;
	if (mode & S_IRGRP) gdbMode |= GDBHIO_S_IRGRP;
	if (mode & S_IWGRP) gdbMode |= GDBHIO_S_IWGRP;
	if (mode & S_IXGRP) gdbMode |= GDBHIO_S_IXGRP;
	if (mode & S_IROTH) gdbMode |= GDBHIO_S_IROTH;
	if (mode & S_IWOTH) gdbMode |= GDBHIO_S_IWOTH;
	if (mode & S_IXOTH) gdbMode |= GDBHIO_S_IXOTH;

	return mode;
}


static int _gdbExportSeekFlag(int flag)
{
	switch (flag) {
		case SEEK_SET: return GDBHIO_SEEK_SET;
		case SEEK_CUR: return GDBHIO_SEEK_CUR;
		case SEEK_END: return GDBHIO_SEEK_END;
		default: return GDBHIO_SEEK_SET;
	}
}
// https://sourceware.org/gdb/onlinedocs/gdb/struct-stat.html#struct-stat
typedef u32 gdbhio_time_t;

struct CTR_PACKED CTR_ALIGN(4) gdbhio_stat {
	u32  gst_dev;               /* device */
	u32  gst_ino;               /* inode */
	gdbhio_mode_t gst_mode;     /* protection */
	u32 gst_nlink;              /* number of hard links */
	u32 gst_uid;                /* user ID of owner */
	u32 gst_gid;                /* group ID of owner */
	u32 gst_rdev;               /* device type (if inode device) */
	u64 gst_size;               /* total size, in bytes */
	u64 gst_blksize;            /* blocksize for filesystem I/O */
	u64 gst_blocks;             /* number of blocks allocated */
	gdbhio_time_t gst_atime;    /* time of last access */
	gdbhio_time_t gst_mtime;    /* time of last modification */
	gdbhio_time_t gst_ctime;    /* time of last change */
};

static inline u32 _gdbHioImportScalar32(u32 v)
{
	return __builtin_bswap32(v);
}

static inline u64 _gdbHioImportScalar64(u64 v)
{
	return __builtin_bswap64(v);
}

static void _gdbHioImportStructStat(struct stat *out, const struct gdbhio_stat *in)
{
	memset(out, 0, sizeof(struct stat));
	out->st_dev = _gdbHioImportScalar32(in->gst_dev);
	out->st_ino = _gdbHioImportScalar32(in->gst_ino);
	out->st_mode = _gdbHioImportFileMode(_gdbHioImportScalar32(in->gst_mode));
	out->st_nlink = _gdbHioImportScalar32(in->gst_nlink);
	out->st_uid = _gdbHioImportScalar32(in->gst_uid);
	out->st_gid = _gdbHioImportScalar32(in->gst_gid);
	out->st_rdev = _gdbHioImportScalar32(in->gst_rdev);
	out->st_size = (off_t)_gdbHioImportScalar64(in->gst_size);
	out->st_blksize = (blksize_t)_gdbHioImportScalar64(in->gst_blksize);
	out->st_blocks = (blkcnt_t)_gdbHioImportScalar64(in->gst_blocks);
	out->st_atime = _gdbHioImportScalar32(in->gst_atime);
	out->st_mtime = _gdbHioImportScalar32(in->gst_mtime);
	out->st_ctime = _gdbHioImportScalar32(in->gst_ctime);
}

struct gdbhio_timeval {
	gdbhio_time_t tv_sec;  /* second */
	u64   tv_usec; /* microsecond */
};

static void _gdbHioImportStructTimeval(struct timeval *out, const struct gdbhio_timeval *in)
{
	out->tv_sec = _gdbHioImportScalar32(in->tv_sec);
	out->tv_usec = _gdbHioImportScalar64(in->tv_usec);
}

static void _gdbHioSetErrno(int gdbErrno, bool ctrlC)
{
	if (gdbErrno != 0) {
		errno = _gdbHioImportErrno(gdbErrno);
	}
	g_gdbHioWasInterruptedByCtrlC = ctrlC;
}

static s64 _gdbHioSendSyncRequest64V(const char *name, const char *paramFormat, va_list args)
{
	PackedGdbHioRequest req = {{0}};
	memcpy(req.magic, "GDB", 4);
	strncpy(req.functionName, name, 16);
	strncpy(req.paramFormat, paramFormat, 8);

	u32 numStrs = 0;
	const char *str;

	for (u32 i = 0; i < 8 && paramFormat[i] != 0; i++) {
		switch (paramFormat[i]) {
			case 'i':
			case 'I':
				req.parameters[i] = va_arg(args, u32);
				break;
			case 'l':
			case 'L':
				req.parameters[i] = va_arg(args, u64);
				break;
			case 'p':
				req.parameters[i] = va_arg(args, uintptr_t);
				break;
			case 's':
				str = va_arg(args, const char *);
				req.parameters[i] = (uintptr_t)str;
				req.stringLengths[numStrs++] = strlen(str)+1;
				break;
			default:
				break;
		}
	}

	if (R_FAILED(svcOutputDebugString((const char *)&req, 0)) || req.paramFormat[0] != 0) {
		errno = EPIPE;
		g_gdbHioWasInterruptedByCtrlC = false;
		return -1;
	}

	_gdbHioSetErrno(req.gdbErrno, req.ctrlC);
	return req.retval;
}

static s64 _gdbHioSendSyncRequest64(const char *name, const char *paramFormat, ...)
{
	s64 ret = 0;
	va_list args;
	va_start(args, paramFormat);
	ret = _gdbHioSendSyncRequest64V(name, paramFormat, args);
	va_end(args);
	return ret;
}

static int _gdbHioSendSyncRequest(const char *name, const char *paramFormat, ...)
{
	s64 ret = 0;
	va_list args;
	va_start(args, paramFormat);
	ret = _gdbHioSendSyncRequest64V(name, paramFormat, args);
	va_end(args);
	return (int)ret;
}

int gdbHioOpen(const char *pathname, int flags, mode_t mode)
{
	return _gdbHioSendSyncRequest("open", "siI", pathname, _gdbHioExportOpenFlags(flags), _gdbHioExportFileMode(mode));
}

int gdbHioClose(int fd)
{
	return _gdbHioSendSyncRequest("close", "i", fd);
}

int gdbHioRead(int fd, void *buf, unsigned int count)
{
	return _gdbHioSendSyncRequest("read", "ipI", fd, buf, count);
}

int gdbHioWrite(int fd, const void *buf, unsigned int count)
{
	return _gdbHioSendSyncRequest("write", "ipI", fd, buf, count);
}

off_t gdbHioLseek(int fd, off_t offset, int flag)
{
	return _gdbHioSendSyncRequest64("lseek", "ili", fd, offset, _gdbExportSeekFlag(flag));
}

int gdbHioRename(const char *oldpath, const char *newpath)
{
	return _gdbHioSendSyncRequest("rename", "ss", oldpath, newpath);
}

int gdbHioUnlink(const char *pathname)
{
	return _gdbHioSendSyncRequest("unlink", "s", pathname);
}

int gdbHioStat(const char *pathname, struct stat *st)
{
	struct gdbhio_stat gst;
	int ret = _gdbHioSendSyncRequest("stat", "sp", pathname, &gst);
	_gdbHioImportStructStat(st, &gst);
	return ret;
}

int gdbHioFstat(int fd, struct stat *st)
{
	struct gdbhio_stat gst;
	int ret = _gdbHioSendSyncRequest("fstat", "ip", fd, &gst);
	_gdbHioImportStructStat(st, &gst);
	return ret;
}

int gdbHioGettimeofday(struct timeval *tv, void *tz)
{
	// GDB ignores tz and passes NULL
	struct gdbhio_timeval gtv;
	int ret = _gdbHioSendSyncRequest("gettimeofday", "pp", &gtv, tz);
	_gdbHioImportStructTimeval(tv, &gtv);
	return ret;
}

int gdbHioIsatty(int fd)
{
	return _gdbHioSendSyncRequest("isatty", "i", fd);
}

// Requires set remote system-call-allowed 1
int gdbHioSystem(const char *command)
{
	return _gdbHioSendSyncRequest("system", "s", command);
}
