/*
  svc.h _ Syscall wrappers.
*/

#pragma once

typedef enum {
	MEMOP_FREE =1, // Free heap
	MEMOP_ALLOC=3, // Allocate heap
	MEMOP_MAP  =4, // Mirror mapping
	MEMOP_UNMAP=5, // Mirror unmapping
	MEMOP_PROT =6, // Change protection

	MEMOP_FREE_LINEAR =0x10001, // Free linear heap
	MEMOP_ALLOC_LINEAR=0x10003  // Allocate linear heap
} MemOp;

typedef enum {
	MEMSTATE_FREE       = 0,
	MEMSTATE_RESERVED   = 1,
	MEMSTATE_IO         = 2,
	MEMSTATE_STATIC     = 3,
	MEMSTATE_CODE       = 4,
	MEMSTATE_PRIVATE    = 5,
	MEMSTATE_SHARED     = 6,
	MEMSTATE_CONTINUOUS = 7,
	MEMSTATE_ALIASED    = 8,
	MEMSTATE_ALIAS      = 9,
	MEMSTATE_ALIASCODE  = 10,
	MEMSTATE_LOCKED     = 11
} MemState;

typedef enum {
	MEMPERM_READ     = 1,
	MEMPERM_WRITE    = 2,
	MEMPERM_EXECUTE  = 4,
	MEMPERM_DONTCARE = 0x10000000,
	MEMPERM_MAX      = 0xFFFFFFFF //force 4-byte
} MemPerm;

typedef struct {
    u32 base_addr;
    u32 size;
    u32 perm;
    u32 state;
} MemInfo;

typedef struct {
    u32 flags;
} PageInfo;

typedef enum {
	ARBITER_FREE           =0,
	ARBITER_ACQUIRE        =1,
	ARBITER_KERNEL2        =2,
	ARBITER_ACQUIRE_TIMEOUT=3,
	ARBITER_KERNEL4        =4,
} ArbitrationType;

typedef enum {
	DBG_EVENT_PROCESS        = 0,
	DBG_EVENT_CREATE_THREAD  = 1,
	DBG_EVENT_EXIT_THREAD    = 2,
	DBG_EVENT_EXIT_PROCESS   = 3,
	DBG_EVENT_EXCEPTION      = 4,
	DBG_EVENT_DLL_LOAD       = 5,
	DBG_EVENT_DLL_UNLOAD     = 6,
	DBG_EVENT_SCHEDULE_IN    = 7,
	DBG_EVENT_SCHEDULE_OUT   = 8,
	DBG_EVENT_SYSCALL_IN     = 9,
	DBG_EVENT_SYSCALL_OUT    = 10,
	DBG_EVENT_OUTPUT_STRING  = 11,
	DBG_EVENT_MAP            = 12
} DebugEventType;

typedef enum {
	REASON_CREATE = 1,
	REASON_ATTACH = 2
} ProcessEventReason;
               
typedef struct {
	u64 program_id;
	u8  process_name[8];
	u32 process_id;
	u32 reason;
} ProcessEvent;

typedef struct {
	u32 creator_thread_id;
	u32 base_addr;
	u32 entry_point;
} CreateThreadEvent;

typedef enum {
	EXITTHREAD_EVENT_NONE              = 0,
	EXITTHREAD_EVENT_TERMINATE         = 1,
	EXITTHREAD_EVENT_UNHANDLED_EXC     = 2,
	EXITTHREAD_EVENT_TERMINATE_PROCESS = 3
} ExitThreadEventReason;

typedef enum {
	EXITPROCESS_EVENT_NONE                = 0,
	EXITPROCESS_EVENT_TERMINATE           = 1,
	EXITPROCESS_EVENT_UNHANDLED_EXCEPTION = 2
} ExitProcessEventReason;

typedef struct {
	u32 reason;
} ExitProcessEvent;

typedef struct {
	u32 reason;
} ExitThreadEvent;

typedef struct {
	u32 type;
	u32 address;
	u32 argument;
} ExceptionEvent;

