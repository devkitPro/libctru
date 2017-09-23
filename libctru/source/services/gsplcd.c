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

Result GSPLCD_PowerOnAllBacklights(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0F,0,0); // 0x0F0000

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;

	return cmdbuf[1];
}

Result GSPLCD_PowerOffAllBacklights(void)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x10,0,0); // 0x100000

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;

	return cmdbuf[1];
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

Result GSPLCD_SetLedForceOff(bool disable)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x13,1,0); // 0x130040
	cmdbuf[1] = disable & 0xFF;

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;

	return cmdbuf[1];
}

Result GSPLCD_GetVendors(u8 *vendors)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x14,0,0); // 0x140000

	Result ret=0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;

	if(vendors) *vendors = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result GSPLCD_GetBrightness(u32 screen, u32 *brightness)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x15,1,0); // 0x150040
	cmdbuf[1] = screen;
	
	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;
	
	*brightness = cmdbuf[2];
	
	return cmdbuf[2];
}

Result GSPLCD_SetBrightness(u32 screen, u32 brightness)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0B,2,0); // 0xB0080
	cmdbuf[1] = screen;
	cmdbuf[2] = brightness;
	
	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;

	return cmdbuf[1];
}

Result GSPLCD_SetBrightnessRaw(u32 screen, u32 brightness)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0A,2,0); // 0xA0080
	cmdbuf[1] = screen;
	cmdbuf[2] = brightness;
	
	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(gspLcdHandle))) return ret;

	return cmdbuf[1];
}