#ifndef SRV_H
#define SRV_H

#define SRV_OVERRIDE_SUPPORT

Result srvInit();
Result srvExit();
Result srvRegisterClient();
Result srvGetServiceHandle(Handle* out, char* name);

#endif
