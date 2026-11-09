#include "soc_common.h"
#include <errno.h>
#include <3ds/ipc.h>
#include <3ds/result.h>

int SOCP_SetNetworkOpt(int level, NetworkOpt optname, void * optval, socklen_t optlen)
{
	int ret = -1;
	u32* cmdbuf = getThreadCommandBuffer();

	//Maximum supported optlen is 0x200, anything above it causes kernel panic.
	if(optlen > 0x200)
	{
		errno = EINVAL;
		return -1;
	}

	cmdbuf[0] = IPC_MakeHeader(0x3,3,2); //0x300C2
	cmdbuf[1] = level;
	cmdbuf[2] = optname;
	cmdbuf[3] = 0;
	cmdbuf[4] = IPC_Desc_StaticBuffer(optlen, 2);
	cmdbuf[5] = (u32)optval;

	ret = svcSendSyncRequest(SOCP_handle);
	if(R_FAILED(ret))
	{
		errno = SYNC_ERROR;
		return ret;
	}

	ret = (int)cmdbuf[1];
	if(R_FAILED(ret))
	{
		errno = SYNC_ERROR;
		return ret;
	}

	ret = _net_convert_error(cmdbuf[2]);

	if(ret < 0)
	{
		errno = -ret;
		return -1;
	}

	return ret;
}
