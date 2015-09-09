#include "soc_common.h"
#include <errno.h>
#include <3ds/ipc.h>

long gethostid(void)
{
	int ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x16,0,0); // 0x160000

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		errno = SYNC_ERROR;
		return -1;
	}

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret = cmdbuf[2];

	return ret;
}
