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

/// HTTP KeepAlive option.
typedef enum {
	HTTPC_KEEPALIVE_DISABLED = 0x0,
	HTTPC_KEEPALIVE_ENABLED = 0x1
} HTTPC_KeepAlive;

/// Result code returned when a download is pending.
#define HTTPC_RESULTCODE_DOWNLOADPENDING 0xd840a02b

// Result code returned when asked about a non-existing header.
#define HTTPC_RESULTCODE_NOTFOUND 0xd840a028

// Result code returned when any timeout function times out.
#define HTTPC_RESULTCODE_TIMEDOUT 0xd820a069

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
Result httpcOpenContext(httpcContext *context, HTTPC_RequestMethod method, const char* url, u32 use_defaultproxy);

/**
 * @brief Closes a HTTP context.
 * @param context Context to close.
 */
Result httpcCloseContext(httpcContext *context);

/**
 * @brief Cancels a HTTP connection.
 * @param context Context to close.
 */
Result httpcCancelConnection(httpcContext *context);

/**
 * @brief Adds a request header field to a HTTP context.
 * @param context Context to use.
 * @param name Name of the field.
 * @param value Value of the field.
 */
Result httpcAddRequestHeaderField(httpcContext *context, const char* name, const char* value);

/**
 * @brief Adds a POST form field to a HTTP context.
 * @param context Context to use.
 * @param name Name of the field.
 * @param value Value of the field.
 */
Result httpcAddPostDataAscii(httpcContext *context, const char* name, const char* value);

/**
 * @brief Adds a POST form field with binary data to a HTTP context.
 * @param context Context to use.
 * @param name Name of the field.
 * @param value The binary data to pass as a value.
 * @param len Length of the binary data which has been passed.
 */
Result httpcAddPostDataBinary(httpcContext *context, const char* name, const u8* value, u32 len);
	
/**
 * @brief Adds a POST body to a HTTP context.
 * @param context Context to use.
 * @param data The data to be passed as raw into the body of the post request.
 * @param len Length of data passed by data param.
 */
Result httpcAddPostDataRaw(httpcContext *context, const u32* data, u32 len);

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
 * @brief Receives data from a HTTP context with a timeout value.
 * @param context Context to use.
 * @param buffer Buffer to receive data to.
 * @param size Size of the buffer.
 * @param timeout Maximum time in nanoseconds to wait for a reply.
 */
Result httpcReceiveDataTimeout(httpcContext *context, u8* buffer, u32 size, u64 timeout);

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
 */
Result httpcGetResponseStatusCode(httpcContext *context, u32* out);

/**
 * @brief Gets the response code of the HTTP context with a timeout value.
 * @param context Context to get the response code of.
 * @param out Pointer to write the response code to.
 * @param timeout Maximum time in nanoseconds to wait for a reply.
 */
Result httpcGetResponseStatusCodeTimeout(httpcContext *context, u32* out, u64 timeout);

/**
 * @brief Gets a response header field from a HTTP context.
 * @param context Context to use.
 * @param name Name of the field.
 * @param value Pointer to output the value of the field to.
 * @param valuebuf_maxsize Maximum size of the value buffer.
 */
Result httpcGetResponseHeader(httpcContext *context, const char* name, char* value, u32 valuebuf_maxsize);

/**
 * @brief Adds a trusted RootCA cert to a HTTP context.
 * @param context Context to use.
 * @param cert Pointer to DER cert.
 * @param certsize Size of the DER cert.
 */
Result httpcAddTrustedRootCA(httpcContext *context, const u8 *cert, u32 certsize);

/**
 * @brief Adds a default RootCA cert to a HTTP context.
 * @param context Context to use.
 * @param certID ID of the cert to add, see sslc.h.
 */
Result httpcAddDefaultCert(httpcContext *context, SSLC_DefaultRootCert certID);

/**
 * @brief Sets the RootCertChain for a HTTP context.
 * @param context Context to use.
 * @param RootCertChain_contexthandle Contexthandle for the RootCertChain.
 */
Result httpcSelectRootCertChain(httpcContext *context, u32 RootCertChain_contexthandle);

/**
 * @brief Sets the ClientCert for a HTTP context.
 * @param context Context to use.
 * @param cert Pointer to DER cert.
 * @param certsize Size of the DER cert.
 * @param privk Pointer to the DER private key.
 * @param privk_size Size of the privk.
 */
Result httpcSetClientCert(httpcContext *context, const u8 *cert, u32 certsize, const u8 *privk, u32 privk_size);

/**
 * @brief Sets the default clientcert for a HTTP context.
 * @param context Context to use.
 * @param certID ID of the cert to add, see sslc.h.
 */
