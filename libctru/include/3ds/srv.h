#pragma once

Result srvInit();
Result srvExit();
Result srvRegisterClient();
Result srvGetServiceHandle(Handle* out, const char* name);

Result srvPmInit();
Result srvRegisterProcess(u32 procid, u32 count, void *serviceaccesscontrol);
Result srvUnregisterProcess(u32 procid);
