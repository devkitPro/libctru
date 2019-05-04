/**
 * @file gdbhio.h
 * @brief Luma3DS GDB HIO (called File I/O in GDB documentation) functions.
 */

#pragma once

#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>

#define GDBHIO_STDIN_FILENO    0
#define GDBHIO_STDOUT_FILENO   1
#define GDBHIO_STDERR_FILENO   2

int gdbHioOpen(const char *pathname, int flags, mode_t mode);
int gdbHioClose(int fd);
int gdbHioRead(int fd, void *buf, unsigned int count);
int gdbHioWrite(int fd, const void *buf, unsigned int count);
off_t gdbHioLseek(int fd, off_t offset, int flag);
int gdbHioRename(const char *oldpath, const char *newpath);
int gdbHioUnlink(const char *pathname);
int gdbHioStat(const char *pathname, struct stat *st);
int gdbHioFstat(int fd, struct stat *st);
int gdbHioGettimeofday(struct timeval *tv, void *tz);
int gdbHioIsatty(int fd);

///< Host I/O 'system' function, requires 'set remote system-call-allowed 1'.
int gdbHioSystem(const char *command);
