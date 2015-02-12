#include "soc_common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ioctl.h>

int ioctl(int sockfd, int request, ...)
{
	int ret;
	int flags;
	int *value;
	va_list ap;

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0) {
		errno = -sockfd;
		return -1;
	}

	switch(request) {
	case FIONBIO:
		va_start(ap, request);
		value = va_arg(ap, int*);
		va_end(ap);

		if(value == NULL) {
			errno = EFAULT;
			return -1;
		}

		flags = fcntl(sockfd, F_GETFL, 0);
		if(flags == -1)	
			return -1;

		if(*value)
			ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
		else
			ret = fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK);

		break;

	default:
		errno = ENOTTY;
		ret = -1;
		break;
	}

	return ret;
}
