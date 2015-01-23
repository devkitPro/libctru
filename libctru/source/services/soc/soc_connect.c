#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret=0;
	int tmp_addrlen=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[0x1c];

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0)
	{
		SOCU_errno = sockfd;
		return -1;
	}

	memset(tmpaddr, 0, 0x1c);

	if(addr->sa_family == AF_INET)
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
	tmpaddr[1] = addr->sa_family;
	memcpy(&tmpaddr[2], &addr->sa_data, tmp_addrlen-2);

	cmdbuf[0] = 0x00060084;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)addrlen;
	cmdbuf[3] = 0x20;
	cmdbuf[5] = (((u32)tmp_addrlen)<<14) | 2;
	cmdbuf[6] = (u32)tmpaddr;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	SOCU_errno = ret;

	if(ret<0)return -1;
	return 0;
}
