#include <3ds.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/services/apt.h>
#include <3ds/ipc.h>
#include <3ds/env.h>
#include <3ds/util/utf.h>
#include <3ds/applets/error.h>

void error_Init(ErrConf* err, ErrorType type, err_lang lang)
{
	memset(err, 0, sizeof(*err));
	err->errorType=         type;
	err->useLanguage=       lang;
	err->upperScreenFlag= NORMAL;
	err->eulaVersion =         0;
	err->homeButton  =      true;
  err->softwareReset =   false;
  err->appJump     =     false;
  err-> returnCode   = UNKNOWN;
}

static void errorConvertToUTF16(u16* out, const char* in, int max)
{
	if (!in || !*in)
	{
		out[0] = 0;
		return;
	}

	ssize_t units = utf8_to_utf16(out, (const uint8_t*)in, max);
	if (units < 0)
	{
		out[0] = 0;
		return;
	}

	out[units] = 0;
}
char k[1900];
char *c_shift(char text[])
{ 
  int i=strlen(text);
  char tex[i];
  tex[0]=text[0];
  int j=1;
  for(int a=0;a<=i;a++)
  {
	  tex[j]=text[a];
	  j++;
  }
	strncpy(k, tex, i+1);
	return k;
}
void error_code(ErrConf* err,int error)
{
	err->errorCode = error;
}

void error_text(ErrConf *err, char* text)
{   
    char *tex=c_shift(text);
	  errorConvertToUTF16(err->Text, tex,1900);
}

void error_disp(ErrConf* err)
{   
	 aptLaunchLibraryApplet(APPID_ERROR , err, sizeof(*err), 0);
}
