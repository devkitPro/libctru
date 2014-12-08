#pragma once

#include <3ds/types.h>

#define POLLIN		0x01
#define POLLPRI		0x02
#define POLLHUP		0x04 // unknown ???
#define POLLERR		0x08 // probably
#define POLLOUT		0x10
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
