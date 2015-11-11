#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/httpc.h>
#include <3ds/ipc.h>

Handle __httpc_servhandle;
static int __httpc_refcount;

Result httpcInit(void)
{
	Result ret=0;

	if (AtomicPostIncrement(&__httpc_refcount)) return 0;

	ret = srvGetServiceHandle(&__httpc_servhandle, "http:C");
	if (R_SUCCEEDED(ret))
	{
		ret = HTTPC_Initialize(__httpc_servhandle);
		if (R_FAILED(ret)) svcCloseHandle(__httpc_servhandle);
	}
	if (R_FAILED(ret)) AtomicDecrement(&__httpc_refcount);

	return ret;
}

void httpcExit(void)
{
	if (AtomicDecrement(&__httpc_refcount)) return;
	svcCloseHandle(__httpc_servhandle);
}

Result httpcOpenContext(httpcContext *context, char* url, u32 use_defaultproxy)
{
	Result ret=0;

	ret = HTTPC_CreateContext(__httpc_servhandle, url, &context->httphandle);
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

Result httpcAddRequestHeaderField(httpcContext *context, char* name, char* value)
{
	return HTTPC_AddRequestHeaderField(context->servhandle, context->httphandle, name, value);
}

Result httpcBeginRequest(httpcContext *context)
{
	return HTTPC_BeginRequest(context->servhandle, context->httphandle);
}

Result httpcReceiveData(httpcContext *context, u8* buffer, u32 size)
{
	return HTTPC_ReceiveData(context->servhandle, context->httphandle, buffer, size);
}

Result httpcGetRequestState(httpcContext *context, HTTPC_RequestStatus* out)
{
	return HTTPC_GetRequestState(context->servhandle, context->httphandle, out);
}

Result httpcGetDownloadSizeState(httpcContext *context, u32* downloadedsize, u32* contentsize)
{
	return HTTPC_GetDownloadSizeState(context->servhandle, context->httphandle, downloadedsize, contentsize);
}

Result httpcGetResponseHeader(httpcContext *context, char* name, char* value, u32 valuebuf_maxsize)
{
	return HTTPC_GetResponseHeader(context->servhandle, context->httphandle, name, value, valuebuf_maxsize);
}

Result httpcGetResponseStatusCode(httpcContext *context, u32* out, u64 delay)
{
	return HTTPC_GetResponseStatusCode(context->servhandle, context->httphandle, out);
}

Result httpcDownloadData(httpcContext *context, u8* buffer, u32 size, u32 *downloadedsize)
{
	Result ret=0;
	u32 contentsize=0;
	u32 pos=0, sz=0;

	if(downloadedsize)*downloadedsize = 0;

	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(R_FAILED(ret))return ret;

	while(pos < size)
	{
		sz = size - pos;

		ret=httpcReceiveData(context, &buffer[pos], sz);

		if(ret==HTTPC_RESULTCODE_DOWNLOADPENDING)
		{
			ret=httpcGetDownloadSizeState(context, &pos, NULL);
			if(R_FAILED(ret))return ret;
		}
		else if(R_FAILED(ret))
		{
			return ret;
		}
		else
		{
			pos+= sz;
		}

		if(downloadedsize)*downloadedsize = pos;
	}

	return 0;
}

Result HTTPC_Initialize(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,1,4); // 0x10044
	cmdbuf[1]=0x1000; //unk
	cmdbuf[2]=IPC_Desc_CurProcessHandle();
	cmdbuf[4]=IPC_Desc_SharedHandles(1);
	cmdbuf[5]=0;//Some sort of handle.
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result HTTPC_CreateContext(Handle handle, char* url, Handle* contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 l=strlen(url)+1;

	cmdbuf[0]=IPC_MakeHeader(0x2,2,2); // 0x20082
	cmdbuf[1]=l;
	cmdbuf[2]=0x01; //unk
	cmdbuf[3]=IPC_Desc_Buffer(l,IPC_BUFFER_R);
	cmdbuf[4]=(u32)url;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;
	
	if(contextHandle)*contextHandle=cmdbuf[2];

	return cmdbuf[1];
}

Result HTTPC_InitializeConnectionSession(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x8,1,2); // 0x80042
	cmdbuf[1]=contextHandle;
	cmdbuf[2]=IPC_Desc_CurProcessHandle();
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result HTTPC_SetProxyDefault(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xE,1,0); // 0xE0040
	cmdbuf[1]=contextHandle;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result HTTPC_CloseContext(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1]=contextHandle;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result HTTPC_AddRequestHeaderField(Handle handle, Handle contextHandle, char* name, char* value)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;
	int value_len=strlen(value)+1;

	cmdbuf[0]=IPC_MakeHeader(0x11,3,4); // 0x1100C4
	cmdbuf[1]=contextHandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=value_len;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len,3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(value_len,IPC_BUFFER_R);
	cmdbuf[7]=(u32)value;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result HTTPC_BeginRequest(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1]=contextHandle;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result HTTPC_ReceiveData(Handle handle, Handle contextHandle, u8* buffer, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xB,2,2); // 0xB0082
	cmdbuf[1]=contextHandle;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[4]=(u32)buffer;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result HTTPC_GetRequestState(Handle handle, Handle contextHandle, HTTPC_RequestStatus* out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1]=contextHandle;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	*out = cmdbuf[2];

	return cmdbuf[1];
}

Result HTTPC_GetDownloadSizeState(Handle handle, Handle contextHandle, u32* downloadedsize, u32* contentsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x6,1,0); // 0x60040
	cmdbuf[1]=contextHandle;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	if(downloadedsize)*downloadedsize = cmdbuf[2];
	if(contentsize)*contentsize = cmdbuf[3];

	return cmdbuf[1];
}

Result HTTPC_GetResponseHeader(Handle handle, Handle contextHandle, char* name, char* value, u32 valuebuf_maxsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;

	cmdbuf[0]=IPC_MakeHeader(0x1E,3,4); // 0x1E00C4
	cmdbuf[1]=contextHandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=valuebuf_maxsize;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len, 3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(valuebuf_maxsize, IPC_BUFFER_W);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result HTTPC_GetResponseStatusCode(Handle handle, Handle contextHandle, u32* out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x22,1,0); // 0x220040
	cmdbuf[1]=contextHandle;
	
	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	*out = cmdbuf[2];

	return cmdbuf[1];
}

