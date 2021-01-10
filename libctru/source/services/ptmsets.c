#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ptmsets.h>
#include <3ds/ipc.h>

static Handle ptmSetsHandle;
static int ptmSetsRefCount;

Result ptmSetsInit(void)
{
	if (AtomicPostIncrement(&ptmSetsRefCount)) return 0;
	Result res = srvGetServiceHandle(&ptmSetsHandle, "ptm:sets");
	if (R_FAILED(res)) AtomicDecrement(&ptmSetsRefCount);
	return res;
}

void ptmSetsExit(void)
{
	if (AtomicDecrement(&ptmSetsRefCount)) return;
	svcCloseHandle(ptmSetsHandle);
}

Handle *ptmSetsGetSessionHandle(void)
{
	return &ptmSetsHandle;
}

Result PTMSETS_SetSystemTime(s64 msY2k)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1,2,0); // 0x00010080
	memcpy(&cmdbuf[1], &msY2k, 8);

	if(R_FAILED(ret = svcSendSyncRequest(ptmSetsHandle)))return ret;

	return (Result)cmdbuf[1];
}
