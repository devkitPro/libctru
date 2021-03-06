#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
	int ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0) {
		errno = -sockfd;
		return -1;
	}

	cmdbuf[0] = IPC_MakeHeader(0x12,4,4); // 0x120104
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)level;
	cmdbuf[3] = (u32)optname;
	cmdbuf[4] = (u32)optlen;
	cmdbuf[5] = IPC_Desc_CurProcessId();
	cmdbuf[7] = IPC_Desc_StaticBuffer(optlen,9);
	cmdbuf[8] = (u32)optval;

	ret = svcSendSyncRequest(SOCU_handle);
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

	return ret;
}
