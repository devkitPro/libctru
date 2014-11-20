#include "soc_common.h"
#include <sys/socket.h>

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	cmdbuf[0] = 0x00110102;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)level;
	cmdbuf[3] = (u32)optname;
	cmdbuf[4] = (u32)*optlen;
	cmdbuf[5] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = ((*optlen)<<14) | 2;
	cmdbuf[0x104>>2] = (u32)optval;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret==0)*optlen = cmdbuf[3];

	if(ret<0)return -1;
	return ret;
}
