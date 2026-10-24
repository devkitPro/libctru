#include <3ds/services/mcu_common.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

static Handle mcuCdcHandle;
static int mcuCdcRefCount;

Result mcuCdcInit(void)
{
	if (AtomicPostIncrement(&mcuCdcRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuCdcHandle, "mcu::CDC");
	if (R_FAILED(res)) AtomicDecrement(&mcuCdcRefCount);
	return res;
}

void mcuCdcExit(void)
{
	if (AtomicDecrement(&mcuCdcRefCount)) return;
	svcCloseHandle(mcuCdcHandle);
}

Handle *mcuCdcGetSessionHandle(void)
{
	return &mcuCdcHandle;
}

Result MCUCDC_SetReg26h()
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0001, 0, 0); // 0x00010000
	
	Result res = svcSendSyncRequest(mcuCdcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}
