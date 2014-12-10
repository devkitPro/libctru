#pragma once

typedef struct {
	Handle servhandle;
	u32 httphandle;
} httpcContext;

typedef enum{
	HTTPCREQSTAT_INPROGRESS_REQSENT = 0x5,
	HTTPCREQSTAT_DLREADY = 0x7
} httpcReqStatus;

#define HTTPC_RESULTCODE_DOWNLOADPENDING 0xd840a02b

Result httpcInit();
void httpcExit();

Result httpcOpenContext(httpcContext *context, char* url, u32 use_defaultproxy);//use_defaultproxy should be zero normally, unless you don't want HTTPC_SetProxyDefault() to be used automatically.
Result httpcCloseContext(httpcContext *context);
Result httpcBeginRequest(httpcContext *context);
Result httpcReceiveData(httpcContext *context, u8* buffer, u32 size);
Result httpcGetRequestState(httpcContext *context, httpcReqStatus* out);
Result httpcGetDownloadSizeState(httpcContext *context, u32* downloadedsize, u32* contentsize);
Result httpcGetResponseStatusCode(httpcContext *context, u32* out, u64 delay);//delay isn't used yet. This writes the HTTP status code from the server to out.

Result httpcDownloadData(httpcContext *context, u8* buffer, u32 size, u32 *downloadedsize);//The *entire* content must be downloaded before using httpcCloseContext(), otherwise httpcCloseContext() will hang.

//Using the below functions directly is not recommended, use the above functions. See also the http example.

Result HTTPC_Initialize(Handle handle);
Result HTTPC_InitializeConnectionSession(Handle handle, Handle contextHandle);
Result HTTPC_CreateContext(Handle handle, char* url, Handle* contextHandle);
Result HTTPC_CloseContext(Handle handle, Handle contextHandle);
Result HTTPC_SetProxyDefault(Handle handle, Handle contextHandle);
Result HTTPC_AddRequestHeaderField(Handle handle, Handle contextHandle, char* name, char* value);
Result HTTPC_BeginRequest(Handle handle, Handle contextHandle);
Result HTTPC_ReceiveData(Handle handle, Handle contextHandle, u8* buffer, u32 size);
Result HTTPC_GetRequestState(Handle handle, Handle contextHandle, httpcReqStatus* out);
Result HTTPC_GetDownloadSizeState(Handle handle, Handle contextHandle, u32* downloadedsize, u32* contentsize);
Result HTTPC_GetResponseStatusCode(Handle handle, Handle contextHandle, u32* out);

