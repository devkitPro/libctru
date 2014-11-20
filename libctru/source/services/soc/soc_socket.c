#include "soc_common.h"
#include <sys/socket.h>

int socket(int domain, int type, int protocol)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000200C2;
	cmdbuf[1] = domain;
	cmdbuf[2] = type;
	cmdbuf[3] = protocol;
	cmdbuf[4] = 0x20;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	SOCU_errno = ret;

	if(ret!=0)return -1;
	return _net_convert_error(cmdbuf[2]);
}
