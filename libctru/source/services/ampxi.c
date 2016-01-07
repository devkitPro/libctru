#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ampxi.h>
#include <3ds/ipc.h>

static Handle ampxiHandle;
static int ampxiRefCount;

Result ampxiInit(Handle servhandle)
{
	Result res=0;
	if (AtomicPostIncrement(&ampxiRefCount)) return 0;
	if(servhandle)
	{
		ampxiHandle = servhandle;
	}
	else
	{
		res = srvGetServiceHandle(&ampxiHandle, "pxi:am9");
		if (R_FAILED(res)) AtomicDecrement(&ampxiRefCount);
	}
	return res;
}

void ampxiExit(void)
{
	if (AtomicDecrement(&ampxiRefCount)) return;
	svcCloseHandle(ampxiHandle);
}

Result ampxiWriteTWLSavedata(u64 titleid, u8 *buffer, u32 size, u32 image_filepos, u8 section_type, u8 operation)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x46,6,2); // 0x460182
	cmdbuf[1] = titleid & 0xffffffff;
	cmdbuf[2] = (titleid >> 32) & 0xffffffff;
	cmdbuf[3] = size;
	cmdbuf[4] = image_filepos;
	cmdbuf[5] = section_type;
	cmdbuf[6] = operation;

	cmdbuf[7] = IPC_Desc_PXIBuffer(size, 0, 0);
	cmdbuf[8] = (u32)buffer;

	if(R_FAILED(ret = svcSendSyncRequest(ampxiHandle)))return ret;

	return (Result)cmdbuf[1];
}
