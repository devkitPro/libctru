#include <string.h>
#include <3ds/result.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/services/fsreg.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/ipc.h>

static Handle fsRegHandle;
static int fsRegRefCount;

Result fsRegInit(void)
{
	Result ret = 0;

	if (AtomicPostIncrement(&fsRegRefCount)) return 0;

	ret = srvGetServiceHandle(&fsRegHandle, "fs:REG");

	if (R_FAILED(ret)) AtomicDecrement(&fsRegRefCount);
	return ret;
}

void fsRegExit(void)
{
	if (AtomicDecrement(&fsRegRefCount)) return;
	svcCloseHandle(fsRegHandle);
}

Handle *fsRegGetSessionHandle(void)
{
	return &fsRegHandle;
}

Result FSREG_Register(u32 pid, u64 programHandle, const FS_ProgramInfo *programInfo, const ExHeader_Arm11StorageInfo *storageInfo)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401, 0xF, 0); // 0x40103C0
	cmdbuf[1] = pid;
	cmdbuf[2] = (u32)programHandle;
	cmdbuf[3] = (u32)(programHandle >> 32);
	memcpy(&cmdbuf[4], programInfo, sizeof(FS_ProgramInfo));
	memcpy(&cmdbuf[8], storageInfo, sizeof(ExHeader_Arm11StorageInfo));

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsRegHandle))) return ret;

	return cmdbuf[1];
}

Result FSREG_Unregister(u32 pid)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x402, 1, 0); // 0x4020040
	cmdbuf[1] = pid;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsRegHandle))) return ret;

	return cmdbuf[1];
}

Result FSREG_GetProgramInfo(ExHeader_Info *exheaderInfos, u32 maxNumEntries, u64 programHandle)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x403, 3, 0); // 0x40300C0
	cmdbuf[1] = maxNumEntries;
	cmdbuf[2] = (u32)programHandle;
	cmdbuf[3] = (u32)(programHandle >> 32);

	u32 staticbufscpy[2] = {staticbufs[0], staticbufs[1]};
	staticbufs[0] = IPC_Desc_StaticBuffer(maxNumEntries * sizeof(ExHeader_Info), 0);
	staticbufs[1] = (u32)exheaderInfos;

	Result ret = svcSendSyncRequest(fsRegHandle);

	staticbufs[0] = staticbufscpy[0];
	staticbufs[1] = staticbufscpy[1];
	return R_FAILED(ret) ? ret : cmdbuf[1];
}

Result FSREG_LoadProgram(u64 *programHandle, const FS_ProgramInfo *programInfo)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x404, 4, 0); // 0x4040100
	memcpy(&cmdbuf[1], programInfo, sizeof(FS_ProgramInfo));

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsRegHandle))) return ret;
	*programHandle = ((u64)cmdbuf[3] << 32) | cmdbuf[2];

	return cmdbuf[1];
}

Result FSREG_UnloadProgram(u64 programHandle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x405, 2, 0); // 0x4050080
	cmdbuf[1] = (u32)programHandle;
	cmdbuf[2] = (u32)(programHandle >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsRegHandle))) return ret;

	return cmdbuf[1];
}

Result FSREG_CheckHostLoadId(u64 programHandle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x406, 2, 0); // 0x4060080
	cmdbuf[1] = (u32)programHandle;
	cmdbuf[2] = (u32)(programHandle >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsRegHandle))) return ret;

	return cmdbuf[1];
}
