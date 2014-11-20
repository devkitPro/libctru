#include "soc_common.h"
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
	return sendto(sockfd, buf, len, flags, NULL, 0);
}
