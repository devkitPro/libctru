#pragma once

Result srvInit();
Result srvExit();
Result srvRegisterClient();
Result srvGetServiceHandle(Handle* out, char* name);
