#include <3ds/synchronization.h>
#include <3ds/services/frdn.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

static Handle frdnHandle;
static int frdnRefCount;

Result frdnInit()
{
    Result ret = 0;

    if (AtomicPostIncrement(&frdnRefCount)) return 0;

    if (R_FAILED(ret = srvGetServiceHandle(&frdnHandle, "frd:n")))
        AtomicDecrement(&frdnRefCount);

    return ret;
}

void frdnExit()
{
    if (AtomicDecrement(&frdnRefCount)) return;
    svcCloseHandle(frdnHandle);
}

Handle *frdnGetSessionHandle(void)
{
    return &frdnHandle;
}

Result FRDN_GetHandleOfNdmStatusChangedEvent(Handle *evt)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(0x1,0,0); // 0x10000

    if (R_FAILED(ret = svcSendSyncRequest(frdnHandle))) return ret;

    *evt = cmdbuf[3];

    return (Result)cmdbuf[1];
}

Result FRDN_Resume()
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

    if (R_FAILED(ret = svcSendSyncRequest(frdnHandle))) return ret;

    return (Result)cmdbuf[1];
}

Result FRDN_SuspendAsync(bool immediately)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(0x3,0,0); // 0x30040
    cmdbuf[1] = (u32)immediately;

    if (R_FAILED(ret = svcSendSyncRequest(frdnHandle))) return ret;

    return (Result)cmdbuf[1];
}

Result FRDN_QueryStatus(u8 *status)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

    if (R_FAILED(ret = svcSendSyncRequest(frdnHandle))) return ret;

    *status = (u8)cmdbuf[2];

    return (Result)cmdbuf[1];
}