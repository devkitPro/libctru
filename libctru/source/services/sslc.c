#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/sslc.h>
#include <3ds/ipc.h>

Handle __sslc_servhandle;
static int __sslc_refcount;

Result SSLC_Initialize(void);

Result sslcInit(Handle session_handle)
{
	Result ret=0;

	if (AtomicPostIncrement(&__sslc_refcount)) return 0;

	__sslc_servhandle = session_handle;

	if(__sslc_servhandle==0)ret = srvGetServiceHandle(&__sslc_servhandle, "ssl:C");
	if(session_handle==0 && R_SUCCEEDED(ret))ret = SSLC_Initialize();
	if (R_FAILED(ret)) AtomicDecrement(&__sslc_refcount);

	return ret;
}

void sslcExit(void)
{
	if (AtomicDecrement(&__sslc_refcount)) return;

	svcCloseHandle(__sslc_servhandle);
}

Result SSLC_Initialize(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,0,2); // 0x10002
	cmdbuf[1]=IPC_Desc_CurProcessHandle();
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

Result sslcAddTrustedRootCA(u32 RootCertChain_contexthandle, u8 *cert, u32 certsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5,2,2); // 0x50082
	cmdbuf[1]=RootCertChain_contexthandle;
	cmdbuf[2]=certsize;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

