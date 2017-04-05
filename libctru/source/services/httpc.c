#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/sslc.h>
#include <3ds/services/httpc.h>
#include <3ds/ipc.h>

Handle __httpc_servhandle;
static int __httpc_refcount;

u32 *__httpc_sharedmem_addr;
static u32 __httpc_sharedmem_size;
static Handle __httpc_sharedmem_handle;

static Result HTTPC_Initialize(Handle handle, u32 sharedmem_size, Handle sharedmem_handle);
static Result HTTPC_Finalize(Handle handle);

static Result HTTPC_CreateContext(Handle handle, HTTPC_RequestMethod method, const char* url, Handle* contextHandle);
static Result HTTPC_CloseContext(Handle handle, Handle contextHandle);

static Result HTTPC_InitializeConnectionSession(Handle handle, Handle contextHandle);
static Result HTTPC_SetProxyDefault(Handle handle, Handle contextHandle);

Result httpcInit(u32 sharedmem_size)
{
	Result ret=0;

	if (AtomicPostIncrement(&__httpc_refcount)) return 0;

	ret = srvGetServiceHandle(&__httpc_servhandle, "http:C");
	if (R_SUCCEEDED(ret))
	{
		__httpc_sharedmem_size = sharedmem_size;
		__httpc_sharedmem_handle = 0;

		if(__httpc_sharedmem_size)
		{
			__httpc_sharedmem_addr = memalign(0x1000, __httpc_sharedmem_size);
			if(__httpc_sharedmem_addr==NULL)ret = -1;

			if (R_SUCCEEDED(ret))
			{
				memset(__httpc_sharedmem_addr, 0, __httpc_sharedmem_size);
				ret = svcCreateMemoryBlock(&__httpc_sharedmem_handle, (u32)__httpc_sharedmem_addr, __httpc_sharedmem_size, 0, 3);
			}
		}

		if (R_SUCCEEDED(ret))ret = HTTPC_Initialize(__httpc_servhandle, __httpc_sharedmem_size, __httpc_sharedmem_handle);
		if (R_FAILED(ret)) svcCloseHandle(__httpc_servhandle);
	}
	if (R_FAILED(ret)) AtomicDecrement(&__httpc_refcount);

	if (R_FAILED(ret) && __httpc_sharedmem_handle)
	{
		svcCloseHandle(__httpc_sharedmem_handle);
		__httpc_sharedmem_handle = 0;
		__httpc_sharedmem_size = 0;

		free(__httpc_sharedmem_addr);
		__httpc_sharedmem_addr = NULL;
	}

	return ret;
}

void httpcExit(void)
{
	if (AtomicDecrement(&__httpc_refcount)) return;

	HTTPC_Finalize(__httpc_servhandle);

	svcCloseHandle(__httpc_servhandle);

	if(__httpc_sharedmem_handle)
	{
		svcCloseHandle(__httpc_sharedmem_handle);
		__httpc_sharedmem_handle = 0;
		__httpc_sharedmem_size = 0;

		free(__httpc_sharedmem_addr);
		__httpc_sharedmem_addr = NULL;
	}
}

Result httpcOpenContext(httpcContext *context, HTTPC_RequestMethod method, const char* url, u32 use_defaultproxy)
{
	Result ret=0;

	ret = HTTPC_CreateContext(__httpc_servhandle, method, url, &context->httphandle);
	if(R_FAILED(ret))return ret;

	ret = srvGetServiceHandle(&context->servhandle, "http:C");
	if(R_FAILED(ret)) {
		HTTPC_CloseContext(__httpc_servhandle, context->httphandle);
		return ret;
        }

	ret = HTTPC_InitializeConnectionSession(context->servhandle, context->httphandle);
	if(R_FAILED(ret)) {
		svcCloseHandle(context->servhandle);
		HTTPC_CloseContext(__httpc_servhandle, context->httphandle);
		return ret;
        }

	if(use_defaultproxy==0)return 0;

	ret = HTTPC_SetProxyDefault(context->servhandle, context->httphandle);
	if(R_FAILED(ret)) {
		svcCloseHandle(context->servhandle);
		HTTPC_CloseContext(__httpc_servhandle, context->httphandle);
		return ret;
        }

	return 0;
}

Result httpcCloseContext(httpcContext *context)
{
	Result ret=0;

	svcCloseHandle(context->servhandle);
	ret = HTTPC_CloseContext(__httpc_servhandle, context->httphandle);

	return ret;
}

