#include "soc_common.h"
#include <sys/socket.h>

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];
	u8 tmpaddr[0x1c];

	cmdbuf[0] = 0x00170082;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = 0x1c;
	cmdbuf[3] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (0x1c<<14) | 2;
	cmdbuf[0x104>>2] = (u32)tmpaddr;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret==0)
	{
		if(*addrlen > tmpaddr[0])*addrlen = tmpaddr[0];
		memset(addr, 0, sizeof(struct sockaddr));
		addr->sa_family = tmpaddr[1];
		memcpy(addr->sa_data, &tmpaddr[2], *addrlen - 2);
	}

	if(ret<0)return -1;
	return ret;
}
