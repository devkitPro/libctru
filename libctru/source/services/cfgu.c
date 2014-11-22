#include <stdlib.h>
#include <3ds.h>

static Handle CFGU_handle = 0;

Result initCfgu()
{
    return srvGetServiceHandle(&CFGU_handle, "cfg:u");
}

Result exitCfgu()
{
    Result ret = svcCloseHandle(CFGU_handle);
    CFGU_handle = 0;

    return ret;
}

Result CFGU_GetSystemModel(u8* model)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = 0x00050000;

    if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

    *model = (u8)cmdbuf[2];

    return (Result)cmdbuf[1];
}
