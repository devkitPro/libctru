#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = 0;
	int tmp_addrlen = 0x1c;
	int fd, dev;
	__handle *handle;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[0x1c];
	u32 saved_threadstorage[2];

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0) {
		errno = -sockfd;
		return -1;
	}

	dev = FindDevice("soc:");
	if(dev < 0) {
		errno = ENODEV;
		return -1;
	}

	fd = __alloc_handle(sizeof(__handle) + sizeof(Handle));
	if(fd < 0) {
		errno = ENOMEM;
		return -1;
	}

	handle = __get_handle(fd);
	handle->device = dev;
	handle->fileStruct = ((void *)handle) + sizeof(__handle);

	memset(tmpaddr, 0, 0x1c);

	cmdbuf[0] = 0x00040082;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)tmp_addrlen;
	cmdbuf[3] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (tmp_addrlen<<14) | 2;
	cmdbuf[0x104>>2] = (u32)tmpaddr;

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		__release_handle(fd);
		errno = SYNC_ERROR;
		return ret;
	}

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret = _net_convert_error(cmdbuf[2]);

	if(ret < 0)
		errno = -ret;

	if(ret >= 0 && addr != NULL) {
		addr->sa_family = tmpaddr[1];
		if(*addrlen > tmpaddr[0])
			*addrlen = tmpaddr[0];
		memcpy(addr->sa_data, &tmpaddr[2], *addrlen - 2);
	}

	if(ret < 0) {
		__release_handle(fd);
		return -1;
	}
	else
		*(Handle*)handle->fileStruct = ret;

	return fd;
}
