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
 * @param[in] redirStdout Whether to redirect stdout to 3dslink output.
 * @param[in] redirStderr Whether to redirect stderr to 3dslink output.
 * @return Socket fd on success, negative number on failure.
 * @note SOC must be initialized (\ref socInit or \ref socInitMulti) before calling this function.
 * @note The connection should be closed with link3dsDisconnectFromHost() during application cleanup.
 * @note Do not call \ref socExit until link3dsDisconnectFromHost() has been called.
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

/**
  * @brief Disconnects from the 3dslink host.
  * @note Do not call \ref socExit until this has been called.
  */
void link3dsDisconnectFromHost();