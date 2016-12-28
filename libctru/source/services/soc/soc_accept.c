#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = 0;
	int tmp_addrlen = 0x1c;
	int fd, dev;
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

	fd = __alloc_handle(dev);
	if(fd < 0) return fd;

	memset(tmpaddr, 0, 0x1c);

	cmdbuf[0] = IPC_MakeHeader(0x4,2,2); // 0x40082
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)tmp_addrlen;
	cmdbuf[3] = IPC_Desc_CurProcessHandle();

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(tmp_addrlen,0);
	staticbufs[1] = (u32)tmpaddr;

	ret = svcSendSyncRequest(SOCU_handle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if(ret != 0) {
		__release_handle(fd);
		errno = SYNC_ERROR;
		return ret;
	}

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
	{
		__handle *handle = __get_handle(fd);
		*(Handle*)handle->fileStruct = ret;
	}

	return fd;
}
