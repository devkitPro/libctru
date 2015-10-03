#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];
	u8 tmpaddr[0x1c];

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0)	{
		errno = -sockfd;
		return -1;
	}

	cmdbuf[0] = IPC_MakeHeader(0x17,2,2); // 0x170082
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = 0x1c;
	cmdbuf[3] = IPC_Desc_CurProcessHandle();

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(0x1c,0);
	staticbufs[1] = (u32)tmpaddr;

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret = _net_convert_error(cmdbuf[2]);

	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	if(*addrlen > tmpaddr[0])
		*addrlen = tmpaddr[0];
	memset(addr, 0, sizeof(struct sockaddr));
	addr->sa_family = tmpaddr[1];
	memcpy(addr->sa_data, &tmpaddr[2], *addrlen - 2);

	return ret;
}
