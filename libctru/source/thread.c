#include "internal.h"
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

static void __panic(void)
{
	svcBreak(USERBREAK_PANIC);
	for (;;);
}

static void _thread_begin(void* arg)
{
	Thread t = (Thread)arg;
	initThreadVars(t);
	t->ep(t->arg);
	threadExit(0);
}

Thread threadCreate(ThreadFunc entrypoint, void* arg, size_t stack_size, int prio, int core_id, bool detached)
{
	// The stack must be 8-aligned at minimum.
	size_t align =  __tdata_align > 8 ? __tdata_align : 8;

	size_t stackoffset = alignTo(sizeof(struct Thread_tag), align);
	size_t allocsize = alignTo(stackoffset + stack_size, align);

	size_t tlssize = __tls_end-__tls_start;
	size_t tlsloadsize = __tdata_lma_end-__tdata_lma;
	size_t tbsssize = tlssize - tlsloadsize;

	// memalign seems to have an implicit requirement that (size % align) == 0.
	// Without this, it seems to return NULL whenever (align > 8).
	size_t size = alignTo(allocsize + tlssize, align);

	// Guard against overflow
	if (allocsize < stackoffset) return NULL;
	if ((allocsize - stackoffset) < stack_size) return NULL;
	if (size < allocsize) return NULL;

	Thread t = (Thread)memalign(align, size);
	if (!t) return NULL;

	t->ep       = entrypoint;
	t->arg      = arg;
	t->detached = detached;
	t->finished = false;
	t->stacktop = (u8*)t + allocsize;

	// ThreadVars.tls_tp must be aligned correctly, so we bump tdata_start to
	// ensure that after subtracting 8 bytes for the TLS header, it will be aligned.
	size_t tdata_start = 8 + alignTo((size_t)t->stacktop - 8, align);

	if (tlsloadsize)
		memcpy((void*)tdata_start, __tdata_lma, tlsloadsize);
	if (tbsssize)
		memset((void*)tdata_start + tlsloadsize, 0, tbsssize);

	// Set up child thread's reent struct, inheriting standard file handles
	_REENT_INIT_PTR(&t->reent);
	struct _reent* cur = getThreadVars()->reent;
	t->reent._stdin  = cur->_stdin;
	t->reent._stdout = cur->_stdout;
	t->reent._stderr = cur->_stderr;

	Result rc;
	rc = svcCreateThread(&t->handle, _thread_begin, (u32)t, (u32*)t->stacktop, prio, core_id);
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

void threadDetach(Thread thread)
{
	if (!thread || thread->detached)
		return;
	if (thread->finished)
	{
		threadFree(thread);
		return;
	}
	thread->detached = true;
	return;
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