Result httpcCancelConnection(httpcContext *context)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x4,1,0); // 0x40040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcDownloadData(httpcContext *context, u8* buffer, u32 size, u32 *downloadedsize)
{
	Result ret=0;
	Result dlret=HTTPC_RESULTCODE_DOWNLOADPENDING;
	u32 pos=0, sz=0;
	u32 dlstartpos=0;
	u32 dlpos=0;

	if(downloadedsize)*downloadedsize = 0;

	ret=httpcGetDownloadSizeState(context, &dlstartpos, NULL);
	if(R_FAILED(ret))return ret;

	while(pos < size && dlret==HTTPC_RESULTCODE_DOWNLOADPENDING)
	{
		sz = size - pos;

		dlret=httpcReceiveData(context, &buffer[pos], sz);

		ret=httpcGetDownloadSizeState(context, &dlpos, NULL);
		if(R_FAILED(ret))return ret;

		pos = dlpos - dlstartpos;
	}

	if(downloadedsize)*downloadedsize = pos;

	return dlret;
}

static Result HTTPC_Initialize(Handle handle, u32 sharedmem_size, Handle sharedmem_handle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,1,4); // 0x10044
	cmdbuf[1]=sharedmem_size; // POST buffer size (page aligned)
	cmdbuf[2]=IPC_Desc_CurProcessHandle();
	cmdbuf[4]=IPC_Desc_SharedHandles(1);
	cmdbuf[5]=sharedmem_handle;// POST buffer memory block handle

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

static Result HTTPC_Finalize(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x39,0,0); // 0x390000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

static Result HTTPC_CreateContext(Handle handle, HTTPC_RequestMethod method, const char* url, Handle* contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 l=strlen(url)+1;

	cmdbuf[0]=IPC_MakeHeader(0x2,2,2); // 0x20082
	cmdbuf[1]=l;
	cmdbuf[2]=method;
	cmdbuf[3]=IPC_Desc_Buffer(l,IPC_BUFFER_R);
	cmdbuf[4]=(u32)url;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	if(contextHandle)*contextHandle=cmdbuf[2];

	return cmdbuf[1];
}

static Result HTTPC_InitializeConnectionSession(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x8,1,2); // 0x80042
	cmdbuf[1]=contextHandle;
	cmdbuf[2]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

static Result HTTPC_SetProxyDefault(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xE,1,0); // 0xE0040
	cmdbuf[1]=contextHandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

static Result HTTPC_CloseContext(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1]=contextHandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result httpcAddRequestHeaderField(httpcContext *context, const char* name, const char* value)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;
	int value_len=strlen(value)+1;

	cmdbuf[0]=IPC_MakeHeader(0x11,3,4); // 0x1100C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=value_len;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len,3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(value_len,IPC_BUFFER_R);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcAddPostDataAscii(httpcContext *context, const char* name, const char* value)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;
	int value_len=strlen(value)+1;

	cmdbuf[0]=IPC_MakeHeader(0x12,3,4); // 0x1200C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=value_len;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len,3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(value_len,IPC_BUFFER_R);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcAddPostDataBinary(httpcContext *context, const char* name, const u8* value, u32 len)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;
	
	cmdbuf[0]=IPC_MakeHeader(0x13, 3, 4); // 0x1300C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=len;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len,3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(len,IPC_BUFFER_R);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcAddPostDataRaw(httpcContext *context, const u32* data, u32 len)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x14, 2, 2); // 0x140082
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=len;
	cmdbuf[3]=IPC_Desc_Buffer(len, IPC_BUFFER_R);
	cmdbuf[4]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))
	{
		return ret;
	}
	return cmdbuf[1];
}

Result httpcBeginRequest(httpcContext *context)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcReceiveData(httpcContext *context, u8* buffer, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xB,2,2); // 0xB0082
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[4]=(u32)buffer;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcReceiveDataTimeout(httpcContext *context, u8* buffer, u32 size, u64 timeout)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xC,4,2); // 0xC0102
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=size;
	cmdbuf[3]=timeout & 0xffffffff;
	cmdbuf[4]=(timeout >> 32) & 0xffffffff;
	cmdbuf[5]=IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[6]=(u32)buffer;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcGetRequestState(httpcContext *context, HTTPC_RequestStatus* out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	*out = cmdbuf[2];

	return cmdbuf[1];
}

