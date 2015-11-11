#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ns.h>
#include <3ds/ipc.h>

static Handle nsHandle;
static int nsRefCount;

Result nsInit(void)
{
	Result res;
	if (AtomicPostIncrement(&nsRefCount)) return 0;
	res = srvGetServiceHandle(&nsHandle, "ns:s");
	if (R_FAILED(res)) AtomicDecrement(&nsRefCount);
	return res;
}

void nsExit(void)
{
	if (AtomicDecrement(&nsRefCount)) return;
	svcCloseHandle(nsHandle);
}

Result NS_LaunchTitle(u64 titleid, u32 launch_flags, u32 *procid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,3,0); // 0x200C0
	cmdbuf[1] = titleid & 0xffffffff;
	cmdbuf[2] = (titleid >> 32) & 0xffffffff;
	cmdbuf[3] = launch_flags;
	
	if(R_FAILED(ret = svcSendSyncRequest(nsHandle)))return ret;

	if(procid != NULL) *procid = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result NS_RebootToTitle(u8 mediatype, u64 titleid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x10,6,0); // 0x100180
	cmdbuf[1] = 0x1;
	cmdbuf[2] = titleid & 0xffffffff;
	cmdbuf[3] = (titleid >> 32) & 0xffffffff;
	cmdbuf[4] = mediatype;
	cmdbuf[5] = 0x0; // reserved
	cmdbuf[6] = 0x0;
	
	if(R_FAILED(ret = svcSendSyncRequest(nsHandle)))return ret;

	return (Result)cmdbuf[1];
}
