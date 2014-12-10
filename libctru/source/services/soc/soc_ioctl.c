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
		if(value == NULL) {
			errno = EFAULT;
			ret = -1;
		}

		flags = fcntl(fd, F_GETFL, 0);
		if(flags == -1) {
			errno = SOC_GetErrno();
			va_end(ap);
			return -1;
		}

		if(*value) ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		else       ret = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

		if(ret != 0)
			errno = SOC_GetErrno();

		break;

	default:
		errno = ENOTTY;
		ret = -1;
		break;
	}

	va_end(ap);

	return ret;
}
