#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/qtmc.h>
#include <3ds/ipc.h>

static Handle qtmcHandle;
static int qtmcRefCount;

Result qtmcInit(void)
{
	if (AtomicPostIncrement(&qtmcRefCount)) return 0;
	Result res = srvGetServiceHandle(&qtmcHandle, "qtm:c");

	if (R_FAILED(res)) AtomicDecrement(&qtmcRefCount);
	return res;
}

void qtmcExit(void)
{
	if (AtomicDecrement(&qtmcRefCount)) return;
	svcCloseHandle(qtmcHandle);
}

Handle *qtmcGetSessionHandle(void)
{
	return &qtmcHandle;
}

Result QTMC_StartHardwareCheck(void)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 0, 0); // 0x10000

	res = svcSendSyncRequest(qtmcHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMC_StopHardwareCheck(void)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 0, 0); // 0x20000

	res = svcSendSyncRequest(qtmcHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMC_SetBarrierPattern(u32 pattern)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3, 1, 0); // 0x30040
	cmdbuf[1] = pattern;

	res = svcSendSyncRequest(qtmcHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMC_WaitAndCheckExpanderWorking(bool *outWorking)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4, 0, 0); // 0x40000

	res = svcSendSyncRequest(qtmcHandle);
	if (R_FAILED(res)) return res;

	*outWorking = (bool)(cmdbuf[2] & 1);

	return cmdbuf[1];
}

Result QTMC_SetIrLedStatusOverride(bool on)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5, 1, 0); // 0x50040
	cmdbuf[1] = (u32)on;

	res = svcSendSyncRequest(qtmcHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}
