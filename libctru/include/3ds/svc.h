/*
  svc.h _ Syscall wrappers.
*/

#ifndef SVC_H
#define SVC_H

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
	MEMPERM_READ   =1,
	MEMPERM_WRITE  =2,
	MEMPERM_EXECUTE=4,
	MEMPERM_MAX    =0xFFFFFFFF //force 4-byte
} MemPerm;

u32* getThreadCommandBuffer(void);

s32  svcControlMemory(u32* addr_out, u32 addr0, u32 addr1, u32 size, MemOp op, MemPerm perm);
void __attribute__((noreturn)) svcExitProcess();
s32  svcCreateThread(Handle* thread, ThreadFunc entrypoint, u32 arg, u32* stack_top, s32 thread_priority, s32 processor_id);
void __attribute__((noreturn)) svcExitThread();
void svcSleepThread(s64 ns);
s32  svcCreateMutex(Handle* mutex, bool initially_locked);
s32  svcReleaseMutex(Handle handle);
s32  svcCreateEvent(Handle* event, u8 reset_type);
s32  svcSignalEvent(Handle handle);
s32  svcClearEvent(Handle handle);
s32  svcCreateMemoryBlock(Handle* memblock, u32 addr, u32 size, MemPerm my_perm, MemPerm other_perm);
s32  svcMapMemoryBlock(Handle memblock, u32 addr, MemPerm my_perm, MemPerm other_perm);
s32  svcUnmapMemoryBlock(Handle memblock, u32 addr);
s32  svcWaitSynchronization(Handle handle, s64 nanoseconds);
s32  svcWaitSynchronizationN(s32* out, Handle* handles, s32 handles_num, bool wait_all, s64 nanoseconds);
s32  svcCloseHandle(Handle handle);
s32  svcDuplicateHandle(Handle* out, Handle original);
u64  svcGetSystemTick();
s32  svcGetSystemInfo(s64* out, u32 type, s32 param);
s32  svcGetProcessInfo(s64* out, Handle process, u32 type);
s32  svcConnectToPort(volatile Handle* out, const char* portName);
s32  svcSendSyncRequest(Handle session);
s32  svcGetProcessId(u32 *out, Handle handle);

#endif
