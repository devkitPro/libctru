#include <string.h>
#include <3ds/services/loader.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/ipc.h>

static Handle loaderHandle;
static int loaderRefCount;

Result loaderInit(void)
{
	Result res;
	if (AtomicPostIncrement(&loaderRefCount)) return 0;
	res = srvGetServiceHandle(&loaderHandle, "Loader");
	if (R_FAILED(res)) AtomicDecrement(&loaderRefCount);
	return res;
}

void loaderExit(void)
{
	if (AtomicDecrement(&loaderRefCount)) return;
	svcCloseHandle(loaderHandle);
}

Result LOADER_LoadProcess(Handle* process, u64 programHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(1, 2, 0); // 0x10080
	cmdbuf[1] = (u32)programHandle;
	cmdbuf[2] = (u32)(programHandle >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(loaderHandle))) return ret;

	*process = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result LOADER_RegisterProgram(u64* programHandle, const FS_ProgramInfo *programInfo, const FS_ProgramInfo *programInfoUpdate)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(2, 8, 0); // 0x20200
	memcpy(&cmdbuf[1], programInfo, sizeof(FS_ProgramInfo));
	memcpy(&cmdbuf[5], programInfoUpdate, sizeof(FS_ProgramInfo));

	if(R_FAILED(ret = svcSendSyncRequest(loaderHandle))) return ret;

	*programHandle = ((u64)cmdbuf[3] << 32) | cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result LOADER_UnregisterProgram(u64 programHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(3, 2, 0); // 0x30080
	cmdbuf[1] = (u32)programHandle;
	cmdbuf[2] = (u32)(programHandle >> 32);

	if(R_FAILED(ret = svcSendSyncRequest(loaderHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result LOADER_GetProgramInfo(ExHeader_Info* exheaderInfo, u64 programHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(4, 2, 0); // 0x40080
	cmdbuf[1] = (u32)programHandle;
	cmdbuf[2] = (u32)(programHandle >> 32);

	u32 staticbufscpy[2] = {staticbufs[0], staticbufs[1]};
	staticbufs[0] = IPC_Desc_StaticBuffer(0x400, 0);
	staticbufs[1] = (u32)exheaderInfo;

	if(R_FAILED(ret = svcSendSyncRequest(loaderHandle))) return ret;

	staticbufs[0] = staticbufscpy[0];
	staticbufs[1] = staticbufscpy[1];

	return (Result)cmdbuf[1];
}
