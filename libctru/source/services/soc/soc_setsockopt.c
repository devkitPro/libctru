#include "soc_common.h"
#include <sys/socket.h>

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00120104;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)level;
	cmdbuf[3] = (u32)optname;
	cmdbuf[4] = (u32)optlen;
	cmdbuf[5] = 0x20;
	cmdbuf[7] = (optlen<<14) | 0x2402;
	cmdbuf[8] = (u32)optval;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}