typedef enum {
	EXC_EVENT_UNDEFINED_INSTRUCTION = 0, // arg: (None)
	EXC_EVENT_UNKNOWN1              = 1, // arg: (None)
	EXC_EVENT_UNKNOWN2              = 2, // arg: address
	EXC_EVENT_UNKNOWN3              = 3, // arg: address
	EXC_EVENT_ATTACH_BREAK          = 4, // arg: (None)
	EXC_EVENT_BREAKPOINT            = 5, // arg: (None)
	EXC_EVENT_USER_BREAK            = 6, // arg: user break type
	EXC_EVENT_DEBUGGER_BREAK        = 7, // arg: (None)
	EXC_EVENT_UNDEFINED_SYSCALL     = 8  // arg: attempted syscall 
} ExceptionEventType;

typedef enum {
	USERBREAK_PANIC  = 0,
	USERBREAK_ASSERT = 1,
	USERBREAK_USER   = 2
} UserBreakType;

typedef struct {
	u64 clock_tick;
} SchedulerInOutEvent;

typedef struct {
	u64 clock_tick;
	u32 syscall;
} SyscallInOutEvent;

typedef struct {
	u32 string_addr;
	u32 string_size;
} OutputStringEvent;

typedef struct {
	u32 mapped_addr;
	u32 mapped_size;
	u32 memperm;
	u32 memstate;
} MapEvent;

typedef struct {
	u32 type;
	u32 thread_id;
	u32 unknown[2];
	union {
		ProcessEvent process;
		CreateThreadEvent create_thread;
		ExitThreadEvent exit_thread;
		ExitProcessEvent exit_process;
		ExceptionEvent exception;
		/* TODO: DLL_LOAD */
		/* TODO: DLL_UNLOAD */
		SchedulerInOutEvent scheduler;
		SyscallInOutEvent syscall;
		OutputStringEvent output_string;
		MapEvent map;		
	};
} DebugEventInfo;

static inline void* getThreadLocalStorage(void)
{
	void* ret;
	asm volatile("mrc p15, 0, %[data], c13, c0, 3" : [data] "=r" (ret));
	return ret;
}

static inline u32* getThreadCommandBuffer(void)
{
	return (u32*)((u8*)getThreadLocalStorage() + 0x80);
}

static inline void __attribute__((noreturn)) svcExitProcess()
{
	asm volatile("svc 0x03"
		::: "lr");
	__builtin_unreachable();
}

static inline void __attribute__((noreturn)) svcExitThread()
{
	asm volatile("svc 0x09"
		::: "lr");
	__builtin_unreachable();
}

static inline void svcSleepThread(s64 ns)
{
	register s64 r0 asm("r0") = ns;

	asm volatile("svc 0x0A"
		:: "r"(r0)
		: "lr");
}

static inline s32 svcSetThreadPriority(Handle thread, s32 prio)
{
	register Handle r0 asm("r0") = thread;
	register s32 r1 asm("r1") = prio;
	register s32 res asm("r0");

	asm volatile("svc 0x0C"
		: "=r"(res)
		: "r"(r0), "r"(r1)
		: "lr");

	return res;
}

