#include "soc_common.h"
#include <sys/socket.h>

ssize_t socuipc_cmd7(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 tmp_addrlen=0;
	u8 tmpaddr[0x1c];
	u32 saved_threadstorage[2];

	memset(tmpaddr, 0, 0x1c);

	if(src_addr)tmp_addrlen = 0x1c;

	cmdbuf[0] = 0x00070104;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)len;
	cmdbuf[3] = (u32)flags;
	cmdbuf[4] = (u32)tmp_addrlen;
	cmdbuf[5] = 0x20;
	cmdbuf[7] = (((u32)len)<<4) | 12;
	cmdbuf[8] = (u32)buf;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (tmp_addrlen<<14) | 2;
	cmdbuf[0x104>>2] = (u32)tmpaddr;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret>0 && src_addr!=NULL)
	{
		src_addr->sa_family = tmpaddr[1];
		if(*addrlen > tmpaddr[0])*addrlen = tmpaddr[0];
		memcpy(src_addr->sa_data, &tmpaddr[2], *addrlen - 2);
	}

	if(ret<0)return -1;
	return ret;
}

ssize_t socuipc_cmd8(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();	
	u32 tmp_addrlen=0;
	u8 tmpaddr[0x1c];
	u32 saved_threadstorage[4];

	if(src_addr)tmp_addrlen = 0x1c;

	memset(tmpaddr, 0, 0x1c);

	cmdbuf[0] = 0x00080102;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)len;
	cmdbuf[3] = (u32)flags;
	cmdbuf[4] = (u32)tmp_addrlen;
	cmdbuf[5] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];
	saved_threadstorage[2] = cmdbuf[0x108>>2];
	saved_threadstorage[3] = cmdbuf[0x10c>>2];
	
	cmdbuf[0x100>>2] = (((u32)len)<<14) | 2;
	cmdbuf[0x104>>2] = (u32)buf;
	cmdbuf[0x108>>2] = (tmp_addrlen<<14) | 2;
	cmdbuf[0x10c>>2] = (u32)tmpaddr;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];
	cmdbuf[0x108>>2] = saved_threadstorage[2];
	cmdbuf[0x10c>>2] = saved_threadstorage[3];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret>0 && src_addr!=NULL)
	{
		src_addr->sa_family = tmpaddr[1];
		if(*addrlen > tmpaddr[0])*addrlen = tmpaddr[0];
		memcpy(src_addr->sa_data, &tmpaddr[2], *addrlen - 2);
	}

	if(ret<0)return -1;
	return ret;
}


ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0)
	{
		SOCU_errno = sockfd;
		return -1;
	}

	if(len<0x2000)return socuipc_cmd8(sockfd, buf, len, flags, src_addr, addrlen);
	return socuipc_cmd7(sockfd, buf, len, flags, src_addr, addrlen);
}
