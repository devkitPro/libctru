/**
 * @file browser.h
 * @brief Browser Applet
 */

#pragma once

/**
 * @brief Open the browser at a url
 * @param url The url to open at, max-length 1024 characters including the trailing zero. If invalid or null, just opens the browser.
 */
void browserOpenUrl(const char* url);
