/**
 * @file svc.h
 * @brief Syscall wrappers.
 */

#pragma once

#include "types.h"


///@name Memory management
///@{

/**
 * @brief @ref svcControlMemory operation flags
 *
 * The lowest 8 bits are the operation
 */
typedef enum {
	MEMOP_FREE    = 1, ///< Memory un-mapping
	MEMOP_RESERVE = 2, ///< Reserve memory
	MEMOP_ALLOC   = 3, ///< Memory mapping
	MEMOP_MAP     = 4, ///< Mirror mapping
	MEMOP_UNMAP   = 5, ///< Mirror unmapping
	MEMOP_PROT    = 6, ///< Change protection

	MEMOP_REGION_APP    = 0x100,
	MEMOP_REGION_SYSTEM = 0x200,
	MEMOP_REGION_BASE   = 0x300,

	MEMOP_OP_MASK     = 0xFF,
	MEMOP_REGION_MASK = 0xF00,
	MEMOP_LINEAR_FLAG = 0x10000, ///< Flag for linear memory operations

	MEMOP_ALLOC_LINEAR   = MEMOP_LINEAR_FLAG | MEMOP_ALLOC,

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

/**
 * @brief Memory permission flags
 */
typedef enum {
	MEMPERM_READ     = 1,
	MEMPERM_WRITE    = 2,
	MEMPERM_EXECUTE  = 4,
	MEMPERM_DONTCARE = 0x10000000
} MemPerm;

typedef struct {
    u32 base_addr;
    u32 size;
    u32 perm;  ///< See @ref MemPerm
    u32 state; ///< See @ref MemState
} MemInfo;

typedef struct {
    u32 flags;
} PageInfo;

typedef enum {
	ARBITRATION_SIGNAL                                  = 0, ///< Signal #value threads for wake-up.
	ARBITRATION_WAIT_IF_LESS_THAN                       = 1, ///< If the memory at the address is strictly lower than #value, then wait for signal.
	ARBITRATION_DECREMENT_AND_WAIT_IF_LESS_THAN         = 2, ///< If the memory at the address is strictly lower than #value, then decrement it and wait for signal.
	ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT               = 3, ///< If the memory at the address is strictly lower than #value, then wait for signal or timeout.
	ARBITRATION_DECREMENT_AND_WAIT_IF_LESS_THAN_TIMEOUT = 4, ///< If the memory at the address is strictly lower than #value, then decrement it and wait for signal or timeout.
} ArbitrationType;

/// Special value to signal all the threads
#define ARBITRATION_SIGNAL_ALL (-1)

///@}

///@name Multithreading
///@{

typedef enum {
	THREADINFO_TYPE_UNKNOWN
} ThreadInfoType;

///@}


///@name Debugging
///@{
typedef enum {
	REASON_CREATE = 1,
	REASON_ATTACH = 2
} ProcessEventReason;

typedef struct {
	u64 program_id;
	u8  process_name[8];
	u32 process_id;
	u32 reason;          ///< See @ref ProcessEventReason
} ProcessEvent;

typedef enum {
	EXITPROCESS_EVENT_NONE                = 0,
	EXITPROCESS_EVENT_TERMINATE           = 1,
	EXITPROCESS_EVENT_UNHANDLED_EXCEPTION = 2
} ExitProcessEventReason;

typedef struct {
	u32 reason; ///< See @ref ExitProcessEventReason
} ExitProcessEvent;

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

typedef struct {
	u32 reason; ///< See @ref ExitThreadEventReason
} ExitThreadEvent;

typedef enum {
	USERBREAK_PANIC  = 0,
	USERBREAK_ASSERT = 1,
	USERBREAK_USER   = 2
} UserBreakType;

typedef enum {
	EXC_EVENT_UNDEFINED_INSTRUCTION = 0, ///< arg: (None)
	EXC_EVENT_UNKNOWN1              = 1, ///< arg: (None)
	EXC_EVENT_UNKNOWN2              = 2, ///< arg: address
	EXC_EVENT_UNKNOWN3              = 3, ///< arg: address
	EXC_EVENT_ATTACH_BREAK          = 4, ///< arg: (None)
	EXC_EVENT_BREAKPOINT            = 5, ///< arg: (None)
	EXC_EVENT_USER_BREAK            = 6, ///< arg: @ref UserBreakType
	EXC_EVENT_DEBUGGER_BREAK        = 7, ///< arg: (None)
	EXC_EVENT_UNDEFINED_SYSCALL     = 8  ///< arg: attempted syscall
} ExceptionEventType;

typedef struct {
	u32 type;     ///< See @ref ExceptionEventType
	u32 address;
	u32 argument; ///< See @ref ExceptionEventType
} ExceptionEvent;

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

typedef struct {
	u32 type;		///< See @ref DebugEventType
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

///@}

static inline void* getThreadLocalStorage(void)
{
	void* ret;
	__asm__ ("mrc p15, 0, %[data], c13, c0, 3" : [data] "=r" (ret));
	return ret;
}

static inline u32* getThreadCommandBuffer(void)
{
	return (u32*)((u8*)getThreadLocalStorage() + 0x80);
}

///@name Memory management
///@{

/**
 * @brief Controls memory mapping
 * @param[out] addr_out The virtual address resulting from the operation. Usually the same as addr0.
 * @param addr0    The virtual address to be used for the operation.
 * @param addr1    The virtual address to be (un)mirrored by @p addr0 when using @ref MEMOP_MAP or @ref MEMOP_UNMAP.
 *                 It has to be pointing to a RW memory.
 *                 Use NULL if the operation is @ref MEMOP_FREE or @ref MEMOP_ALLOC.
 * @param size     The requested size for @ref MEMOP_ALLOC and @ref MEMOP_ALLOC_LINEAR.
 * @param op       Operation flags. See @ref MemOp.
 * @param perm     A combination of @ref MEMPERM_READ and @ref MEMPERM_WRITE. Using MEMPERM_EXECUTE will return an error.
 * 			       Value 0 is used when unmapping memory.
 *
 * If a memory is mapped for two or more addresses, you have to use MEMOP_UNMAP before being able to MEMOP_FREE it.
 * MEMOP_MAP will fail if @p addr1 was already mapped to another address.
 *
 * More information is available at http://3dbrew.org/wiki/SVC#Memory_Mapping.
 *
 * @sa svcControlProcessMemory
 */
Result svcControlMemory(u32* addr_out, u32 addr0, u32 addr1, u32 size, MemOp op, MemPerm perm);

/**
 * @brief Controls the memory mapping of a process
 * @param addr0 The virtual address to map
 * @param addr1 The virtual address to be mapped by @p addr0
 * @param type Only operations @ref MEMOP_MAP, @ref MEMOP_UNMAP and @ref MEMOP_PROT are allowed.
 *
 * This is the only SVC which allows mapping executable memory.
 * Using @ref MEMOP_PROT will change the memory permissions of an already mapped memory.
 *
 * @note The pseudo handle for the current process is not supported by this service call.
 * @sa svcControlProcess
 */
Result svcControlProcessMemory(Handle process, u32 addr0, u32 addr1, u32 size, u32 type, u32 perm);

/**
 * @brief Creates a block of shared memory
 * @param memblock Pointer to store the handle of the block
 * @param addr Address of the memory to map, page-aligned. So its alignment must be 0x1000.
 * @param size Size of the memory to map, a multiple of 0x1000.
 * @param my_perm Memory permissions for the current process
 * @param my_perm Memory permissions for the other processes
 *
 * @note The shared memory block, and its rights, are destroyed when the handle is closed.
 */
Result svcCreateMemoryBlock(Handle* memblock, u32 addr, u32 size, MemPerm my_perm, MemPerm other_perm);
Result svcMapMemoryBlock(Handle memblock, u32 addr, MemPerm my_perm, MemPerm other_perm);
Result svcMapProcessMemory(Handle process, u32 startAddr, u32 endAddr);
Result svcUnmapProcessMemory(Handle process, u32 startAddr, u32 endAddr);
Result svcUnmapMemoryBlock(Handle memblock, u32 addr);

Result svcStartInterProcessDma(Handle* dma, Handle dstProcess, void* dst, Handle srcProcess, const void* src, u32 size, void* dmaConfig);
Result svcStopDma(Handle dma);
Result svcGetDmaState(void* dmaState, Handle dma);
/**
 * @brief Memory information query
 * @param addr Virtual memory address
 */
Result svcQueryMemory(MemInfo* info, PageInfo* out, u32 addr);
Result svcQueryProcessMemory(MemInfo* info, PageInfo* out, Handle process, u32 addr);


Result svcInvalidateProcessDataCache(Handle process, void* addr, u32 size);
Result svcFlushProcessDataCache(Handle process, void const* addr, u32 size);

Result svcReadProcessMemory(void* buffer, Handle debug, u32 addr, u32 size);
Result svcWriteProcessMemory(Handle debug, const void* buffer, u32 addr, u32 size);
///@}


///@name Process management
///@{

/**
 * @brief Gets the handle of a process.
 * @param[out] process   The handle of the process
 * @param      processId The ID of the process to open
 */
Result svcOpenProcess(Handle* process, u32 processId);
void svcExitProcess() __attribute__((noreturn));
Result svcTerminateProcess(Handle process);

Result svcGetProcessInfo(s64* out, Handle process, u32 type);
Result svcGetProcessId(u32 *out, Handle handle);
Result svcGetProcessList(s32* processCount, u32* processIds, s32 processIdMaxCount);
Result svcCreatePort(Handle* portServer, Handle* portClient, const char* name, s32 maxSessions);
Result svcConnectToPort(volatile Handle* out, const char* portName);
///@}

///@name Multithreading
///@{
/**
 * @brief Creates a new thread.
 * @param[out] thread     The thread handle
 * @param entrypoint      The function that will be called first upon thread creation
 * @param arg             The argument passed to @p entrypoint
 * @param stack_top       The top of the thread's stack. Must be 0x8 bytes mem-aligned.
 * @param thread_priority Low values gives the thread higher priority.
 *                        For userland apps, this has to be within the range [0x18;0x3F]
 * @param processor_id    The id of the processor the thread should be ran on. Those are labelled starting from 0.
 *                        For old 3ds it has to be <2, and for new 3DS <4.
 *                        Value -1 means all CPUs and -2 read from the Exheader.
 *
 * The processor with ID 1 is the system processor.
 * To enable multi-threading on this core you need to call APT_SetAppCpuTimeLimit at least once with a non-zero value.
 *
 * Since a thread is considered as a waitable object, you can use @ref svcWaitSynchronization
 * and @ref svcWaitSynchronizationN to join with it.
 *
 * @note The kernel will clear the @p stack_top's address low 3 bits to make sure it is 0x8-bytes aligned.
 */
Result svcCreateThread(Handle* thread, ThreadFunc entrypoint, u32 arg, u32* stack_top, s32 thread_priority, s32 processor_id);

/**
 * @brief Gets the handle of a thread.
 * @param[out] thread  The handle of the thread
 * @param      process The ID of the process linked to the thread
 */
Result svcOpenThread(Handle* thread,Handle process, u32 threadId);

/**
 * @brief Exits the current thread.
 *
 * This will trigger a state change and hence release all @ref svcWaitSynchronization operations.
 * It means that you can join a thread by calling @code svcWaitSynchronization(threadHandle,yourtimeout); @endcode
 */
void svcExitThread(void) __attribute__((noreturn));

/**
 * @brief Puts the current thread to sleep.
 * @param ns The minimum number of nanoseconds to sleep for.
 */
void svcSleepThread(s64 ns);

/**
 * @brief Retrieves the priority of a thread.
 */
Result svcGetThreadPriority(s32 *out, Handle handle);

/**
 * @brief Changes the priority of a thread
 * @param prio For userland apps, this has to be within the range [0x18;0x3F]
 *
 * Low values gives the thread higher priority.
 */
Result svcSetThreadPriority(Handle thread, s32 prio);
Result svcGetThreadAffinityMask(u8* affinitymask, Handle thread, s32 processorcount);
Result svcSetThreadAffinityMask(Handle thread, u8* affinitymask, s32 processorcount);
Result svcGetThreadIdealProcessor(s32* processorid, Handle thread);
Result svcSetThreadIdealProcessor(Handle thread, s32 processorid);

/**
 * @brief Returns the ID of the processor the current thread is running on.
 * @sa svcCreateThread
 */
s32    svcGetProcessorID();

/**
 * @param out The thread ID of the thread @p handle.
 */
Result svcGetThreadId(u32 *out, Handle handle);

/**
 * @param out The process ID of the thread @p handle.
 * @sa svcOpenProcess
 */
Result svcGetProcessIdOfThread(u32 *out, Handle handle);

/**
 * @brief Checks if a thread handle is valid.
 * This requests always return an error when called, it only checks if the handle is a thread or not.
 * @return 0xD8E007ED (BAD_ENUM) if the Handle is a Thread Handle
 * @return 0xD8E007F7 (BAD_HANDLE) if it isn't.
 */
Result svcGetThreadInfo(s64* out, Handle thread, ThreadInfoType type);
///@}


///@name Synchronization
///@{
Result svcCreateMutex(Handle* mutex, bool initially_locked);
Result svcReleaseMutex(Handle handle);

Result svcCreateSemaphore(Handle* semaphore, s32 initial_count, s32 max_count);
Result svcReleaseSemaphore(s32* count, Handle semaphore, s32 release_count);

Result svcCreateEvent(Handle* event, u8 reset_type);
Result svcSignalEvent(Handle handle);
Result svcClearEvent(Handle handle);

Result svcWaitSynchronization(Handle handle, s64 nanoseconds);
Result svcWaitSynchronizationN(s32* out, Handle* handles, s32 handles_num, bool wait_all, s64 nanoseconds);

/**
 * @brief Creates an address arbiter
 * @sa svcArbitrateAddress
 */
Result svcCreateAddressArbiter(Handle *arbiter);

/**
 * @brief Arbitrate an address, can be used for synchronization
 * @param arbiter Handle of the arbiter
 * @param addr A pointer to a s32 value.
 * @param type Type of action to be performed by the arbiter
 * @param value Number of threads to signal if using @ref ARBITRATION_SIGNAL, or the value used for comparison.
 * 
 * This will perform an arbitration based on #type. The comparisons are done between #value and the value at the address #addr.
 * 
 * @code
 * s32 val=0;
 * // Does *nothing* since val >= 0
 * svcCreateAddressArbiter(arbiter,&val,ARBITRATION_WAIT_IF_LESS_THAN,0,0);
 * // Thread will wait for a signal or wake up after 10000000 nanoseconds because val < 1.
 * svcCreateAddressArbiter(arbiter,&val,ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT,1,10000000ULL);
 * @endcode
 */
Result svcArbitrateAddress(Handle arbiter, u32 addr, ArbitrationType type, s32 value, s64 nanoseconds);

Result svcSendSyncRequest(Handle session);
Result svcAcceptSession(Handle* session, Handle port);
Result svcReplyAndReceive(s32* index, Handle* handles, s32 handleCount, Handle replyTarget);
///@}

///@name Time
///@{
Result svcCreateTimer(Handle* timer, u8 reset_type);
Result svcSetTimer(Handle timer, s64 initial, s64 interval);
Result svcCancelTimer(Handle timer);
Result svcClearTimer(Handle timer);
u64    svcGetSystemTick();
///@}

///@name System
///@{
Result svcCloseHandle(Handle handle);
Result svcDuplicateHandle(Handle* out, Handle original);
Result svcGetSystemInfo(s64* out, u32 type, s32 param);
Result svcKernelSetState(u32 type, u32 param0, u32 param1, u32 param2);
///@}


///@name Debugging
///@{
void svcBreak(UserBreakType breakReason);
Result svcOutputDebugString(const char* str, int length);
Result svcDebugActiveProcess(Handle* debug, u32 processId);
Result svcBreakDebugProcess(Handle debug);
Result svcTerminateDebugProcess(Handle debug);
Result svcGetProcessDebugEvent(DebugEventInfo* info, Handle debug);
Result svcContinueDebugEvent(Handle debug, u32 flags);
///@}

Result svcBackdoor(s32 (*callback)(void));


