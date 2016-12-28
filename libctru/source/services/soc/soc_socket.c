#include "soc_common.h"
#include <errno.h>
#include <netinet/in.h>
#include <sys/iosupport.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

int socket(int domain, int type, int protocol)
{
	int ret = 0;
	int fd, dev;
	__handle *handle;
	u32 *cmdbuf = getThreadCommandBuffer();

	// The protocol on the 3DS *must* be 0 to work
	// To that end, when appropriate, we will make the change for the user
	if (domain == AF_INET
	&& type == SOCK_STREAM
	&& protocol == IPPROTO_TCP) {
		protocol = 0; // TCP is the only option, so 0 will work as expected
	}
	if (domain == AF_INET
	&& type == SOCK_DGRAM
	&& protocol == IPPROTO_UDP) {
		protocol = 0; // UDP is the only option, so 0 will work as expected
	}

	cmdbuf[0] = IPC_MakeHeader(0x2,3,2); // 0x200C2
	cmdbuf[1] = domain;
	cmdbuf[2] = type;
	cmdbuf[3] = protocol;
	cmdbuf[4] = IPC_Desc_CurProcessHandle();

	dev = FindDevice("soc:");
	if(dev < 0) {
		errno = ENODEV;
		return -1;
	}

	fd = __alloc_handle(dev);
	if(fd < 0) return fd;

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
