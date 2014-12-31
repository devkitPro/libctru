#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/hb.h>

static Handle hbHandle;

Result hbInit()
{
	return srvGetServiceHandle(&hbHandle, "hb:HB");
}

void hbExit()
{
	svcCloseHandle(hbHandle);
}

Result HB_FlushInvalidateCache(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010042;
	cmdbuf[1] = 0x00000000;
	cmdbuf[2] = 0x00000000;
	cmdbuf[3] = 0xFFFF8001;

	if((ret = svcSendSyncRequest(hbHandle))!=0) return ret;
	
	return (Result)cmdbuf[1];
}

Result HB_GetBootloaderAddresses(void** load3dsx, void** setArgv)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00060000;

	if((ret = svcSendSyncRequest(hbHandle))!=0) return ret;

	if(load3dsx)*load3dsx=(void*)cmdbuf[2];
	if(setArgv)*setArgv=(void*)cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result HB_ReprotectMemory(u32* addr, u32 pages, u32 mode, u32* reprotectedPages)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000900C0;
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
