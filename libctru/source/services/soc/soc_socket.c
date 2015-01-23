#include "soc_common.h"
#include <errno.h>
#include <sys/iosupport.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol)
{
	int ret=0;
	int fd, dev;
	__handle *handle;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000200C2;
	cmdbuf[1] = domain;
	cmdbuf[2] = type;
	cmdbuf[3] = protocol;
	cmdbuf[4] = 0x20;

	dev = FindDevice("soc:");
	if(dev < 0)
	{
		SOCU_errno = -ENODEV;
		return -1;
	}

	fd = __alloc_handle(sizeof(__handle) + sizeof(Handle));
	if(fd < 0)
	{
		SOCU_errno = -ENOMEM;
		return -1;
	}

	handle = __get_handle(fd);
	handle->device = dev;
	handle->fileStruct = ((void *)handle) + sizeof(__handle);

	if((ret = svcSendSyncRequest(SOCU_handle)) != 0)
	{
		__release_handle(fd);
		return ret;
	}

	ret = (int)cmdbuf[1];
	if(ret != 0)
	{
		SOCU_errno = _net_convert_error(cmdbuf[2]);
		__release_handle(fd);
		return -1;
	}

	*(Handle*)handle->fileStruct = cmdbuf[2];
	return fd;
}