Result httpcSetClientCertDefault(httpcContext *context, SSLC_DefaultClientCert certID);

/**
 * @brief Sets the ClientCert contexthandle for a HTTP context.
 * @param context Context to use.
 * @param ClientCert_contexthandle Contexthandle for the ClientCert.
 */
Result httpcSetClientCertContext(httpcContext *context, u32 ClientCert_contexthandle);

/**
 * @brief Sets SSL options for the context.
 * The HTTPC SSL option bits are the same as those defined in sslc.h
 * @param context Context to set flags on.
 * @param options SSL option flags.
 */
Result httpcSetSSLOpt(httpcContext *context, u32 options);

/**
 * @brief Sets the SSL options which will be cleared for the context.
 * The HTTPC SSL option bits are the same as those defined in sslc.h
 * @param context Context to clear flags on.
 * @param options SSL option flags.
 */
Result httpcSetSSLClearOpt(httpcContext *context, u32 options);

/**
 * @brief Creates a RootCertChain. Up to 2 RootCertChains can be created under this user-process.
 * @param RootCertChain_contexthandle Output RootCertChain contexthandle.
 */
Result httpcCreateRootCertChain(u32 *RootCertChain_contexthandle);

/**
 * @brief Destroy a RootCertChain.
 * @param RootCertChain_contexthandle RootCertChain to use.
 */
Result httpcDestroyRootCertChain(u32 RootCertChain_contexthandle);

/**
 * @brief Adds a RootCA cert to a RootCertChain.
 * @param RootCertChain_contexthandle RootCertChain to use.
 * @param cert Pointer to DER cert.
 * @param certsize Size of the DER cert.
 * @param cert_contexthandle Optional output ptr for the cert contexthandle(this can be NULL).
 */
Result httpcRootCertChainAddCert(u32 RootCertChain_contexthandle, const u8 *cert, u32 certsize, u32 *cert_contexthandle);

/**
 * @brief Adds a default RootCA cert to a RootCertChain.
 * @param RootCertChain_contexthandle RootCertChain to use.
 * @param certID ID of the cert to add, see sslc.h.
 * @param cert_contexthandle Optional output ptr for the cert contexthandle(this can be NULL).
 */
Result httpcRootCertChainAddDefaultCert(u32 RootCertChain_contexthandle, SSLC_DefaultRootCert certID, u32 *cert_contexthandle);

/**
 * @brief Removes a cert from a RootCertChain.
 * @param RootCertChain_contexthandle RootCertChain to use.
 * @param cert_contexthandle Contexthandle of the cert to remove.
 */
Result httpcRootCertChainRemoveCert(u32 RootCertChain_contexthandle, u32 cert_contexthandle);

/**
 * @brief Opens a ClientCert-context. Up to 2 ClientCert-contexts can be open under this user-process.
 * @param cert Pointer to DER cert.
 * @param certsize Size of the DER cert.
 * @param privk Pointer to the DER private key.
 * @param privk_size Size of the privk.
 * @param ClientCert_contexthandle Output ClientCert context handle.
 */
Result httpcOpenClientCertContext(const u8 *cert, u32 certsize, const u8 *privk, u32 privk_size, u32 *ClientCert_contexthandle);

/**
 * @brief Opens a ClientCert-context with a default clientclient. Up to 2 ClientCert-contexts can be open under this user-process.
 * @param certID ID of the cert to add, see sslc.h.
 * @param ClientCert_contexthandle Output ClientCert context handle.
 */
Result httpcOpenDefaultClientCertContext(SSLC_DefaultClientCert certID, u32 *ClientCert_contexthandle);

/**
 * @brief Closes a ClientCert context.
 * @param ClientCert_contexthandle ClientCert context to use.
 */
Result httpcCloseClientCertContext(u32 ClientCert_contexthandle);

/**
 * @brief Downloads data from the HTTP context into a buffer.
 * The *entire* content must be downloaded before using httpcCloseContext(), otherwise httpcCloseContext() will hang.
 * @param context Context to download data from.
 * @param buffer Buffer to write data to.
 * @param size Size of the buffer.
 * @param downloadedsize Pointer to write the size of the downloaded data to.
 */
Result httpcDownloadData(httpcContext *context, u8* buffer, u32 size, u32 *downloadedsize);

/**
 * @brief Sets Keep-Alive for the context.
 * @param context Context to set the KeepAlive flag on.
 * @param option HTTPC_KeepAlive option.
 */
Result httpcSetKeepAlive(httpcContext *context, HTTPC_KeepAlive option);

