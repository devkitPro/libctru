/**
 * @file 3dslink.h
 * @brief Netloader (3dslink) utilities
 */

#pragma once

#include <stdbool.h>

struct in_addr;

/// Address of the host connected through 3dslink
extern struct in_addr __3dslink_host;

#define LINK3DS_COMM_PORT 17491 ///< 3dslink TCP server port

/**
 * @brief Connects to the 3dslink host, setting up an output stream.
 * @param[in] redirStdout Whether to redirect stdout to nxlink output.
 * @param[in] redirStderr Whether to redirect stderr to nxlink output.
 * @return Socket fd on success, negative number on failure.
 * @note The socket should be closed with close() during application cleanup.
 */
int link3dsConnectToHost(bool redirStdout, bool redirStderr);

/// Same as \ref link3dsConnectToHost but redirecting both stdout/stderr.
static inline int link3dsStdio(void) {
    return link3dsConnectToHost(true, true);
}

/// Same as \ref link3dsConnectToHost but redirecting only stderr.
static inline int link3dsStdioForDebug(void) {
    return link3dsConnectToHost(false, true);
}
