/**
 * @file sslc.h
 * @brief SSLC(TLS) service. https://3dbrew.org/wiki/SSL_Services
 */
#pragma once

/// HTTP context.
typedef struct {
	Handle servhandle; ///< Service handle.
	u32 sslchandle;    ///< SSLC handle.
} sslcContext;

/// Initializes SSLC. Normally session_handle should be 0. When non-zero this will use the specified handle for the main-service-session without using the Initialize command, instead of using srvGetServiceHandle.
Result sslcInit(Handle session_handle);

/// Exits SSLC.
void sslcExit(void);

/**
 * @brief Adds a trusted RootCA cert to a RootCertChain.
 * @param RootCertChain_contexthandle RootCertChain to use.
 * @param cert Pointer to DER cert.
 * @param certsize Size of the DER cert.
 */
Result sslcAddTrustedRootCA(u32 RootCertChain_contexthandle, u8 *cert, u32 certsize);

