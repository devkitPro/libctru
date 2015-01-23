#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>

ssize_t socuipc_cmd9(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 tmp_addrlen=0;
	u8 tmpaddr[0x1c];

	memset(tmpaddr, 0, 0x1c);

	if(dest_addr)
	{
		if(dest_addr->sa_family == AF_INET)
		{
			tmp_addrlen = 8;
		}
		else
		{
			tmp_addrlen = 0x1c;
		}

		if(addrlen < tmp_addrlen)
		{
			SOCU_errno = -EINVAL;
			return -1;
		}

		tmpaddr[0] = tmp_addrlen;
		tmpaddr[1] = dest_addr->sa_family;
		memcpy(&tmpaddr[2], &dest_addr->sa_data, tmp_addrlen-2);
	}

	cmdbuf[0] = 0x00090106;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)len;
	cmdbuf[3] = (u32)flags;
	cmdbuf[4] = (u32)tmp_addrlen;
	cmdbuf[5] = 0x20;
	cmdbuf[7] = (tmp_addrlen<<14) | 0x402;
	cmdbuf[8] = (u32)tmpaddr;
	cmdbuf[9] = (((u32)len)<<4) | 10;
	cmdbuf[10] = (u32)buf;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

ssize_t socuipc_cmda(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 tmp_addrlen=0;
	u8 tmpaddr[0x1c];

	memset(tmpaddr, 0, 0x1c);

	if(dest_addr)
	{
		if(dest_addr->sa_family == AF_INET)
		{
			tmp_addrlen = 8;
		}
		else
		{
			tmp_addrlen = 0x1c;
		}

		if(addrlen < tmp_addrlen)
		{
			SOCU_errno = -EINVAL;
			return -1;
		}

		tmpaddr[0] = tmp_addrlen;
		tmpaddr[1] = dest_addr->sa_family;
		memcpy(&tmpaddr[2], &dest_addr->sa_data, tmp_addrlen-2);
	}

	cmdbuf[0] = 0x000A0106;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)len;
	cmdbuf[3] = (u32)flags;
	cmdbuf[4] = (u32)tmp_addrlen;
	cmdbuf[5] = 0x20;
	cmdbuf[7] = (((u32)len)<<14) | 0x802;
	cmdbuf[8] = (u32)buf;
	cmdbuf[9] = (tmp_addrlen<<14) | 0x402;
	cmdbuf[10] = (u32)tmpaddr;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0)
	{
		SOCU_errno = sockfd;
		return -1;
	}

	if(len<0x2000)return socuipc_cmda(sockfd, buf, len, flags, dest_addr, addrlen);
	return socuipc_cmd9(sockfd, buf, len, flags, (struct sockaddr*)dest_addr, addrlen);
}
