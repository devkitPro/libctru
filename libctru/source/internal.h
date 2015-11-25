#pragma once
#include <sys/reent.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/thread.h>

#define THREADVARS_MAGIC  0x21545624 // !TV$
#define FS_OVERRIDE_MAGIC 0x21465324 // !FS$

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
	bool   fs_sdmc;
} ThreadVars;

static inline ThreadVars* getThreadVars(void)
{
	return (ThreadVars*)getThreadLocalStorage();
}
