#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ndm.h>
#include <3ds/ipc.h>

Handle ndmuHandle;
static int ndmuRefCount;

Result ndmuInit(void)
{
	Result ret=0;

	if (AtomicPostIncrement(&ndmuRefCount)) return 0;

	ret = srvGetServiceHandle(&ndmuHandle, "ndm:u");
	if (R_FAILED(ret)) AtomicDecrement(&ndmuRefCount);

	return ret;
}

void ndmuExit(void)
{
	if (AtomicDecrement(&ndmuRefCount)) return;

	svcCloseHandle(ndmuHandle);
	ndmuHandle = 0;
}

Result NDMU_EnterExclusiveState(NDM_ExclusiveState state)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,1,2); // 0x10042
	cmdbuf[1]=state;
	cmdbuf[2]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return cmdbuf[1];
}

Result NDMU_LeaveExclusiveState(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2,0,2); // 0x20002
	cmdbuf[1]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return cmdbuf[1];
}

