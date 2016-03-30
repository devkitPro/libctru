#include "soc_common.h"
#include <3ds/ipc.h>
#include <3ds/result.h>

int SOCU_AddGlobalSocket(int sockfd)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0) {
		errno = -sockfd;
		return -1;
	}

	cmdbuf[0] = IPC_MakeHeader(0x23,1,0); // 0x230040
	cmdbuf[1] = (u32)sockfd;

	int ret = svcSendSyncRequest(SOCU_handle);
	if(R_FAILED(ret))return ret;
	return cmdbuf[1];
}
