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

Result ERRF_Throw(const ERRF_FatalErrInfo* error)
{
	uint32_t *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,32,0); // 0x10800
	memcpy(&cmdbuf[1], error, sizeof(ERRF_FatalErrInfo));

	Result ret = 0;
	if (R_FAILED(ret = svcSendSyncRequest(errfHandle)))
		return ret;

	return cmdbuf[1];
}

static inline void getCommonErrorData(ERRF_FatalErrInfo* error, Result failure)
{
	error->resCode = failure;
	svcGetProcessId(&error->procId, 0xFFFF8001);
}

Result ERRF_ThrowResult(Result failure)
{
	ERRF_FatalErrInfo error;
	Result ret;

	if (R_FAILED(ret = errfInit()))
		return ret;

	memset(&error, 0, sizeof(error));

	error.type = ERRF_ERRTYPE_GENERIC;

	// pcAddr is not used by ErrDisp for ERRF_ERRTYPE_FAILURE
	error.pcAddr = (u32)__builtin_extract_return_addr(__builtin_return_address(0));
	getCommonErrorData(&error, failure);

	ret = ERRF_Throw(&error);

	errfExit();

	return ret;
}

Result ERRF_ThrowResultWithMessage(Result failure, const char* message)
{
	ERRF_FatalErrInfo error;
	Result ret;
	size_t msglen;

	if (R_FAILED(ret = errfInit()))
		return ret;

	memset(&error, 0, sizeof(error));

	error.type = ERRF_ERRTYPE_FAILURE;
	getCommonErrorData(&error, failure);

	if ((msglen = strlen(message)) > sizeof(error.data.failure_mesg) - 1)
		msglen = sizeof(error.data.failure_mesg) - 1;

	memcpy(error.data.failure_mesg, message, msglen);
	error.data.failure_mesg[msglen] = '\0';

	ret = ERRF_Throw(&error);

	errfExit();

	return ret;
}

void ERRF_ExceptionHandler(ERRF_ExceptionInfo* excep, CpuRegisters* regs)
{
	ERRF_FatalErrInfo error;
	Result ret;

	if (R_FAILED(ret = errfInit()))
	{
		svcBreak(USERBREAK_PANIC);
		for(;;);
	}

	memset(&error, 0, sizeof(error));

	error.type = ERRF_ERRTYPE_EXCEPTION;

	error.pcAddr = regs->pc;
	getCommonErrorData(&error, 0);
	error.data.exception_data.excep = *excep;
	error.data.exception_data.regs = *regs;	

	ret = ERRF_Throw(&error);

	errfExit();

	for(;;);
}
