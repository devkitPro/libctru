#include "soc_common.h"
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/services/soc.h>

int SOCU_GetNetworkOpt(int level, NetworkOpt optname, void * optval, socklen_t * optlen)
{
	int			  i, ret;
	u32			  *cmdbuf	  = getThreadCommandBuffer();
	u32			  *staticbufs = getThreadStaticBuffers();
	u32			  saved_threadstorage[2];

	cmdbuf[0] = IPC_MakeHeader(0x1A,3,0); //0x1A00C0
	cmdbuf[1] = level;
	cmdbuf[2] = optname;
	cmdbuf[3] = *optlen;

	// Save the thread storage values
	for(i = 0 ; i < 2 ; ++i)
		saved_threadstorage[i] = staticbufs[i];

	staticbufs[0] = IPC_Desc_StaticBuffer(*optlen, 0);
	staticbufs[1] = (u32)optval;

	ret = svcSendSyncRequest(SOCU_handle);

	// Restore the thread storage values
	for(i = 0 ; i < 2 ; ++i)
		staticbufs[i] = saved_threadstorage[i];

	if(R_FAILED(ret)) {
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

	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	*optlen = cmdbuf[3];

	return ret;
}
