#include "soc_common.h"
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfsd, int timeout)
{
	int ret = 0;
	u32 size = sizeof(struct pollfd)*nfsd;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	cmdbuf[0] = 0x00140084;
	cmdbuf[1] = (u32)nfsd;
	cmdbuf[2] = (u32)timeout;
	cmdbuf[3] = 0x20;
	cmdbuf[5] = (size<<14) | 0x2802;
	cmdbuf[6] = (u32)fds;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (size<<14) | 2;
	cmdbuf[0x104>>2] = (u32)fds;

	if((ret = svcSendSyncRequest(SOCU_handle)) != 0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
				cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}
