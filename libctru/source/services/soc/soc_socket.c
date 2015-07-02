#include "soc_common.h"
#include <errno.h>
#include <sys/iosupport.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol)
{
	int ret = 0;
	int fd, dev;
	__handle *handle;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000200C2;
	cmdbuf[1] = domain;
	cmdbuf[2] = type;
	cmdbuf[3] = protocol;
	cmdbuf[4] = 0x20;

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

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0)
	{
		__release_handle(fd);
		errno = SYNC_ERROR;
		return ret;
	}

	ret = (int)cmdbuf[1];
	if(ret == 0)ret = cmdbuf[2];
	if(ret < 0) {
		__release_handle(fd);
		if(cmdbuf[1] == 0)errno = _net_convert_error(ret);
		if(cmdbuf[1] != 0)errno = SYNC_ERROR;
		return -1;
	}

	*(Handle*)handle->fileStruct = cmdbuf[2];
	return fd;
}
