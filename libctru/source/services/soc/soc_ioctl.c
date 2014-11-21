#include "soc_common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ioctl.h>

int ioctl(int fd, int request, ...)
{
	int ret;
	int flags;
	int *value;
	va_list ap;

	va_start(ap, request);

	switch(request) {
	case FIONBIO:
		value = va_arg(ap, int*);
		if(value == NULL) ret = -1;
		else if(*value) {
			flags = fcntl(fd, F_GETFL, 0);
			ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		}
		else {
			flags = fcntl(fd, F_GETFL, 0);
			ret = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
		}
		break;

	default:
		errno = ENOTTY;
		ret = -1;
		break;
	}

	va_end(ap);

	return ret;
}
