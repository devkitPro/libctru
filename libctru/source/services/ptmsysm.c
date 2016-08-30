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

Result PTMSYSM_CheckNew3DS(void)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x040A,0,0); // 0x040A0000

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return 0;

	return (Result)cmdbuf[1];
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

Result PTMSYSM_ShutdownAsync(u64 timeout)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x407,3,0); // 0x040700C0
	cmdbuf[1] = 0;
	cmdbuf[2] = timeout & 0xffffffff;
	cmdbuf[3] = (timeout >> 32) & 0xffffffff;

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_RebootAsync(u64 timeout)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x409,2,0); // 0x04090080
	cmdbuf[1] = timeout & 0xffffffff;
	cmdbuf[2] = (timeout >> 32) & 0xffffffff;

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}
