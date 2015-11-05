#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/hb.h>
#include <3ds/ipc.h>

static Handle hbHandle;

Result hbInit(void)
{
	return srvGetServiceHandle(&hbHandle, "hb:HB");
}

void hbExit(void)
{
	svcCloseHandle(hbHandle);
}

Result HB_FlushInvalidateCache(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,1,2); // 0x10042
	cmdbuf[1] = 0x00000000;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = CUR_PROCESS_HANDLE;

	if((ret = svcSendSyncRequest(hbHandle))!=0) return ret;
	
	return (Result)cmdbuf[1];
}

Result HB_GetBootloaderAddresses(void** load3dsx, void** setArgv)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if((ret = svcSendSyncRequest(hbHandle))!=0) return ret;

	if(load3dsx)*load3dsx=(void*)cmdbuf[2];
	if(setArgv)*setArgv=(void*)cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result HB_ReprotectMemory(u32* addr, u32 pages, u32 mode, u32* reprotectedPages)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,3,0); // 0x900C0
	cmdbuf[1] = (u32)addr;
	cmdbuf[2] = pages;
	cmdbuf[3] = mode;

	if((ret = svcSendSyncRequest(hbHandle))!=0) return ret;

	if(reprotectedPages)
	{
		if(!ret)*reprotectedPages=(u32)cmdbuf[2];
		else *reprotectedPages=0;
	}
	
	return (Result)cmdbuf[1];
}