Result httpcGetDownloadSizeState(httpcContext *context, u32* downloadedsize, u32* contentsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x6,1,0); // 0x60040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	if(downloadedsize)*downloadedsize = cmdbuf[2];
	if(contentsize)*contentsize = cmdbuf[3];

	return cmdbuf[1];
}
Result httpcGetResponseHeader(httpcContext *context, const char* name, char* value, u32 valuebuf_maxsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;

	cmdbuf[0]=IPC_MakeHeader(0x1E,3,4); // 0x1E00C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=valuebuf_maxsize;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len, 3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(valuebuf_maxsize, IPC_BUFFER_W);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcGetResponseStatusCode(httpcContext *context, u32* out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x22,1,0); // 0x220040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	*out = cmdbuf[2];

	return cmdbuf[1];
}


Result httpcGetResponseStatusCodeTimeout(httpcContext *context, u32* out, u64 timeout)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x23,3,0); // 0x2300C0
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=timeout & 0xffffffff;
	cmdbuf[3]=(timeout >> 32) & 0xffffffff;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	*out = cmdbuf[2];

	return cmdbuf[1];
}

Result httpcAddTrustedRootCA(httpcContext *context, const u8 *cert, u32 certsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x24,2,2); // 0x240082
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=certsize;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcAddDefaultCert(httpcContext *context, SSLC_DefaultRootCert certID)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x25,2,0); // 0x250080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcSelectRootCertChain(httpcContext *context, u32 RootCertChain_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x26,2,0); // 0x260080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=RootCertChain_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcSetClientCert(httpcContext *context, const u8 *cert, u32 certsize, const u8 *privk, u32 privk_size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x27,3,4); // 0x2700C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=certsize;
	cmdbuf[3]=privk_size;
	cmdbuf[4]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[5]=(u32)cert;
	cmdbuf[6]=IPC_Desc_Buffer(privk_size, IPC_BUFFER_R);
	cmdbuf[7]=(u32)privk;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcSetClientCertDefault(httpcContext *context, SSLC_DefaultClientCert certID)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x28,2,0); // 0x280080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcSetClientCertContext(httpcContext *context, u32 ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x29,2,0); // 0x290080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=ClientCert_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcSetSSLOpt(httpcContext *context, u32 options)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2B,2,0); // 0x2B0080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=options;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcSetSSLClearOpt(httpcContext *context, u32 options)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2C,2,0); // 0x2C0080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=options;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcCreateRootCertChain(u32 *RootCertChain_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2D,0,0); // 0x2D0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && RootCertChain_contexthandle)*RootCertChain_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcDestroyRootCertChain(u32 RootCertChain_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2E,1,0); // 0x2E0040
	cmdbuf[1]=RootCertChain_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcRootCertChainAddCert(u32 RootCertChain_contexthandle, const u8 *cert, u32 certsize, u32 *cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2F,2,2); // 0x2F0082
	cmdbuf[1]=RootCertChain_contexthandle;
	cmdbuf[2]=certsize;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && cert_contexthandle)*cert_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcRootCertChainAddDefaultCert(u32 RootCertChain_contexthandle, SSLC_DefaultRootCert certID, u32 *cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x30,2,0); // 0x300080
	cmdbuf[1]=RootCertChain_contexthandle;
	cmdbuf[2]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && cert_contexthandle)*cert_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcRootCertChainRemoveCert(u32 RootCertChain_contexthandle, u32 cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x31,2,0); // 0x310080
	cmdbuf[1]=RootCertChain_contexthandle;
	cmdbuf[2]=cert_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcOpenClientCertContext(const u8 *cert, u32 certsize, const u8 *privk, u32 privk_size, u32 *ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x32,2,4); // 0x320084
	cmdbuf[1]=certsize;
	cmdbuf[2]=privk_size;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;
	cmdbuf[5]=IPC_Desc_Buffer(privk_size, IPC_BUFFER_R);
	cmdbuf[6]=(u32)privk;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && ClientCert_contexthandle)*ClientCert_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcOpenDefaultClientCertContext(SSLC_DefaultClientCert certID, u32 *ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x33,1,0); // 0x330040
	cmdbuf[1]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && ClientCert_contexthandle)*ClientCert_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcCloseClientCertContext(u32 ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x34,1,0); // 0x340040
	cmdbuf[1]=ClientCert_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpc_servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcSetKeepAlive(httpcContext *context, HTTPC_KeepAlive option)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x37,2,0); // 0x370080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=option;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}
