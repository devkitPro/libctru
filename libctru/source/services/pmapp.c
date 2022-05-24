#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/pmapp.h>
#include <3ds/ipc.h>

static Handle pmAppHandle;
static int pmAppRefcount;

Result pmAppInit(void)
{
	Result res;
	if (AtomicPostIncrement(&pmAppRefcount)) return 0;
	res = srvGetServiceHandle(&pmAppHandle, "pm:app");
	if (R_FAILED(res)) AtomicDecrement(&pmAppRefcount);
	return res;
}

void pmAppExit(void)
{
	if (AtomicDecrement(&pmAppRefcount)) return;
	svcCloseHandle(pmAppHandle);
}

Handle *pmAppGetSessionHandle(void)
{
	return &pmAppHandle;
}

Result PMAPP_LaunchTitle(const FS_ProgramInfo *programInfo, u32 launchFlags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 5, 0); // 0x10140
	memcpy(&cmdbuf[1], programInfo, sizeof(FS_ProgramInfo));
	cmdbuf[5] = launchFlags;

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_LaunchFIRMSetParams(u32 firmTidLow, u32 size, const void* in)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 2, 2); // 0x20082
	cmdbuf[1] = firmTidLow;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[4] = (u32)in;

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_TerminateCurrentApplication(s64 timeout)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3, 2, 0); // 0x30080
	cmdbuf[1] = (u32)timeout;
	cmdbuf[2] = (u32)(timeout >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_TerminateTitle(u64 titleId, s64 timeout)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4, 4, 0); // 0x40100
	cmdbuf[1] = (u32)titleId;
	cmdbuf[2] = (u32)(titleId >> 32);
	cmdbuf[3] = (u32)timeout;
	cmdbuf[4] = (u32)(timeout >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_TerminateProcess(u32 pid, s64 timeout)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5, 3, 0); // 0x500C0
	cmdbuf[1] = pid;
	cmdbuf[2] = (u32)timeout;
	cmdbuf[3] = (u32)(timeout >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_PrepareForReboot(s64 timeout)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6, 2, 2); // 0x60082
	cmdbuf[1] = (u32)timeout;
	cmdbuf[2] = (u32)(timeout >> 32);
	cmdbuf[3] = IPC_Desc_CurProcessId();
	cmdbuf[4] = 0;

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_GetFIRMLaunchParams(void *out, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7, 1, 2); // 0x70042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[3] = (u32)out;

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_GetTitleExheaderFlags(ExHeader_Arm11CoreInfo* outCoreInfo, ExHeader_SystemInfoFlags* outSiFlags, const FS_ProgramInfo *programInfo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8, 4, 0); // 0x80100
	memcpy(&cmdbuf[1], programInfo, sizeof(FS_ProgramInfo));

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	memcpy(outCoreInfo, &cmdbuf[2], sizeof(ExHeader_Arm11CoreInfo));
	memcpy(outSiFlags, &cmdbuf[4], sizeof(ExHeader_SystemInfoFlags));

	return (Result)cmdbuf[1];
}

Result PMAPP_SetFIRMLaunchParams(u32 size, const void* in)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9, 1, 2); // 0x90042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[3] = (u32)in;

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_SetAppResourceLimit(s64 cpuTime)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA, 5, 0); // 0xA0140
	cmdbuf[1] = 0;
	cmdbuf[2] = RESLIMIT_CPUTIME;
	cmdbuf[3] = (u32)cpuTime;
	cmdbuf[4] = 0;
	cmdbuf[5] = 0;

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_GetAppResourceLimit(s64 *outCpuTime)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB, 5, 0); // 0xB0140
	cmdbuf[1] = 0;
	cmdbuf[2] = RESLIMIT_CPUTIME;
	cmdbuf[3] = 0;
	cmdbuf[4] = 0;
	cmdbuf[5] = 0;

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;
	*outCpuTime = cmdbuf[2] | ((s64)cmdbuf[3] << 32);

	return (Result)cmdbuf[1];
}

Result PMAPP_UnregisterProcess(u64 tid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC, 0, 0); // 0xC0000
	cmdbuf[1] = (u32)tid;
	cmdbuf[2] = (u32)(tid >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PMAPP_LaunchTitleUpdate(const FS_ProgramInfo *programInfo, const FS_ProgramInfo *programInfoUpdate, u32 launchFlags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD, 9, 0); // 0xD0240
	memcpy(&cmdbuf[1], programInfo, sizeof(FS_ProgramInfo));
	memcpy(&cmdbuf[5], programInfoUpdate, sizeof(FS_ProgramInfo));
	cmdbuf[9] = launchFlags;

	if(R_FAILED(ret = svcSendSyncRequest(pmAppHandle)))return ret;

	return (Result)cmdbuf[1];
}
