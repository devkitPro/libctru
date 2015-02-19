#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

int closesocket(int sockfd)
{
	int fd = soc_get_fd(sockfd);
	if(fd < 0) {
		errno = -fd;
		return -1;
	}

	return close(sockfd);
}
