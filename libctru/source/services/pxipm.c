#include <string.h>
#include <3ds/result.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/services/pxipm.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/ipc.h>

static Handle pxiPmHandle;
static int pxiPmRefCount;

Result pxiPmInit(void)
{
	Result ret = 0;

	if (AtomicPostIncrement(&pxiPmRefCount)) return 0;

	ret = srvGetServiceHandle(&pxiPmHandle, "PxiPM");

	if (R_FAILED(ret)) AtomicDecrement(&pxiPmRefCount);
	return ret;
}

void pxiPmExit(void)
{
	if (AtomicDecrement(&pxiPmRefCount)) return;
	svcCloseHandle(pxiPmHandle);
}

Handle *pxiPmGetSessionHandle(void)
{
	return &pxiPmHandle;
}

Result PXIPM_GetProgramInfo(ExHeader_Info *exheaderInfo, u64 programHandle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 2, 2); // 0x10082
	cmdbuf[1] = (u32)programHandle;
	cmdbuf[2] = (u32)(programHandle >> 32);
	cmdbuf[3] = IPC_Desc_PXIBuffer(sizeof(ExHeader_Info), 0, false);
	cmdbuf[4] = (u32)exheaderInfo;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(pxiPmHandle))) return ret;

	return cmdbuf[1];
}

Result PXIPM_RegisterProgram(u64 *programHandle, const FS_ProgramInfo *programInfo, const FS_ProgramInfo *updateInfo)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 8, 0); // 0x20200
	memcpy(&cmdbuf[1], programInfo, sizeof(FS_ProgramInfo));
	memcpy(&cmdbuf[5], updateInfo, sizeof(FS_ProgramInfo));

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(pxiPmHandle))) return ret;
	*programHandle = ((u64)cmdbuf[3] << 32) | cmdbuf[2];

	return cmdbuf[1];
}

Result PXIPM_UnregisterProgram(u64 programHandle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3, 2, 0); // 0x30080
	cmdbuf[1] = (u32)programHandle;
	cmdbuf[2] = (u32)(programHandle >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(pxiPmHandle))) return ret;

	return cmdbuf[1];
}
