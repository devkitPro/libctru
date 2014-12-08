#pragma once

#include <3ds/types.h>

#define POLLIN		0x01
#define POLLPRI		0x02
#define POLLOUT		0x10
#define POLLERR		0x00 // unknown ???
#define POLLHUP		0x00 // unknown ???
#define POLLNVAL	0x20

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
