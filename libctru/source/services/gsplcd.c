#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/gsplcd.h>

Handle gspLcdHandle;
static int gspLcdRefCount;

Result gspLcdInit(void)
{
	Result res=0;
	if (AtomicPostIncrement(&gspLcdRefCount)) return 0;
	res = srvGetServiceHandle(&gspLcdHandle, "gsp::Lcd");
	if (R_FAILED(res)) AtomicDecrement(&gspLcdRefCount);
	return res;
}

void gspLcdExit(void)
{
	if (AtomicDecrement(&gspLcdRefCount)) return;
	svcCloseHandle(gspLcdHandle);
}

Result GSPLCD_PowerOnBacklight(u32 screen)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x11,1,0); // 0x110040
	cmdbuf[1] = screen;

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;

	return cmdbuf[1];
}

Result GSPLCD_PowerOffBacklight(u32 screen)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x12,1,0); // 0x120040
	cmdbuf[1] = screen;

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;

	return cmdbuf[1];
}

