#include <3ds.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/synchronization.h>
#include <3ds/services/apt.h>
#include <3ds/services/cfgu.h>
#include <3ds/util/utf.h>
#include <3ds/applets/error.h>

void errorInit(errorConf* err, errorType type, CFG_Language lang)
{
	memset(err, 0, sizeof(*err));
	err->type = type;
	err->useLanguage = lang;
	err->upperScreenFlag = ERROR_NORMAL;
	err->eulaVersion =  0;
	err->homeButton = true;
	err->softwareReset = false;
	err->appJump = false;
	err->returnCode = ERROR_UNKNOWN;
}

void errorCode(errorConf* err, int error)
{
	err->errorCode = error;
}

static void errorConvertToUTF16(u16* out, const char* in, size_t max)
{
	if (!in || !*in)
	{
		out[0] = 0;
		return;
	}

	ssize_t units = utf8_to_utf16(out, (const uint8_t*)in, max-1);
	if (units < 0)
	{
		out[0] = 0;
		return;
	}

	out[units] = 0;	
}

void errorText(errorConf *err, const char* text)
{
	errorConvertToUTF16(err->Text, text, 1900);
}

void errorDisp(errorConf* err)
{   
	aptLaunchLibraryApplet(APPID_ERROR, err, sizeof(*err), 0);
}