static inline s32 svcReleaseMutex(Handle handle)
{
	register Handle r0 asm("r0") = handle;
	register s32 res asm("r0");

	asm volatile("svc 0x14"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline s32 svcSignalEvent(Handle handle)
{
	register Handle r0 asm("r0") = handle;
	register s32 res asm("r0");

	asm volatile("svc 0x18"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline s32 svcClearEvent(Handle handle)
{
	register Handle r0 asm("r0") = handle;
	register s32 res asm("r0");

	asm volatile("svc 0x19"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline s32 svcSetTimer(Handle timer, s64 initial, s64 interval)
{
	register Handle r0 asm("r0") = timer;
	register s64 r1 asm("r1") = initial;
	register s64 r2 asm("r2") = interval;
	register s32 res asm("r0");

	asm volatile("svc 0x1B"
		: "=r"(res)
		: "r"(r0), "r"(r1), "r"(r2)
		: "lr");

	return res;
}

static inline s32 svcCancelTimer(Handle timer)
{
	register Handle r0 asm("r0") = timer;
	register s32 res asm("r0");

	asm volatile("svc 0x1C"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline s32 svcClearTimer(Handle timer)
{
	register Handle r0 asm("r0") = timer;
	register s32 res asm("r0");

	asm volatile("svc 0x1D"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline s32 svcMapMemoryBlock(
	Handle memblock, u32 addr, MemPerm my_perm, MemPerm other_perm)
{
	register Handle r0 asm("r0") = memblock;
	register u32 r1 asm("r1") = addr;
	register MemPerm r2 asm("r2") = my_perm;
	register MemPerm r3 asm("r3") = other_perm;
	register s32 res asm("r0");

	asm volatile("svc 0x1F"
		: "=r"(res)
		: "r"(r0), "r"(r1), "r"(r2), "r"(r3)
		: "lr");

	return res;
}

static inline s32 svcUnmapMemoryBlock(Handle memblock, u32 addr)
{
	register Handle r0 asm("r0") = memblock;
	register u32 r1 asm("r1") = addr;
	register s32 res asm("r0");

	asm volatile("svc 0x20"
		: "=r"(res)
		: "r"(r0), "r"(r1)
		: "lr");

	return res;
}

static inline s32 svcCreateAddressArbiter(Handle *arbiter)
{
	register Handle *r0 asm("r0") = arbiter;
	register s32 res asm("r0");

	asm volatile("svc 0x21"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline s32 svcCloseHandle(Handle handle)
{
	register Handle r0 asm("r0") = handle;
	register s32 res asm("r0");

	asm volatile("svc 0x23"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline s32 svcWaitSynchronization(Handle handle, s64 nanoseconds)
{
	register Handle r0 asm("r0") = handle;
	register s32 r1 asm("r1") = nanoseconds & 0xFFFFFFFF;
	register s32 r2 asm("r2") = (nanoseconds >> 32) & 0xFFFFFFFF;
	register s32 res asm("r0");

	asm volatile("svc 0x24"
		: "=r"(res)
		: "r"(r0), "r"(r1), "r"(r2)
		: "lr");

	return res;
}

static inline u64 svcGetSystemTick()
{
	register u32 r0 asm("r0");
	register u32 r1 asm("r1");

	asm volatile("svc 0x28"
		: "=r"(r0), "=r"(r1)
		:: "lr");

	return r0 | ((u64)r1 << 32);
}

static inline s32 svcSendSyncRequest(Handle session)
{
	register Handle r0 asm("r0") = session;
	register s32 res asm("r0");

	asm volatile("svc 0x32"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline s32 svcOutputDebugString(const char* str, int length)
{
	register const char *r0 asm("r0") = str;
	register int r1 asm("r1") = length;
	register s32 res asm("r0");

	asm volatile("svc 0x3D"
		: "=r"(res)
		: "r"(r0), "r"(r1)
		: "lr");

	return res;
}

static inline Result svcBreakDebugProcess(Handle debug)
{
	register Handle r0 asm("r0") = debug;
	register s32 res asm("r0");

	asm volatile("svc 0x61"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline Result svcTerminateDebugProcess(Handle debug)
{
	register Handle r0 asm("r0") = debug;
	register s32 res asm("r0");

	asm volatile("svc 0x62"
		: "=r"(res)
		: "r"(r0)
		: "lr");

	return res;
}

static inline Result svcGetProcessDebugEvent(DebugEventInfo *info, Handle debug)
{
	register DebugEventInfo *r0 asm("r0") = info;
	register Handle r1 asm("r1") = debug;
	register Result res asm("r0");

	asm volatile("svc 0x63"
		: "=r"(res)
		: "r"(r0), "r"(r1)
		: "lr");

	return res;
}

static inline Result svcContinueDebugEvent(Handle debug, u32 flags)
{
	register Handle r0 asm("r0") = debug;
	register u32 r1 asm("r1") = flags;
	register Result res asm("r0");

	asm volatile("svc 0x64"
		: "=r"(res)
		: "r"(r0), "r"(r1)
		: "lr");

	return res;
}

static inline Result svcReadProcessMemory(void* buffer, Handle debug, u32 addr, u32 size)
{
	register void* r0 asm("r0") = buffer;
	register Handle r1 asm("r1") = debug;
	register u32 r2 asm("r2") = addr;
	register u32 r3 asm("r3") = size;
	register Result res asm("r0");

	asm volatile("svc 0x6A"
		: "=r"(res)
		: "r"(r0), "r"(r1), "r"(r2), "r"(r3)
		: "lr");

	return res;
}

static inline Result svcMapProcessMemory(Handle process, u32 startAddr, u32 endAddr)
{
	register Handle r0 asm("r0") = process;
	register u32 r1 asm("r1") = startAddr;
	register u32 r2 asm("r2") = endAddr;
	register Result res asm("r0");

	asm volatile("svc 0x71"
		: "=r"(res)
		: "r"(r0), "r"(r1), "r"(r2)
		: "lr");

	return res;
}

static inline Result svcUnmapProcessMemory(Handle process, u32 startAddr, u32 endAddr)
{
	register Handle r0 asm("r0") = process;
	register u32 r1 asm("r1") = startAddr;
	register u32 r2 asm("r2") = endAddr;
	register Result res asm("r0");

	asm volatile("svc 0x72"
		: "=r"(res)
		: "r"(r0), "r"(r1), "r"(r2)
		: "lr");

	return res;
}

static inline s32 svcGetProcessorID()
{
	register s32 res asm("r0");

	asm volatile("svc 0x11"
		: "=r"(res)
		:: "lr");

	return res;
}

s32  svcControlMemory(u32* addr_out, u32 addr0, u32 addr1, u32 size, MemOp op, MemPerm perm);
s32  svcQueryMemory(MemInfo* info, PageInfo* out, u32 addr);
s32  svcCreateThread(Handle* thread, ThreadFunc entrypoint, u32 arg, u32* stack_top, s32 thread_priority, s32 processor_id);
s32  svcSetThreadPriority(Handle thread, s32 prio);
s32  svcCreateMutex(Handle* mutex, bool initially_locked);
s32  svcCreateSemaphore(Handle* semaphore, s32 initial_count, s32 max_count);
s32  svcReleaseSemaphore(s32* count, Handle semaphore, s32 release_count);
s32  svcCreateEvent(Handle* event, u8 reset_type);
s32  svcCreateTimer(Handle* timer, u8 reset_type);
s32  svcCreateMemoryBlock(Handle* memblock, u32 addr, u32 size, MemPerm my_perm, MemPerm other_perm);
s32  svcArbitrateAddress(Handle arbiter, u32 addr, ArbitrationType type, s32 value, s64 nanoseconds);
s32  svcWaitSynchronizationN(s32* out, Handle* handles, s32 handles_num, bool wait_all, s64 nanoseconds);
s32  svcDuplicateHandle(Handle* out, Handle original);
s32  svcGetSystemInfo(s64* out, u32 type, s32 param);
s32  svcGetProcessInfo(s64* out, Handle process, u32 type);
s32  svcConnectToPort(volatile Handle* out, const char* portName);
Result svcOpenProcess(Handle* process, u32 processId);
s32  svcGetProcessId(u32 *out, Handle handle);
s32  svcGetThreadId(u32 *out, Handle handle);
Result svcCreatePort(Handle* portServer, Handle* portClient, const char* name, s32 maxSessions);
Result svcDebugActiveProcess(Handle* debug, u32 processId);

Result svcGetProcessList(s32* processCount, u32* processIds, s32 processIdMaxCount);
Result svcQueryProcessMemory(MemInfo* info, PageInfo* out, Handle process, u32 addr);
