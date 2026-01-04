#include <3ds/types.h>
#include <3ds/services/apt.h>

#include <3ds/applets/browser.h>

#include <stdlib.h>
#include <string.h>

void browserOpenUrl(const char* url)
{
	size_t url_len = url ? (strlen(url) + 1) : 0;
	// check that we passed a url, it isn't too long and not too short (needs at least "http://")
	// if that is not the case, we just claim our buffer was 0 in size
	if (url_len > 0x400 || url_len < 8) url_len = 0;
	// Something (aptLaunchSystemApplet? webbrowser?) tries to write / trashes the buffer.
	// So, we copy the url onto the heap so that all still works out and our original argument remains const
	void* buffer = malloc(url_len);
	if (buffer) memcpy(buffer, url, url_len);
	aptLaunchSystemApplet(APPID_WEB, buffer, url_len, 0);
	if (buffer) free(buffer);
}
