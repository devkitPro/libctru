#pragma once
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <sys/reent.h>

#define THREADVARS_MAGIC  0x21545624 // !TV$
#define FS_OVERRIDE_MAGIC 0x21465324 // !FS$

// Keep this structure under 0x80 bytes
typedef struct
{
	// Magic value used to check if the struct is initialized
	u32 magic;

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
