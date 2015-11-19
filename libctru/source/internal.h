#pragma once
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <sys/reent.h>

#define FS_OVERRIDE_MAGIC 0x21465324 // !FS$

typedef struct
{
	// Pointer to this thread's newlib state
	struct _reent* reent;

	// FS session override
	u32    fs_magic;
	Handle fs_session;
	bool   fs_sdmc;
} ThreadVars;

static inline ThreadVars* getThreadVars(void)
{
	return (ThreadVars*)getThreadLocalStorage();
}
