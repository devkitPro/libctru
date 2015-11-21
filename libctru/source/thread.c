#include "internal.h"
#include <stdlib.h>
#include <string.h>

struct Thread_tag
{
	Handle handle;
	ThreadFunc ep;
	void* arg;
	int rc;
	bool detached, finished;
	struct _reent reent;
	void* stacktop;
};

static void __panic(void)
{
	svcBreak(USERBREAK_PANIC);
	for (;;);
}

static void _thread_begin(void* arg)
{
	Thread t = (Thread)arg;
	ThreadVars* tv = getThreadVars();
	tv->magic = THREADVARS_MAGIC;
	tv->reent = &t->reent;
	tv->thread_ptr = t;
	t->ep(t->arg);
	threadExit(0);
}

Thread threadCreate(ThreadFunc entrypoint, void* arg, size_t stack_size, int prio, int affinity, bool detached)
{
	size_t stackoffset = (sizeof(struct Thread_tag)+7)&~7;
	size_t allocsize   = stackoffset + ((stack_size+7)&~7);
	if (allocsize < stackoffset) return NULL; // guard against overflow
	if ((allocsize-stackoffset) < stack_size) return NULL; // guard against overflow
	Thread t = (Thread)malloc(allocsize);
	if (!t) return NULL;

	t->ep       = entrypoint;
	t->arg      = arg;
	t->detached = detached;
	t->finished = false;
	t->stacktop = (u8*)t + allocsize;

	// Set up child thread's reent struct, inheriting standard file handles
	_REENT_INIT_PTR(&t->reent);
	struct _reent* cur = getThreadVars()->reent;
	t->reent._stdin  = cur->_stdin;
	t->reent._stdout = cur->_stdout;
	t->reent._stderr = cur->_stderr;

	Result rc;
	rc = svcCreateThread(&t->handle, _thread_begin, (u32)t, (u32*)t->stacktop, prio, affinity);
	if (R_FAILED(rc))
	{
		free(t);
		return NULL;
	}

	return t;
}

Handle threadGetHandle(Thread thread)
{
	if (!thread || thread->finished) return ~0UL;
	return thread->handle;
}

int threadGetExitCode(Thread thread)
{
	if (!thread || !thread->finished) return 0;
	return thread->rc;
}

void threadFree(Thread thread)
{
	if (!thread || !thread->finished) return;
	svcCloseHandle(thread->handle);
	free(thread);
}

Result threadJoin(Thread thread, u64 timeout_ns)
{
	if (!thread || thread->finished) return 0;
	return svcWaitSynchronization(thread->handle, timeout_ns);
}

Thread threadGetCurrent(void)
{
	ThreadVars* tv = getThreadVars();
	if (tv->magic != THREADVARS_MAGIC)
		__panic();
	return tv->thread_ptr;
}

void threadExit(int rc)
{
	Thread t = threadGetCurrent();
	if (!t)
		__panic();

	t->finished = true;
	if (t->detached)
		threadFree(t);
	else
		t->rc = rc;

	svcExitThread();
}
