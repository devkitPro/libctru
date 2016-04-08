/**
 * @file httpc.h
 * @brief HTTP service.
 */
#pragma once

/// HTTP context.
typedef struct {
	Handle servhandle; ///< Service handle.
	u32 httphandle;    ///< HTTP handle.
} httpcContext;

/// HTTP request method.
typedef enum {
	HTTPC_METHOD_GET = 0x1,
	HTTPC_METHOD_POST = 0x2,
	HTTPC_METHOD_HEAD = 0x3,
	HTTPC_METHOD_PUT = 0x4,
	HTTPC_METHOD_DELETE = 0x5
} HTTPC_RequestMethod;

/// HTTP request status.
typedef enum {
	HTTPC_STATUS_REQUEST_IN_PROGRESS = 0x5, ///< Request in progress.
	HTTPC_STATUS_DOWNLOAD_READY = 0x7       ///< Download ready.
} HTTPC_RequestStatus;

/// Result code returned when a download is pending.
#define HTTPC_RESULTCODE_DOWNLOADPENDING 0xd840a02b

// Result code returned when asked about a non-existing header
#define HTTPC_RESULTCODE_NOTFOUND 0xd840a028

/// Initializes HTTPC. For HTTP GET the sharedmem_size can be zero. The sharedmem contains data which will be later uploaded for HTTP POST. sharedmem_size should be aligned to 0x1000-bytes.
Result httpcInit(u32 sharedmem_size);

/// Exits HTTPC.
void httpcExit(void);

/**
 * @brief Opens a HTTP context.
 * @param context Context to open.
 * @param url URL to connect to.
 * @param use_defaultproxy Whether the default proxy should be used (0 for default)
 */
Result httpcOpenContext(httpcContext *context, HTTPC_RequestMethod method, char* url, u32 use_defaultproxy);

/**
 * @brief Closes a HTTP context.
 * @param context Context to close.
 */
Result httpcCloseContext(httpcContext *context);

/**
 * @brief Adds a request header field to a HTTP context.
 * @param context Context to use.
 * @param name Name of the field.
 * @param value Value of the field.
 */
Result httpcAddRequestHeaderField(httpcContext *context, char* name, char* value);

/**
 * @brief Adds a POST form field to a HTTP context.
 * @param context Context to use.
 * @param name Name of the field.
 * @param value Value of the field.
 */
Result httpcAddPostDataAscii(httpcContext *context, char* name, char* value);

/**
 * @brief Adds a POST body to a HTTP context.
 * @param context Context to use.
 * @param data The data to be passed as raw into the body of the post request.
 * @param len Length of data passed by data param.
 */
Result httpcAddPostDataRaw(httpcContext *context, u32* data, u32 len);

/**
 * @brief Begins a HTTP request.
 * @param context Context to use.
 */
Result httpcBeginRequest(httpcContext *context);

/**
 * @brief Receives data from a HTTP context.
 * @param context Context to use.
 * @param buffer Buffer to receive data to.
 * @param size Size of the buffer.
 */
Result httpcReceiveData(httpcContext *context, u8* buffer, u32 size);

/**
 * @brief Gets the request state of a HTTP context.
 * @param context Context to use.
 * @param out Pointer to output the HTTP request state to.
 */
Result httpcGetRequestState(httpcContext *context, HTTPC_RequestStatus* out);

/**
 * @brief Gets the download size state of a HTTP context.
 * @param context Context to use.
 * @param downloadedsize Pointer to output the downloaded size to.
 * @param contentsize Pointer to output the total content size to.
 */
Result httpcGetDownloadSizeState(httpcContext *context, u32* downloadedsize, u32* contentsize);

/**
 * @brief Gets the response code of the HTTP context.
 * @param context Context to get the response code of.
 * @param out Pointer to write the response code to.
 * @param delay Delay to wait for the status code. Not used yet.
 */
Result httpcGetResponseStatusCode(httpcContext *context, u32* out, u64 delay);

/**
 * @brief Gets a response header field from a HTTP context.
 * @param context Context to use.
 * @param name Name of the field.
 * @param value Pointer to output the value of the field to.
 * @param valuebuf_maxsize Maximum size of the value buffer.
 */
Result httpcGetResponseHeader(httpcContext *context, char* name, char* value, u32 valuebuf_maxsize);

