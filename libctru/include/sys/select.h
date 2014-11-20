#pragma once

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

	int	select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

#ifdef __cplusplus
}
#endif
