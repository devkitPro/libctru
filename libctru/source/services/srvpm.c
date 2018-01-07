#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/srvpm.h>
#include <3ds/ipc.h>
#include <3ds/os.h>

#define IS_PRE_7X (osGetFirmVersion() < SYSTEM_VERSION(2, 39, 4))

static Handle srvPmHandle;
static int srvPmRefCount;

Result srvPmInit(void)
{
	Result res = 0;

	if (!IS_PRE_7X) res = srvInit();
	if (R_FAILED(res)) return res;

	if (AtomicPostIncrement(&srvPmRefCount)) return 0;

	if (!IS_PRE_7X)
		res = srvGetServiceHandleDirect(&srvPmHandle, "srv:pm");
	else
	{
		res = svcConnectToPort(&srvPmHandle, "srv:pm");
		if (R_SUCCEEDED(res)) res = srvInit();
	}

	if (R_FAILED(res)) srvPmExit();
	return res;
}

void srvPmExit(void)
{
	if (*srvGetSessionHandle() != 0) srvExit();
	if (AtomicDecrement(&srvPmRefCount)) return;
	if(srvPmHandle != 0) svcCloseHandle(srvPmHandle);
	srvPmHandle = 0;
}

Handle *srvPmGetSessionHandle(void)
{
	return &srvPmHandle;
}

static Result srvPmSendCommand(u32* cmdbuf)
{
	Result rc = 0;
	if (IS_PRE_7X) cmdbuf[0] |= 0x04000000;
	rc = svcSendSyncRequest(srvPmHandle);
	if (R_SUCCEEDED(rc)) rc = cmdbuf[1];

	return rc;
}

Result SRVPM_PublishToProcess(u32 notificationId, Handle process)
{
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,1,2); // 0x10042
	cmdbuf[1] = notificationId;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = process;

	return srvPmSendCommand(cmdbuf);
}

Result SRVPM_PublishToAll(u32 notificationId)
{
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,1,0); // 0x20040
	cmdbuf[1] = notificationId;

	return srvPmSendCommand(cmdbuf);
}

Result SRVPM_RegisterProcess(u32 pid, u32 count, const char (*serviceAccessControlList)[8])
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,2,2); // 0x30082
	cmdbuf[1] = pid;
	cmdbuf[2] = count*2;
	cmdbuf[3] = IPC_Desc_StaticBuffer(count*8,0);
	cmdbuf[4] = (u32)serviceAccessControlList;

	return srvPmSendCommand(cmdbuf);
}

Result SRVPM_UnregisterProcess(u32 pid)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,1,0); // 0x40040
	cmdbuf[1] = pid;

	return srvPmSendCommand(cmdbuf);
}

