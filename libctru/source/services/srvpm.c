#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/srvpm.h>
#include <3ds/ipc.h>

static Handle srvPmHandle;
static int srvPmRefCount;

Result srvPmInit(void)
{
	if (AtomicPostIncrement(&srvPmRefCount)) return 0;
	Result res = srvGetServiceHandle(&srvPmHandle, "srv:pm");
	if (R_FAILED(res)) AtomicDecrement(&srvPmRefCount);
	return res;
}

void srvPmExit(void)
{
	if (AtomicDecrement(&srvPmRefCount)) return;
	svcCloseHandle(srvPmHandle);
}

Result SRVPM_PublishToProcess(u32 notificationId, Handle process)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,1,2); // 0x10042
	cmdbuf[1] = notificationId;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = process;

	if(R_FAILED(rc = svcSendSyncRequest(srvPmHandle)))return rc;

	return cmdbuf[1];
}

Result SRVPM_PublishToAll(u32 notificationId)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,1,0); // 0x20040
	cmdbuf[1] = notificationId;

	if(R_FAILED(rc = svcSendSyncRequest(srvPmHandle)))return rc;

	return cmdbuf[1];
}

Result SRVPM_RegisterProcess(u32 procid, u32 count, void* serviceaccesscontrol)
{
	Result rc = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,2,2); // 0x30082
	cmdbuf[1] = procid;
	cmdbuf[2] = count;
	cmdbuf[3] = IPC_Desc_StaticBuffer(count*4,0);
	cmdbuf[4] = (u32)serviceaccesscontrol;

	if(R_FAILED(rc = svcSendSyncRequest(srvPmHandle))) return rc;

	return cmdbuf[1];
}

Result SRVPM_UnregisterProcess(u32 procid)
{
	Result rc = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,1,0); // 0x40040
	cmdbuf[1] = procid;

	if(R_FAILED(rc = svcSendSyncRequest(srvPmHandle))) return rc;

	return cmdbuf[1];
}

