#include "soc_common.h"
#include <3ds/ipc.h>
#include <3ds/result.h>

int SOCU_ShutdownSockets(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x19,0,0); // 0x190000

	int ret = svcSendSyncRequest(SOCU_handle);
	if(R_FAILED(ret))return ret;
	return cmdbuf[1];
}
