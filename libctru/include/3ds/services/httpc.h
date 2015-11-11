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

/// HTTP request status.
typedef enum {
	HTTPC_STATUS_REQUEST_IN_PROGRESS = 0x5, ///< Request in progress.
	HTTPC_STATUS_DOWNLOAD_READY = 0x7       ///< Download ready.
} HTTPC_RequestStatus;

/// Result code returned when a download is pending.
#define HTTPC_RESULTCODE_DOWNLOADPENDING 0xd840a02b

/// Initializes HTTPC.
Result httpcInit(void);

/// Exits HTTPC.
void httpcExit(void);

/**
 * @brief Opens a HTTP context.
 * @param context Context to open.
 * @param url URL to connect to.
 * @param use_defaultproxy Whether the default proxy should be used (0 for default)
 */
Result httpcOpenContext(httpcContext *context, char* url, u32 use_defaultproxy);

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
Result HTTPC_Initialize(Handle handle);

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
Result HTTPC_CreateContext(Handle handle, char* url, Handle* contextHandle);

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

