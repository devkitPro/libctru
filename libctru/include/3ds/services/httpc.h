/**
 * @file httpc.h
 * @brief HTTP service.
 */
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

Result httpcInit(void);
void httpcExit(void);

/**
 * @brief Opens an HTTP context.
 * @param context Context to open.
 * @param url URL to connect to.
 * @param use_defaultproxy Whether the default proxy should be used (0 for default)
 */
Result httpcOpenContext(httpcContext *context, char* url, u32 use_defaultproxy);
Result httpcCloseContext(httpcContext *context);
Result httpcAddRequestHeaderField(httpcContext *context, char* name, char* value);
Result httpcBeginRequest(httpcContext *context);
Result httpcReceiveData(httpcContext *context, u8* buffer, u32 size);
Result httpcGetRequestState(httpcContext *context, httpcReqStatus* out);
Result httpcGetDownloadSizeState(httpcContext *context, u32* downloadedsize, u32* contentsize);
/**
 * @brief Gets the response code of the HTTP context.
 * @param context Context to get the response code of.
 * @param out Pointer to write the response code to.
 * @param delay Delay to wait for the status code. Not used yet.
 */
Result httpcGetResponseStatusCode(httpcContext *context, u32* out, u64 delay);
Result httpcGetResponseHeader(httpcContext *context, char* name, char* value, u32 valuebuf_maxsize);
/**
 * @brief Downloads data from the HTTP context into a buffer.
 * The *entire* content must be downloaded before using httpcCloseContext(), otherwise httpcCloseContext() will hang.
 * @param context Context to download data from.
 * @param buffer Buffer to write data to.
 * @param size Size of the buffer.
 * @param downloadedsize Pointer to write the size of the downloaded data to.
 */
Result httpcDownloadData(httpcContext *context, u8* buffer, u32 size, u32 *downloadedsize);

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
Result HTTPC_GetResponseHeader(Handle handle, Handle contextHandle, char* name, char* value, u32 valuebuf_maxsize);
Result HTTPC_GetResponseStatusCode(Handle handle, Handle contextHandle, u32* out);

