#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

static Handle mcuCamHandle;
static int mcuCamRefCount;

Result mcuCamInit(void)
{
	if (AtomicPostIncrement(&mcuCamRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuCamHandle, "mcu::CAM");
	if (R_FAILED(res)) AtomicDecrement(&mcuCamRefCount);
	return res;
}

void mcuCamExit(void)
{
	if (AtomicDecrement(&mcuCamRefCount)) return;
	svcCloseHandle(mcuCamHandle);
}

Handle *mcuCamGetSessionHandle(void)
{
	return &mcuCamHandle;
}

Result MCUCAM_SetCameraLedState(bool on)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0001, 1, 0); // 0x00010040
	cmdbuf[1] = !!on;
	
	Result res = svcSendSyncRequest(mcuCamHandle);
	if (R_FAILED(res)) return res;
	
	return (Result)cmdbuf[1];
}

Result MCUCAM_GetCameraLedState(bool *out_on)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0002, 0, 0); // 0x00020000
	
	Result res = svcSendSyncRequest(mcuCamHandle);
	if (R_FAILED(res)) return res;
	
	*out_on = !!cmdbuf[2];
	return (Result)cmdbuf[1];
}