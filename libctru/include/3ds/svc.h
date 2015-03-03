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
s32  svcGetCurrentProcessorNumber(void);
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
Result svcGetProcessList(s32* processCount, u32* processIds, s32 processIdMaxCount);
Result svcReadProcessMemory(void* buffer, Handle debug, u32 addr, u32 size);
Result svcMapProcessMemory(Handle process, u32 startAddr, u32 endAddr);
Result svcUnmapProcessMemory(Handle process, u32 startAddr, u32 endAddr);
Result svcQueryProcessMemory(MemInfo* info, PageInfo* out, Handle process, u32 addr);
s32 svcGetProcessorID();
