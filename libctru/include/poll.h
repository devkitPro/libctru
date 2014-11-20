#pragma once

#include <3ds/types.h>

/* only POLLIN confirmed to work so far */
#define POLLIN		0x001
#define POLLPRI		0x002
#define POLLOUT		0x004
#define POLLERR		0x008
#define POLLHUP		0x010
#define POLLNVAL	0x020
#define POLLRDNORM	0x040
#define POLLRDBAND	0x080
#define POLLWRNORM	0x100
#define POLLWRBAND	0x200

typedef u32 nfds_t;

struct pollfd
{
	int	fd;
	int	events;
	int	revents;
};

#ifdef __cplusplus
extern "C" {
#endif

	int	poll(struct pollfd *fds, nfds_t nfsd, int timeout);

#ifdef __cplusplus
}
#endif
