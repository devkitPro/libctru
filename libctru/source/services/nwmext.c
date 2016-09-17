#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/nwmext.h>

Handle nwmExtHandle;
static int nwmExtRefCount;

Result nwmExtInit(void)
{
	Result res=0;
	if (AtomicPostIncrement(&nwmExtRefCount)) return 0;
	res = srvGetServiceHandle(&nwmExtHandle, "nwm::EXT");
	if (R_FAILED(res)) AtomicDecrement(&nwmExtRefCount);
	return res;
}

void nwmExtExit(void)
{
	if (AtomicDecrement(&nwmExtRefCount)) return;
	svcCloseHandle(nwmExtHandle);
}

Result NWMEXT_ControlWirelessEnabled(bool enableWifi)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8, 1, 0); // 0x00080040
	cmdbuf[1] = enableWifi ? 0 : 1;

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(nwmExtHandle))) return ret;

	return cmdbuf[1];
}
