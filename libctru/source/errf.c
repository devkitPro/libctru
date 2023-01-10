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
	if (R_FAILED(rc))
	{
		errfHandle = 0;
		errfExit();
	}

	return rc;
}

void errfExit(void)
{
	if (AtomicDecrement(&errfRefCount))
		return;
	if (errfHandle != 0) svcCloseHandle(errfHandle);
	errfHandle = 0;
}

Handle* errfGetSessionHandle(void)
{
	return &errfHandle;
}

Result ERRF_Throw(const ERRF_FatalErrInfo* error)
{
	u32 *cmdbuf = getThreadCommandBuffer();

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
	svcGetProcessId(&error->procId, CUR_PROCESS_HANDLE);
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

Result ERRF_LogResult(Result failure)
{
	ERRF_FatalErrInfo error;
	Result ret;

	if (R_FAILED(ret = errfInit()))
		return ret;

	memset(&error, 0, sizeof(error));

	error.type = ERRF_ERRTYPE_LOG_ONLY;

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

	if (R_FAILED(ret = errfInit()))
		return ret;

	memset(&error, 0, sizeof(error));

	error.type = ERRF_ERRTYPE_FAILURE;
	getCommonErrorData(&error, failure);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
	// Official client code always copies at most 95 bytes + NUL byte, but server codes uses %.96s
	// and explicitely handles 96 non-NUL bytes.
	strncpy(error.data.failure_mesg, message, sizeof(error.data.failure_mesg));
#pragma GCC diagnostic pop

	ret = ERRF_Throw(&error);

	errfExit();

	return ret;
}

Result ERRF_SetUserString(const char* user_string)
{
	Result ret = errfInit();
	size_t size = strnlen(user_string, 256);

	if (R_FAILED(ret))
		return ret;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,1,2); // 0x20042
	cmdbuf[1] = size; // unused
	cmdbuf[2] = IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[3] = (u32)user_string;

	if (R_SUCCEEDED(ret = svcSendSyncRequest(errfHandle)))
		ret = cmdbuf[1];

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
