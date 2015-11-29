#include "internal.h"
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

extern const u8 __tdata_lma[];
extern const u8 __tdata_lma_end[];
extern u8 __tls_start[];
extern u8 __tls_end[];

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
	tv->tls_tp = (u8*)t->stacktop-8; // ARM ELF TLS ABI mandates an 8-byte header
	t->ep(t->arg);
	threadExit(0);
}

Thread threadCreate(ThreadFunc entrypoint, void* arg, size_t stack_size, int prio, int affinity, bool detached)
{
	size_t stackoffset = (sizeof(struct Thread_tag)+7)&~7;
	size_t allocsize   = stackoffset + ((stack_size+7)&~7);
	size_t tlssize = __tls_end-__tls_start;
	size_t tlsloadsize = __tdata_lma_end-__tdata_lma;
	size_t tbsssize = tlssize-tlsloadsize;

	// Guard against overflow
	if (allocsize < stackoffset) return NULL;
	if ((allocsize-stackoffset) < stack_size) return NULL;
	if ((allocsize+tlssize) < allocsize) return NULL;

	Thread t = (Thread)memalign(8,allocsize+tlssize);
	if (!t) return NULL;

	t->ep       = entrypoint;
	t->arg      = arg;
	t->detached = detached;
	t->finished = false;
	t->stacktop = (u8*)t + allocsize;

	if (tlsloadsize)
		memcpy(t->stacktop, __tdata_lma, tlsloadsize);
	if (tbsssize)
		memset((u8*)t->stacktop+tlsloadsize, 0, tbsssize);

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
