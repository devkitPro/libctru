#pragma once

#define POLLIN          0x01
#define POLLPRI         0x02
#define POLLWRNORM      0x08
#define POLLWRBAND      0x10
#define POLLNVAL        0x20
#define POLLHUP         0x00 // Unknown?
#define POLLERR         0x00 // Unknown?
#define POLLOUT         POLLWRNORM

typedef unsigned int nfds_t;

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
