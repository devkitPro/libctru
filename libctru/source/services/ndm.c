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

Handle __ndmu_servhandle;
static int __ndmu_refcount;

Result ndmuInit(void)
{
	Result ret=0;

	if (AtomicPostIncrement(&__ndmu_refcount)) return 0;

	ret = srvGetServiceHandle(&__ndmu_servhandle, "ndm:u");
	if (R_FAILED(ret)) AtomicDecrement(&__ndmu_refcount);

	return ret;
}

void ndmuExit(void)
{
	if (AtomicDecrement(&__ndmu_refcount)) return;

	svcCloseHandle(__ndmu_servhandle);
	__ndmu_servhandle = 0;
}

Result ndmuEnterExclusiveState(NDM_ExclusiveState state)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,1,2); // 0x10042
	cmdbuf[1]=state;
	cmdbuf[2]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__ndmu_servhandle)))return ret;

	return cmdbuf[1];
}

Result ndmuLeaveExclusiveState(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2,0,2); // 0x20002
	cmdbuf[1]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__ndmu_servhandle)))return ret;

	return cmdbuf[1];
}

