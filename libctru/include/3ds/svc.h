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

s32  svcControlMemory(u32* addr_out, u32 addr0, u32 addr1, u32 size, MemOp op, MemPerm perm);
s32  svcQueryMemory(MemInfo* info, PageInfo* out, u32 addr);
void __attribute__((noreturn)) svcExitProcess();
s32  svcCreateThread(Handle* thread, ThreadFunc entrypoint, u32 arg, u32* stack_top, s32 thread_priority, s32 processor_id);
void __attribute__((noreturn)) svcExitThread();
void svcSleepThread(s64 ns);
s32  svcSetThreadPriority(Handle thread, s32 prio);
s32  svcCreateMutex(Handle* mutex, bool initially_locked);
s32  svcReleaseMutex(Handle handle);
s32  svcCreateSemaphore(Handle* semaphore, s32 initial_count, s32 max_count);
s32  svcReleaseSemaphore(s32* count, Handle semaphore, s32 release_count);
s32  svcCreateEvent(Handle* event, u8 reset_type);
s32  svcSignalEvent(Handle handle);
s32  svcClearEvent(Handle handle);
s32  svcCreateTimer(Handle* timer, u8 reset_type);
s32  svcSetTimer(Handle timer, s64 initial, s64 interval);
s32  svcCancelTimer(Handle timer);
s32  svcClearTimer(Handle timer);
s32  svcCreateMemoryBlock(Handle* memblock, u32 addr, u32 size, MemPerm my_perm, MemPerm other_perm);
s32  svcMapMemoryBlock(Handle memblock, u32 addr, MemPerm my_perm, MemPerm other_perm);
s32  svcUnmapMemoryBlock(Handle memblock, u32 addr);
s32  svcCreateAddressArbiter(Handle *arbiter);
s32  svcArbitrateAddress(Handle arbiter, u32 addr, ArbitrationType type, s32 value, s64 nanoseconds);
s32  svcWaitSynchronization(Handle handle, s64 nanoseconds);
s32  svcWaitSynchronizationN(s32* out, Handle* handles, s32 handles_num, bool wait_all, s64 nanoseconds);
s32  svcCloseHandle(Handle handle);
s32  svcDuplicateHandle(Handle* out, Handle original);
u64  svcGetSystemTick();
s32  svcGetSystemInfo(s64* out, u32 type, s32 param);
s32  svcGetProcessInfo(s64* out, Handle process, u32 type);
s32  svcConnectToPort(volatile Handle* out, const char* portName);
s32  svcSendSyncRequest(Handle session);
Result svcOpenProcess(Handle* process, u32 processId);
s32  svcGetProcessId(u32 *out, Handle handle);
s32  svcGetThreadId(u32 *out, Handle handle);
s32  svcOutputDebugString(const char* str, int length);
Result svcCreatePort(Handle* portServer, Handle* portClient, const char* name, s32 maxSessions);
Result svcDebugActiveProcess(Handle* debug, u32 processId);
Result svcBreakDebugProcess(Handle debug);
Result svcTerminateDebugProcess(Handle debug);
Result svcGetProcessDebugEvent(DebugEventInfo *info, Handle debug);
Result svcContinueDebugEvent(Handle debug, u32 flags);
Result svcGetProcessList(s32* processCount, u32* processIds, s32 processIdMaxCount);
Result svcReadProcessMemory(void* buffer, Handle debug, u32 addr, u32 size);
Result svcMapProcessMemory(Handle process, u32 startAddr, u32 endAddr);
Result svcUnmapProcessMemory(Handle process, u32 startAddr, u32 endAddr);
Result svcQueryProcessMemory(MemInfo* info, PageInfo* out, Handle process, u32 addr);
s32 svcGetProcessorID();
