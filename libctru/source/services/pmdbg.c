#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/pmdbg.h>
#include <3ds/ipc.h>

static Handle pmDbgHandle;
static int pmDbgRefcount;

Result pmDbgInit(void)
{
	Result res;
	if (AtomicPostIncrement(&pmDbgRefcount)) return 0;
	res = srvGetServiceHandle(&pmDbgHandle, "pm:dbg");
	if (R_FAILED(res)) AtomicDecrement(&pmDbgRefcount);
	return res;
}

void pmDbgExit(void)
{
	if (AtomicDecrement(&pmDbgRefcount)) return;
	svcCloseHandle(pmDbgHandle);
}

Handle *pmDbgGetSessionHandle(void)
{
	return &pmDbgHandle;
}

Result PMDBG_LaunchAppDebug(Handle *outDebug, const FS_ProgramInfo *programInfo, u32 launchFlags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 5, 0); // 0x10140
	memcpy(&cmdbuf[1], programInfo, sizeof(FS_ProgramInfo));
	cmdbuf[5] = launchFlags;

	if(R_FAILED(ret = svcSendSyncRequest(pmDbgHandle)))return ret;

	*outDebug = cmdbuf[3];
	return (Result)cmdbuf[1];
}

Result PMDBG_LaunchApp(const FS_ProgramInfo *programInfo, u32 launchFlags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 5, 0); // 0x20140
	memcpy(&cmdbuf[1], programInfo, sizeof(FS_ProgramInfo));
	cmdbuf[5] = launchFlags;

	if(R_FAILED(ret = svcSendSyncRequest(pmDbgHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMDBG_RunQueuedProcess(Handle *outDebug)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3, 0, 0); // 0x30000

	if(R_FAILED(ret = svcSendSyncRequest(pmDbgHandle)))return ret;

	*outDebug = cmdbuf[3];

	return (Result)cmdbuf[1];
}
