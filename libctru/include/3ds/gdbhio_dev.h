/**
 * @file gdbhio_dev.h
 * @brief Luma3DS GDB HIO (called File I/O in GDB documentation) devoptab wrapper.
 */

#pragma once

#include <stdbool.h>

struct timeval;

///< Initializes the GDB HIO devoptab wrapper, returns 0 on success, -1 on failure.
int gdbHioDevInit(void);

///< Deinitializes the GDB HIO devoptab wrapper.
void gdbHioDevExit(void);

///< Returns a file descriptor mapping to the GDB client console's standard input stream.
int gdbHioDevGetStdin(void);

///< Returns a file descriptor mapping to the GDB client console's standard output stream.
int gdbHioDevGetStdout(void);

///< Returns a file descriptor mapping to the GDB client console's standard error stream.
int gdbHioDevGetStderr(void);

///< Redirects 0 to 3 of the application's standard streams to GDB client console's. Returns -1, -2, or -3, resp., on failure; 0 on success.
int gdbHioDevRedirectStdStreams(bool in, bool out, bool err);

///< GDB HIO POSIX function gettimeofday.
int gdbHioDevGettimeofday(struct timeval *tv, void *tz);

///< GDB HIO POSIX function isatty.
int gdbHioDevIsatty(int fd);

///< GDB HIO POSIX function system. Requires 'set remote system-call-allowed 1'.
int gdbHioDevSystem(const char *command);
