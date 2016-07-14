#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/errf.h>
#include <3ds/ipc.h>
#include <3ds/env.h>
#include "internal.h"

static Handle errfHandle;
static int errfRefCount;

Result errfInit(void)
{
	Result rc = 0;

	if (AtomicPostIncrement(&errfRefCount)) return 0;

	rc = svcConnectToPort(&errfHandle, "err:f");
	if (R_FAILED(rc)) goto end;

end:
	if (R_FAILED(rc)) errfExit();
	return rc;
}

void errfExit(void)
{
	if (AtomicDecrement(&errfRefCount))
		return;
	svcCloseHandle(errfHandle);
}

Handle* errfGetSessionHandle(void)
{
	return &errfHandle;
}

Result ERRF_Throw(ERRF_FatalErrInfo *error)
{
	uint32_t *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x10800;
	memcpy(&cmdbuf[1], error, sizeof(ERRF_FatalErrInfo));

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(errfHandle)))
		return ret;

	return cmdbuf[1];
}
