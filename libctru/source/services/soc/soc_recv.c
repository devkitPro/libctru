#include "soc_common.h"
#include <sys/socket.h>

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
	return recvfrom(sockfd, buf, len, flags, NULL, 0);
}