/**
 * @brief Adds a trusted RootCA cert to a HTTP context.
 * @param context Context to use.
 * @param cert Pointer to DER cert.
 * @param certsize Size of the DER cert.
 */
Result httpcAddTrustedRootCA(httpcContext *context, u8 *cert, u32 certsize);

/**
 * @brief Sets SSL options for the context.
 * The HTTPC SSL option bits are the same as those defined in sslc.h
 * @param contect Context to set flags on.
 * @param options SSL option flags.
 */
Result httpcSetSSLOpt(httpcContext *context, u32 options);

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

/**
 * @brief Initializes HTTPC.
 * @param handle HTTPC service handle to use.
 */
Result HTTPC_Initialize(Handle handle, u32 sharedmem_size, Handle sharedmem_handle);

/**
 * @brief Finalizes HTTPC.
 * @param handle HTTPC service handle to use.
 */
Result HTTPC_Finalize(Handle handle);

/**
 * @brief Initializes a HTTP connection session.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 */
Result HTTPC_InitializeConnectionSession(Handle handle, Handle contextHandle);

/**
 * @brief Creates a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param url URL to connect to.
 * @param contextHandle Pointer to output the created HTTP context handle to.
 */
Result HTTPC_CreateContext(Handle handle, HTTPC_RequestMethod method, char* url, Handle* contextHandle);

/**
 * @brief Closes a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 */
Result HTTPC_CloseContext(Handle handle, Handle contextHandle);

/**
 * @brief Applies the default proxy to a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 */
Result HTTPC_SetProxyDefault(Handle handle, Handle contextHandle);

/**
 * @brief Adds a request header field to a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param name Name of the field.
 * @param value of the field.
 */
Result HTTPC_AddRequestHeaderField(Handle handle, Handle contextHandle, char* name, char* value);

/**
 * @brief Adds a POST form field to a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param name Name of the field.
 * @param value of the field.
 */
Result HTTPC_AddPostDataAscii(Handle handle, Handle contextHandle, char* name, char* value);

/**
 * @brief Adds a POST body to a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param data Data to be passed as raw into the body of the post request.
 * @param len Length of data passed by data param.
 */
Result HTTPC_AddPostDataRaw(Handle handle, Handle contextHandle, u32* data, u32 len);

/**
 * @brief Begins a HTTP request.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 */
Result HTTPC_BeginRequest(Handle handle, Handle contextHandle);

/**
 * @brief Receives data from a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param buffer Buffer to receive data to.
 * @param size Size of the buffer.
 */
Result HTTPC_ReceiveData(Handle handle, Handle contextHandle, u8* buffer, u32 size);

/**
 * @brief Gets the request state of a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param out Pointer to output the request state to.
 */
Result HTTPC_GetRequestState(Handle handle, Handle contextHandle, HTTPC_RequestStatus* out);

/**
 * @brief Gets the download size state of a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param downloadedsize Pointer to output the downloaded size to.
 * @param contentsize Pointer to output the total content size to.
 */
Result HTTPC_GetDownloadSizeState(Handle handle, Handle contextHandle, u32* downloadedsize, u32* contentsize);

/**
 * @brief Gets a response header field from a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param name Name of the field.
 * @param value Pointer to output the value of the field to.
 * @param valuebuf_maxsize Maximum size of the value buffer.
 */
Result HTTPC_GetResponseHeader(Handle handle, Handle contextHandle, char* name, char* value, u32 valuebuf_maxsize);

/**
 * @brief Gets the status code of a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param out Pointer to output the status code to.
 */
Result HTTPC_GetResponseStatusCode(Handle handle, Handle contextHandle, u32* out);

/**
 * @brief Adds a trusted RootCA cert to a HTTP context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param cert Pointer to DER cert.
 * @param certsize Size of the DER cert.
 */
Result HTTPC_AddTrustedRootCA(Handle handle, Handle contextHandle, u8 *cert, u32 certsize);

/**
 * @brief Sets SSL options for the context.
 * @param handle HTTPC service handle to use.
 * @param contextHandle HTTP context handle to use.
 * @param options SSL option flags.
 */
Result HTTPC_SetSSLOpt(Handle handle, Handle contextHandle, u32 options);

