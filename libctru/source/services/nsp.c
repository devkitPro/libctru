#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/nsp.h>
#include <3ds/ipc.h>

static Handle nspHandle;
static int nspRefCount;

Result nspInit(void)
{
	Result res;
	if (AtomicPostIncrement(&nspRefCount)) return 0;
	res = srvGetServiceHandle(&nspHandle, "ns:p");
	if (R_FAILED(res)) AtomicDecrement(&nspRefCount);
	return res;
}

void nspExit(void)
{
	if (AtomicDecrement(&nspRefCount)) return;
	svcCloseHandle(nspHandle);
}

Result NSP_RebootSystem(bool launchtitle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 1, 0); // 0x10040
	cmdbuf[1] = launchtitle;

	if(R_FAILED(ret = svcSendSyncRequest(nspHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NSP_ShutdownAsync(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 0, 0); // 0x20000

	if(R_FAILED(ret = svcSendSyncRequest(nspHandle)))return ret;
	return (Result)cmdbuf[1];
}