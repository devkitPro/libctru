#pragma once

Result srvInit(void);
Result srvExit(void);
Handle *srvGetSessionHandle(void);
Result srvRegisterClient(void);
Result srvGetServiceHandleDirect(Handle* out, const char* name);
Result srvGetServiceHandle(Handle* out, const char* name);
Result srvRegisterService(Handle* out, const char* name, int maxSessions);
Result srvUnregisterService(const char* name);

Result srvPmInit(void);
Result srvRegisterProcess(u32 procid, u32 count, void *serviceaccesscontrol);
Result srvUnregisterProcess(u32 procid);
