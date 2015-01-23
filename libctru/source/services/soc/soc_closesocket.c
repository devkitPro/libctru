#include "soc_common.h"
#include <sys/socket.h>

int closesocket(int sockfd)
{
	int fd = soc_get_fd(sockfd);
	if(fd < 0)
	{
		SOCU_errno = fd;
		return -1;
	}

	return close(sockfd);
}
