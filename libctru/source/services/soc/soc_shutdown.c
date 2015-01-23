#include "soc_common.h"
#include <sys/socket.h>

int shutdown(int sockfd, int shutdown_type)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0)
	{
		SOCU_errno = sockfd;
		return -1;
	}

	cmdbuf[0] = 0x000C0082;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)shutdown_type;
	cmdbuf[3] = 0x20;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	SOCU_errno = ret;

	if(ret!=0)return -1;
	return 0;
}
