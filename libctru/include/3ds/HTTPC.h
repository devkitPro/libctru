#ifndef HTTPC_H
#define HTTPC_H

Result HTTPC_Initialize(Handle handle);
Result HTTPC_InitializeConnectionSession(Handle handle, Handle contextHandle);
Result HTTPC_CreateContext(Handle handle, char* url, Handle* contextHandle);
Result HTTPC_CloseContext(Handle handle, Handle contextHandle);
Result HTTPC_SetProxyDefault(Handle handle, Handle contextHandle);
Result HTTPC_AddRequestHeaderField(Handle handle, Handle contextHandle, char* name, char* value);
Result HTTPC_BeginRequest(Handle handle, Handle contextHandle);
Result HTTPC_ReceiveData(Handle handle, Handle contextHandle, u8* buffer, u32 size);

#endif
