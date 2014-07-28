/*
  srv.c _ Service manager.
*/

#include <3ds/types.h>
#include <3ds/srv.h>
#include <3ds/svc.h>


static Handle g_srv_handle = 0;

Result srvInit()
{
    Result rc = 0;

    if(rc = svcConnectToPort(&g_srv_handle, "srv:"))
        return rc;

    if(rc = srvRegisterClient()) {
        svcCloseHandle(g_srv_handle);
        g_srv_handle = 0;
    }

    return rc;
}

Result srvExit()
{
    if(g_srv_handle != 0)
        svcCloseHandle(g_srv_handle);

    g_srv_handle = 0;
}

Result srvRegisterClient()
{
    u32* cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0x10002;
    cmdbuf[1] = 0x20;

    Result rc;
    if(rc = svcSendSyncRequest(g_srv_handle))
        return rc;

    return cmdbuf[1];
}

Result srvGetServiceHandle(Handle* out, char* name)
{
    u8 len = strlen(name);
    Result rc;

    u32* cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = 0x50100;
    strcpy((char*) &cmdbuf[1], name);
    cmdbuf[3] = len;
    cmdbuf[4] = 0x0;

    if(rc = svcSendSyncRequest(g_srv_handle))
        return rc;

    *out = cmdbuf[3];
    return cmdbuf[1];
}
