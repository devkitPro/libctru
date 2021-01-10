#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ptmgets.h>
#include <3ds/ipc.h>

static Handle ptmGetsHandle;
static int ptmGetsRefCount;

Result ptmGetsInit(void)
{
	if (AtomicPostIncrement(&ptmGetsRefCount)) return 0;
	Result res = srvGetServiceHandle(&ptmGetsHandle, "ptm:gets");
	if (R_FAILED(res)) AtomicDecrement(&ptmGetsRefCount);
	return res;
}

void ptmGetsExit(void)
{
	if (AtomicDecrement(&ptmGetsRefCount)) return;
	svcCloseHandle(ptmGetsHandle);
}

Handle *ptmGetsGetSessionHandle(void)
{
	return &ptmGetsHandle;
}

Result PTMGETS_GetSystemTime(s64 *outMsY2k)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x401,0,0); // 0x04010000

	if(R_FAILED(ret = svcSendSyncRequest(ptmGetsHandle)))return ret;

	memcpy(outMsY2k, &cmdbuf[2], 8);

	return (Result)cmdbuf[1];
}
