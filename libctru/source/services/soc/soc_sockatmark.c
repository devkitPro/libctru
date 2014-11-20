#include "soc_common.h"
#include <sys/socket.h>

int sockatmark(int sockfd)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00150042;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = 0x20;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}
