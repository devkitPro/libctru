#include "soc_common.h"
#include <3ds/ipc.h>
#include <3ds/result.h>

int SOCU_CloseSockets(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x21,0,2); // 0x210002;
	cmdbuf[1] = IPC_Desc_CurProcessHandle();

	int ret = svcSendSyncRequest(SOCU_handle);
	if(R_FAILED(ret))return ret;
	return cmdbuf[1];
}
