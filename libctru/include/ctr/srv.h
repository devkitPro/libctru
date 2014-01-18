#ifndef SRV_H
#define SRV_H


Result srv_10002(Handle handle);
void getSrvHandle(Handle* out);
void srv_getServiceHandle(Handle handle, Handle* out, char* server);

#endif
