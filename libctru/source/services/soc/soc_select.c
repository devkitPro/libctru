#include "soc_common.h"
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	struct pollfd *pollinfo;
	nfds_t numfds = 0;
	size_t i, j;
	int rc, found;

	for(i = 0; i < nfds; ++i) {
		if((readfds && FD_ISSET(i, readfds))
		|| (writefds && FD_ISSET(i, writefds))
		|| (exceptfds && FD_ISSET(i, exceptfds)))
			++numfds;
	}

	pollinfo = (struct pollfd*)calloc(numfds, sizeof(struct pollfd));
	if(pollinfo == NULL) {
		errno = ENOMEM;
		return -1;
	}

	for(i = 0, j = 0; i < nfds; ++i) {
		if((readfds && FD_ISSET(i, readfds))
		|| (writefds && FD_ISSET(i, writefds))
		|| (exceptfds && FD_ISSET(i, exceptfds))) {
			pollinfo[j].fd      = i;
			pollinfo[j].events  = 0;
			pollinfo[j].revents = 0;

			if(readfds && FD_ISSET(i, readfds))
				pollinfo[j].events |= POLLIN;
			if(writefds && FD_ISSET(i, writefds))
				pollinfo[j].events |= POLLOUT;

			++j;
		}
	}

	if(timeout)
		rc = poll(pollinfo, numfds, timeout->tv_sec*1000 + timeout->tv_usec/1000);
	else
		rc = poll(pollinfo, numfds, -1);

	if(rc < 0) {
		free(pollinfo);
		return rc;
	}

	for(i = 0, j = 0, rc = 0; i < nfds; ++i) {
		found = 0;

		if((readfds && FD_ISSET(i, readfds))
		|| (writefds && FD_ISSET(i, writefds))
		|| (exceptfds && FD_ISSET(i, exceptfds))) {

			if(readfds && FD_ISSET(i, readfds)) {
				if(pollinfo[j].events & (POLLIN|POLLHUP))
					found = 1;
				else
					FD_CLR(i, readfds);
			}

			if(writefds && FD_ISSET(i, writefds)) {
				if(pollinfo[j].events & (POLLOUT|POLLHUP))
					found = 1;
				else
					FD_CLR(i, writefds);
			}

			if(exceptfds && FD_ISSET(i, exceptfds)) {
				if(pollinfo[j].events & POLLERR)
					found = 1;
				else
					FD_CLR(i, exceptfds);
			}

			if(found)
				++rc;
			++j;
		}
	}

	free(pollinfo);

	return rc;
}
