#include "soc_common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

#define O_NONBLOCK_3DS 0x4

#define ALL_3DS   (O_NONBLOCK_3DS)
#define ALL_FLAGS (O_NONBLOCK)

static int from_3ds(int flags)
{
	int newflags = 0;

	if(flags & O_NONBLOCK_3DS)
		newflags |= O_NONBLOCK;
	/* add other flag translations here, but I have only seen O_NONBLOCK */

	return newflags;
}

static int to_3ds(int flags)
{
	int newflags = 0;

	if(flags & O_NONBLOCK)
		newflags |= O_NONBLOCK_3DS;
	/* add other flag translations here, but I have only seen O_NONBLOCK */

	return newflags;
}

int fcntl(int sockfd, int cmd, ...)
{
	int ret = 0;
	int arg = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	va_list args;

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0) {
		errno = -sockfd;
		return -1;
	}

	if(cmd != F_GETFL && cmd != F_SETFL) {
		errno = EINVAL;
		return -1;
	}

	if(cmd == F_SETFL) {
		va_start(args, cmd);
		arg = va_arg(args, int);
		va_end(args);

		/* make sure they only used known flags */
		if(arg & ~ALL_FLAGS) {
			errno = EINVAL;
			return -1;
		}

		arg = to_3ds(arg);
	}

	cmdbuf[0] = 0x001300C2;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)cmd;
	cmdbuf[3] = (u32)arg;
	cmdbuf[4] = 0x20;

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret = _net_convert_error(cmdbuf[2]);

	if(ret < 0) {
		errno = ret;
		return -1;
	}

	if(ret & ~ALL_3DS) {
		/* somehow report unknown flags */
	}

	return from_3ds(ret);
}
