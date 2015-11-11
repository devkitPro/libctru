#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ptmsysm.h>
#include <3ds/ipc.h>

static Handle ptmSysmHandle;
static int ptmSysmRefCount;

Result ptmSysmInit(void)
{
	if (AtomicPostIncrement(&ptmSysmRefCount)) return 0;
	Result res = srvGetServiceHandle(&ptmSysmHandle, "ptm:sysm");
	if (R_FAILED(res)) AtomicDecrement(&ptmSysmRefCount);
	return res;
}

void ptmSysmExit(void)
{
	if (AtomicDecrement(&ptmSysmRefCount)) return;
	svcCloseHandle(ptmSysmHandle);
}

Result PTMSYSM_ConfigureNew3DSCPU(u8 value)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x818,1,0); // 0x08180040
	cmdbuf[1] = value;

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

