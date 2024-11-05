#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];
	u8 tmpaddr[ADDR_STORAGE_LEN];

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0)	{
		errno = -sockfd;
		return -1;
	}

	memset(tmpaddr, 0, ADDR_STORAGE_LEN);

	cmdbuf[0] = IPC_MakeHeader(0x17,2,2); // 0x170082
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = ADDR_STORAGE_LEN;
	cmdbuf[3] = IPC_Desc_CurProcessId();

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(ADDR_STORAGE_LEN,0);
	staticbufs[1] = (u32)tmpaddr;

	ret = svcSendSyncRequest(SOCU_handle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret = _net_convert_error(cmdbuf[2]);

	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	addr->sa_family = tmpaddr[1];

	socklen_t user_addrlen = tmpaddr[0];
	if(addr->sa_family == AF_INET)
		user_addrlen += 8;

	if(*addrlen > user_addrlen)
		*addrlen = user_addrlen;
	memcpy(addr->sa_data, &tmpaddr[2], *addrlen - sizeof(addr->sa_family));

	return ret;
}
