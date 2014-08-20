#ifndef SRV_H
#define SRV_H

Result srvInit();
Result srvExit();
Result srvRegisterClient();
Result srvGetServiceHandle(Handle* out, char* name);

#endif
