#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = 0;
	int tmp_addrlen = ADDR_STORAGE_LEN;
	int fd, dev;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[ADDR_STORAGE_LEN];
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

	memset(tmpaddr, 0, ADDR_STORAGE_LEN);

	cmdbuf[0] = IPC_MakeHeader(0x4,2,2); // 0x40082
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)tmp_addrlen;
	cmdbuf[3] = IPC_Desc_CurProcessId();

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

		socklen_t user_addrlen = tmpaddr[0];
		if(addr->sa_family == AF_INET)
		    user_addrlen += 8; // Accounting for the 8 bytes of sin_zero padding, which must be written for compatibility.

		if(*addrlen > user_addrlen)
			*addrlen = user_addrlen;
		memcpy(addr->sa_data, &tmpaddr[2], *addrlen - sizeof(addr->sa_family));
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
