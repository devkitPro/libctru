#pragma once
#include <sys/reent.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/thread.h>

#define THREADVARS_MAGIC  0x21545624 // !TV$
#define FS_OVERRIDE_MAGIC 0x21465324 // !FS$

extern const size_t __tdata_align;
extern const u8 __tdata_lma[];
extern const u8 __tdata_lma_end[];
extern u8 __tls_start[];
extern u8 __tls_end[];

// Keep this structure under 0x80 bytes
typedef struct
{
	// Magic value used to check if the struct is initialized
	u32 magic;

	// Pointer to the current thread (if exists)
	Thread thread_ptr;

	// Pointer to this thread's newlib state
	struct _reent* reent;

	// Pointer to this thread's thread-local segment
	void* tls_tp; // !! Keep offset in sync inside __aeabi_read_tp !!

	// FS session override
	u32    fs_magic;
	Handle fs_session;

	// Whether srvGetServiceHandle is non-blocking in case of full service ports.
	bool srv_blocking_policy;
} ThreadVars;

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

static inline ThreadVars* getThreadVars(void)
{
	return (ThreadVars*)getThreadLocalStorage();
}

void initThreadVars(struct Thread_tag *thread);

static inline size_t alignTo(const size_t base, const size_t align) {
	return (base + (align - 1)) & ~(align - 1);
}
