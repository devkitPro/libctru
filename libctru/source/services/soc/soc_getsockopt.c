#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
	int ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0) {
		errno = -sockfd;
		return -1;
	}

	cmdbuf[0] = IPC_MakeHeader(0x11,4,2); // 0x110102
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)level;
	cmdbuf[3] = (u32)optname;
	cmdbuf[4] = (u32)*optlen;
	cmdbuf[5] = IPC_Desc_CurProcessHandle();

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(*optlen,0);
	staticbufs[1] = (u32)optval;

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

	*optlen = cmdbuf[3];

	return ret;
}
