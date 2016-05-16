/**
 * @file svc.h
 * @brief Syscall wrappers.
 */
#pragma once

#include "types.h"

/// Pseudo handle for the current process
#define CUR_PROCESS_HANDLE 0xFFFF8001

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

	MEMOP_REGION_APP    = 0x100, ///< APPLICATION memory region.
	MEMOP_REGION_SYSTEM = 0x200, ///< SYSTEM memory region.
	MEMOP_REGION_BASE   = 0x300, ///< BASE memory region.

	MEMOP_OP_MASK     = 0xFF,    ///< Operation bitmask.
	MEMOP_REGION_MASK = 0xF00,   ///< Region bitmask.
	MEMOP_LINEAR_FLAG = 0x10000, ///< Flag for linear memory operations

	MEMOP_ALLOC_LINEAR = MEMOP_LINEAR_FLAG | MEMOP_ALLOC, ///< Allocates linear memory.
} MemOp;

/// The state of a memory block.
typedef enum {
	MEMSTATE_FREE       = 0,  ///< Free memory
	MEMSTATE_RESERVED   = 1,  ///< Reserved memory
	MEMSTATE_IO         = 2,  ///< I/O memory
	MEMSTATE_STATIC     = 3,  ///< Static memory
	MEMSTATE_CODE       = 4,  ///< Code memory
	MEMSTATE_PRIVATE    = 5,  ///< Private memory
	MEMSTATE_SHARED     = 6,  ///< Shared memory
	MEMSTATE_CONTINUOUS = 7,  ///< Continuous memory
	MEMSTATE_ALIASED    = 8,  ///< Aliased memory
	MEMSTATE_ALIAS      = 9,  ///< Alias memory
	MEMSTATE_ALIASCODE  = 10, ///< Aliased code memory
	MEMSTATE_LOCKED     = 11  ///< Locked memory
} MemState;

/// Memory permission flags
typedef enum {
	MEMPERM_READ     = 1,         ///< Readable
	MEMPERM_WRITE    = 2,         ///< Writable
	MEMPERM_EXECUTE  = 4,         ///< Executable
	MEMPERM_DONTCARE = 0x10000000 ///< Don't care
} MemPerm;

/// Memory information.
typedef struct {
    u32 base_addr; ///< Base address.
    u32 size;      ///< Size.
    u32 perm;      ///< Memory permissions. See @ref MemPerm
    u32 state;     ///< Memory state. See @ref MemState
} MemInfo;

/// Memory page information.
typedef struct {
    u32 flags; ///< Page flags.
} PageInfo;

/// Arbitration modes.
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

/// Types of thread info.
typedef enum {
	THREADINFO_TYPE_UNKNOWN ///< Unknown.
} ThreadInfoType;

/// Pseudo handle for the current thread
#define CUR_THREAD_HANDLE 0xFFFF8000

///@}


///@name Debugging
///@{

/// Reasons for a process event.
typedef enum {
	REASON_CREATE = 1, ///< Process created.
	REASON_ATTACH = 2  ///< Process attached.
} ProcessEventReason;

/// Event relating to a process.
typedef struct {
	u64 program_id;      ///< ID of the program.
	u8  process_name[8]; ///< Name of the process.
	u32 process_id;      ///< ID of the process.
	u32 reason;          ///< Reason for the event. See @ref ProcessEventReason
} ProcessEvent;

/// Reasons for an exit process event.
typedef enum {
	EXITPROCESS_EVENT_NONE                = 0, ///< No reason.
	EXITPROCESS_EVENT_TERMINATE           = 1, ///< Process terminated.
	EXITPROCESS_EVENT_UNHANDLED_EXCEPTION = 2  ///< Unhandled exception occurred.
} ExitProcessEventReason;

/// Event relating to the exiting of a process.
typedef struct {
	u32 reason; ///< Reason for exiting. See @ref ExitProcessEventReason
} ExitProcessEvent;

/// Event relating to the creation of a thread.
typedef struct {
	u32 creator_thread_id; ///< ID of the creating thread.
	u32 base_addr;         ///< Base address.
	u32 entry_point;       ///< Entry point of the thread.
} CreateThreadEvent;

/// Reasons for an exit thread event.
typedef enum {
	EXITTHREAD_EVENT_NONE              = 0, ///< No reason.
	EXITTHREAD_EVENT_TERMINATE         = 1, ///< Thread terminated.
	EXITTHREAD_EVENT_UNHANDLED_EXC     = 2, ///< Unhandled exception occurred.
	EXITTHREAD_EVENT_TERMINATE_PROCESS = 3  ///< Process terminated.
} ExitThreadEventReason;

/// Event relating to the exiting of a thread.
typedef struct {
	u32 reason; ///< Reason for exiting. See @ref ExitThreadEventReason
} ExitThreadEvent;

/// Reasons for a user break.
typedef enum {
	USERBREAK_PANIC  = 0, ///< Panic.
	USERBREAK_ASSERT = 1, ///< Assertion failed.
	USERBREAK_USER   = 2  ///< User related.
} UserBreakType;

/// Reasons for an exception event.
typedef enum {
	EXC_EVENT_UNDEFINED_INSTRUCTION = 0, ///< Undefined instruction.   arg: (None)
	EXC_EVENT_UNKNOWN1              = 1, ///< Unknown.                 arg: (None)
	EXC_EVENT_UNKNOWN2              = 2, ///< Unknown.                 arg: address
	EXC_EVENT_UNKNOWN3              = 3, ///< Unknown.                 arg: address
	EXC_EVENT_ATTACH_BREAK          = 4, ///< Attached break.          arg: (None)
	EXC_EVENT_BREAKPOINT            = 5, ///< Breakpoint reached.      arg: (None)
	EXC_EVENT_USER_BREAK            = 6, ///< User break occurred.     arg: @ref UserBreakType
	EXC_EVENT_DEBUGGER_BREAK        = 7, ///< Debugger break occurred. arg: (None)
	EXC_EVENT_UNDEFINED_SYSCALL     = 8  ///< Undefined syscall.       arg: attempted syscall
} ExceptionEventType;

/// Event relating to exceptions.
typedef struct {
	u32 type;     ///< Type of event. See @ref ExceptionEventType
	u32 address;  ///< Address of the exception.
	u32 argument; ///< Event argument. See @ref ExceptionEventType
} ExceptionEvent;

/// Event relating to the scheduler.
typedef struct {
	u64 clock_tick; ///< Clock tick that the event occurred.
} SchedulerInOutEvent;

/// Event relating to syscalls.
typedef struct {
	u64 clock_tick; ///< Clock tick that the event occurred.
	u32 syscall;    ///< Syscall sent/received.
} SyscallInOutEvent;

/// Event relating to debug output.
typedef struct {
	u32 string_addr; ///< Address of the outputted string.
	u32 string_size; ///< Size of the outputted string.
} OutputStringEvent;

/// Event relating to the mapping of memory.
typedef struct {
	u32 mapped_addr; ///< Mapped address.
	u32 mapped_size; ///< Mapped size.
	u32 memperm;     ///< Memory permissions. See @ref MemPerm
	u32 memstate;    ///< Memory state. See @ref MemState
} MapEvent;

/// Debug event type.
typedef enum {
	DBG_EVENT_PROCESS        = 0,  ///< Process event.
	DBG_EVENT_CREATE_THREAD  = 1,  ///< Thread creation event.
	DBG_EVENT_EXIT_THREAD    = 2,  ///< Thread exit event.
	DBG_EVENT_EXIT_PROCESS   = 3,  ///< Process exit event.
	DBG_EVENT_EXCEPTION      = 4,  ///< Exception event.
	DBG_EVENT_DLL_LOAD       = 5,  ///< DLL load event.
	DBG_EVENT_DLL_UNLOAD     = 6,  ///< DLL unload event.
	DBG_EVENT_SCHEDULE_IN    = 7,  ///< Schedule in event.
	DBG_EVENT_SCHEDULE_OUT   = 8,  ///< Schedule out event.
	DBG_EVENT_SYSCALL_IN     = 9,  ///< Syscall in event.
	DBG_EVENT_SYSCALL_OUT    = 10, ///< Syscall out event.
	DBG_EVENT_OUTPUT_STRING  = 11, ///< Output string event.
	DBG_EVENT_MAP            = 12  ///< Map event.
} DebugEventType;

/// Information about a debug event.
typedef struct {
	u32 type;       ///< Type of event. See @ref DebugEventType
	u32 thread_id;  ///< ID of the thread.
	u32 unknown[2]; ///< Unknown data.
	union {
		ProcessEvent process;            ///< Process event data.
		CreateThreadEvent create_thread; ///< Thread creation event data.
		ExitThreadEvent exit_thread;     ///< Thread exit event data.
		ExitProcessEvent exit_process;   ///< Process exit event data.
		ExceptionEvent exception;        ///< Exception event data.
		/* TODO: DLL_LOAD */
		/* TODO: DLL_UNLOAD */
		SchedulerInOutEvent scheduler;   ///< Schedule in/out event data.
		SyscallInOutEvent syscall;       ///< Syscall in/out event data.
		OutputStringEvent output_string; ///< Output string event data.
		MapEvent map;                    ///< Map event data.
	};
} DebugEventInfo;

///@}

///@name Processes
///@{

/// Information on address space for process. All sizes are in pages (0x1000 bytes)
typedef struct {
	u8 name[8];           ///< ASCII name of codeset
	u16 unk1;
	u16 unk2;
	u32 unk3;
	u32 text_addr;        ///< .text start address
	u32 text_size;        ///< .text number of pages
	u32 ro_addr;          ///< .rodata start address
	u32 ro_size;          ///< .rodata number of pages
	u32 rw_addr;          ///< .data, .bss start address
	u32 rw_size;          ///< .data number of pages
	u32 text_size_total;  ///< total pages for .text (aligned)
	u32 ro_size_total;    ///< total pages for .rodata (aligned)
	u32 rw_size_total;    ///< total pages for .data, .bss (aligned)
	u32 unk4;
	u64 program_id;       ///< Program ID
} CodeSetInfo;

/// Information for the main thread of a process.
typedef struct
{
	int priority;   ///< Priority of the main thread.
	u32 stack_size; ///< Size of the stack of the main thread.
	int argc;       ///< Unused on retail kernel.
	u16* argv;      ///< Unused on retail kernel.
	u16* envp;      ///< Unused on retail kernel.
} StartupInfo;

///@}

/**
 * @brief Gets the thread local storage buffer.
 * @return The thread local storage bufger.
 */
static inline void* getThreadLocalStorage(void)
{
	void* ret;
	__asm__ ("mrc p15, 0, %[data], c13, c0, 3" : [data] "=r" (ret));
	return ret;
}

/**
 * @brief Gets the thread command buffer.
 * @return The thread command bufger.
 */
static inline u32* getThreadCommandBuffer(void)
{
	return (u32*)((u8*)getThreadLocalStorage() + 0x80);
}

/**
 * @brief Gets the thread static buffer.
 * @return The thread static bufger.
 */
static inline u32* getThreadStaticBuffers(void)
{
	return (u32*)((u8*)getThreadLocalStorage() + 0x180);
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
 *                 Value 0 is used when unmapping memory.
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
 * @param[out] memblock Pointer to store the handle of the block
 * @param addr Address of the memory to map, page-aligned. So its alignment must be 0x1000.
 * @param size Size of the memory to map, a multiple of 0x1000.
 * @param my_perm Memory permissions for the current process
 * @param other_perm Memory permissions for the other processes
 *
 * @note The shared memory block, and its rights, are destroyed when the handle is closed.
 */
Result svcCreateMemoryBlock(Handle* memblock, u32 addr, u32 size, MemPerm my_perm, MemPerm other_perm);

/**
 * @brief Maps a block of shared memory
 * @param memblock Handle of the block
 * @param addr Address of the memory to map, page-aligned. So its alignment must be 0x1000.
 * @param my_perm Memory permissions for the current process
 * @param other_perm Memory permissions for the other processes
 *
 * @note The shared memory block, and its rights, are destroyed when the handle is closed.
 */
Result svcMapMemoryBlock(Handle memblock, u32 addr, MemPerm my_perm, MemPerm other_perm);

/**
 * @brief Maps a block of process memory.
 * @param process Handle of the process.
 * @param startAddr Start address of the memory to map.
 * @param endAddr End address of the memory to map.
 */
Result svcMapProcessMemory(Handle process, u32 startAddr, u32 endAddr);

/**
 * @brief Unmaps a block of process memory.
 * @param process Handle of the process.
 * @param startAddr Start address of the memory to unmap.
 * @param endAddr End address of the memory to unmap.
 */
Result svcUnmapProcessMemory(Handle process, u32 startAddr, u32 endAddr);

/**
 * @brief Unmaps a block of shared memory
 * @param memblock Handle of the block
 * @param addr Address of the memory to unmap, page-aligned. So its alignment must be 0x1000.
 */
Result svcUnmapMemoryBlock(Handle memblock, u32 addr);

/**
 * @brief Begins an inter-process DMA.
 * @param[out] dma Pointer to output the handle of the DMA to.
 * @param dstProcess Destination process.
 * @param dst Buffer to write data to.
 * @param srcprocess Source process.
 * @param src Buffer to read data from.
 * @param size Size of the data to DMA.
 * @param dmaConfig DMA configuration data.
 */
Result svcStartInterProcessDma(Handle* dma, Handle dstProcess, void* dst, Handle srcProcess, const void* src, u32 size, void* dmaConfig);

/**
 * @brief Terminates an inter-process DMA.
 * @param dma Handle of the DMA.
 */
Result svcStopDma(Handle dma);

/**
 * @brief Gets the state of an inter-process DMA.
 * @param[out] dmaState Pointer to output the state of the DMA to.
 * @param dma Handle of the DMA.
 */
Result svcGetDmaState(void* dmaState, Handle dma);

/**
 * @brief Queries memory information.
 * @param[out] info Pointer to output memory info to.
 * @param out Pointer to output page info to.
 * @param addr Virtual memory address to query.
 */
Result svcQueryMemory(MemInfo* info, PageInfo* out, u32 addr);

/**
 * @brief Queries process memory information.
 * @param[out] info Pointer to output memory info to.
 * @param[out] out Pointer to output page info to.
 * @param process Process to query memory from.
 * @param addr Virtual memory address to query.
 */
Result svcQueryProcessMemory(MemInfo* info, PageInfo* out, Handle process, u32 addr);

/**
 * @brief Invalidates a process's data cache.
 * @param process Handle of the process.
 * @param addr Address to invalidate.
 * @param size Size of the memory to invalidate.
 */
Result svcInvalidateProcessDataCache(Handle process, void* addr, u32 size);

/**
 * @brief Flushes a process's data cache.
 * @param process Handle of the process.
 * @param addr Address to flush.
 * @param size Size of the memory to flush.
 */
Result svcFlushProcessDataCache(Handle process, void const* addr, u32 size);

/**
 * @brief Reads from a process's memory.
 * @param buffer Buffer to read data to.
 * @param debug Debug handle of the process.
 * @param addr Address to read from.
 * @param size Size of the memory to read.
 */
Result svcReadProcessMemory(void* buffer, Handle debug, u32 addr, u32 size);

/**
 * @brief Writes to a process's memory.
 * @param debug Debug handle of the process.
 * @param buffer Buffer to write data from.
 * @param addr Address to write to.
 * @param size Size of the memory to write.
 */
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

/// Exits the current process.
void svcExitProcess() __attribute__((noreturn));

/**
 * @brief Terminates a process.
 * @param process Handle of the process to terminate.
 */
Result svcTerminateProcess(Handle process);

/**
 * @brief Gets information about a process.
 * @param[out] out Pointer to output process info to.
 * @param process Handle of the process to get information about.
 * @param type Type of information to retreieve.
 */
Result svcGetProcessInfo(s64* out, Handle process, u32 type);

/**
 * @brief Gets the ID of a process.
 * @param[out] out Pointer to output the process ID to.
 * @param handle Handle of the process to get the ID of.
 */
Result svcGetProcessId(u32 *out, Handle handle);

/**
 * @brief Gets a list of running processes.
 * @param[out] processCount Pointer to output the process count to.
 * @param[out] processIds Pointer to output the process IDs to.
 * @param processIdMaxCount Maximum number of process IDs.
 */
Result svcGetProcessList(s32* processCount, u32* processIds, s32 processIdMaxCount);

/**
 * @brief Creates a port.
 * @param[out] portServer Pointer to output the port server handle to.
 * @param[out] portClient Pointer to output the port client handle to.
 * @param name Name of the port.
 * @param maxSessions Maximum number of sessions that can connect to the port.
 */
Result svcCreatePort(Handle* portServer, Handle* portClient, const char* name, s32 maxSessions);

/**
 * @brief Connects to a port.
 * @param[out] out Pointer to output the port handle to.
 * @param portName Name of the port.
 */
Result svcConnectToPort(volatile Handle* out, const char* portName);

/**
 * @brief Sets up virtual address space for a new process
 * @param[out] out Pointer to output the code set handle to.
 * @param info Description for setting up the addresses
 * @param code_ptr Pointer to .text in shared memory
 * @param ro_ptr Pointer to .rodata in shared memory
 * @param data_ptr Pointer to .data in shared memory
 */
Result svcCreateCodeSet(Handle* out, const CodeSetInfo *info, void* code_ptr, void* ro_ptr, void* data_ptr);

/**
 * @brief Sets up virtual address space for a new process
 * @param[out] out Pointer to output the process handle to.
 * @param codeset Codeset created for this process
 * @param arm11kernelcaps ARM11 Kernel Capabilities from exheader
 * @param arm11kernelcaps_num Number of kernel capabilities
 */
Result svcCreateProcess(Handle* out, Handle codeset, const u32 *arm11kernelcaps, u32 arm11kernelcaps_num);

/**
 * @brief Sets a process's affinity mask.
 * @param process Handle of the process.
 * @param affinitymask Pointer to retrieve the affinity masks from.
 * @param processorcount Number of processors.
 */
Result svcSetProcessAffinityMask(Handle process, const u8* affinitymask, s32 processorcount);

/**
 * Sets a process's ideal processor.
 * @param process Handle of the process.
 * @param processorid ID of the thread's ideal processor.
 */
Result svcSetProcessIdealProcessor(Handle process, s32 processorid);

/**
 * Launches the main thread of the process.
 * @param process Handle of the process.
 * @param info Pointer to a StartupInfo structure describing information for the main thread.
 */
Result svcRun(Handle process, const StartupInfo* info);

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

/// Retrieves the priority of a thread.
Result svcGetThreadPriority(s32 *out, Handle handle);

/**
 * @brief Changes the priority of a thread
 * @param prio For userland apps, this has to be within the range [0x18;0x3F]
 *
 * Low values gives the thread higher priority.
 */
Result svcSetThreadPriority(Handle thread, s32 prio);

/**
 * @brief Gets a thread's affinity mask.
 * @param[out] affinitymask Pointer to output the affinity masks to.
 * @param thread Handle of the thread.
 * @param processorcount Number of processors.
 */
Result svcGetThreadAffinityMask(u8* affinitymask, Handle thread, s32 processorcount);

/**
 * @brief Sets a thread's affinity mask.
 * @param thread Handle of the thread.
 * @param affinitymask Pointer to retrieve the affinity masks from.
 * @param processorcount Number of processors.
 */
Result svcSetThreadAffinityMask(Handle thread, const u8* affinitymask, s32 processorcount);

/**
 * @brief Gets a thread's ideal processor.
 * @param[out] processorid Pointer to output the ID of the thread's ideal processor to.
 * @param thread Handle of the thread.
 */
Result svcGetThreadIdealProcessor(s32* processorid, Handle thread);

/**
 * Sets a thread's ideal processor.
 * @param thread Handle of the thread.
 * @param processorid ID of the thread's ideal processor.
 */
Result svcSetThreadIdealProcessor(Handle thread, s32 processorid);

/**
 * @brief Returns the ID of the processor the current thread is running on.
 * @sa svcCreateThread
 */
s32    svcGetProcessorID(void);

/**
 * @brief Gets the ID of a thread.
 * @param[out] out Pointer to output the thread ID of the thread @p handle to.
 * @param handle Handle of the thread.
 */
Result svcGetThreadId(u32 *out, Handle handle);

/**
 * @brief Gets the resource limit set of a process.
 * @param[out] resourceLimit Pointer to output the resource limit set handle to.
 * @param process Process to get the resource limits of.
 */
Result svcGetResourceLimit(Handle* resourceLimit, Handle process);

/**
 * @brief Gets the value limits of a resource limit set.
 * @param[out] values Pointer to output the value limits to.
 * @param resourceLimit Resource limit set to use.
 * @param names Resource limit names to get the limits of.
 * @param nameCount Number of resource limit names.
 */
Result svcGetResourceLimitLimitValues(s64* values, Handle resourceLimit, u32* names, s32 nameCount);

/**
 * @brief Gets the values of a resource limit set.
 * @param[out] values Pointer to output the values to.
 * @param resourceLimit Resource limit set to use.
 * @param names Resource limit names to get the values of.
 * @param nameCount Number of resource limit names.
 */
Result svcGetResourceLimitCurrentValues(s64* values, Handle resourceLimit, u32* names, s32 nameCount);

/**
 * @brief Gets the process ID of a thread.
 * @param[out] out Pointer to output the process ID of the thread @p handle to.
 * @param handle Handle of the thread.
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
/**
 * @brief Creates a mutex.
 * @param[out] mutex Pointer to output the handle of the created mutex to.
 * @param initially_locked Whether the mutex should be initially locked.
 */
Result svcCreateMutex(Handle* mutex, bool initially_locked);

/**
 * @brief Releases a mutex.
 * @param handle Handle of the mutex.
 */
Result svcReleaseMutex(Handle handle);

/**
 * @brief Creates a semaphore.
 * @param[out] semaphore Pointer to output the handle of the created semaphore to.
 * @param initial_count Initial count of the semaphore.
 * @param max_count Maximum count of the semaphore.
 */
Result svcCreateSemaphore(Handle* semaphore, s32 initial_count, s32 max_count);

/**
 * @brief Releases a semaphore.
 * @param[out] count Pointer to output the current count of the semaphore to.
 * @param semaphore Handle of the semaphore.
 * @param release_count Number to increase the semaphore count by.
 */
Result svcReleaseSemaphore(s32* count, Handle semaphore, s32 release_count);

/**
 * @brief Creates an event handle.
 * @param[out] event Pointer to output the created event handle to.
 * @param reset_type Type of reset the event uses.
 */
Result svcCreateEvent(Handle* event, u8 reset_type);

/**
 * @brief Signals an event.
 * @param handle Handle of the event to signal.
 */
Result svcSignalEvent(Handle handle);

/**
 * @brief Clears an event.
 * @param handle Handle of the event to clear.
 */
Result svcClearEvent(Handle handle);

/**
 * @brief Waits for synchronization on a handle.
 * @param handle Handle to wait on.
 * @param nanoseconds Maximum nanoseconds to wait for.
 */
Result svcWaitSynchronization(Handle handle, s64 nanoseconds);

/**
 * @brief Waits for synchronization on multiple handles.
 * @param[out] out Pointer to output the index of the synchronized handle to.
 * @param handles Handles to wait on.
 * @param handles_num Number of handles.
 * @param wait_all Whether to wait for synchronization on all handles.
 * @param nanoseconds Maximum nanoseconds to wait for.
 */
Result svcWaitSynchronizationN(s32* out, Handle* handles, s32 handles_num, bool wait_all, s64 nanoseconds);

/**
 * @brief Creates an address arbiter
 * @param[out] mutex Pointer to output the handle of the created address arbiter to.
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

/**
 * @brief Sends a synchronized request to a session handle.
 * @param session Handle of the session.
 */
Result svcSendSyncRequest(Handle session);

/**
 * @brief Accepts a session.
 * @param[out] session Pointer to output the created session handle to.
 * @param port Handle of the port to accept a session from.
 */
Result svcAcceptSession(Handle* session, Handle port);

/**
 * @brief Replies to and receives a new request.
 * @param index Pointer to the index of the request.
 * @param handles Session handles to receive requests from.
 * @param handleCount Number of handles.
 * @param replyTarget Handle of the session to reply to.
 */
Result svcReplyAndReceive(s32* index, Handle* handles, s32 handleCount, Handle replyTarget);
///@}

///@name Time
///@{
/**
 * @brief Creates a timer.
 * @param[out] timer Pointer to output the handle of the created timer to.
 * @param reset_type Type of reset to perform on the timer.
 */
Result svcCreateTimer(Handle* timer, u8 reset_type);

/**
 * @brief Sets a timer.
 * @param timer Handle of the timer to set.
 * @param initial Initial value of the timer.
 * @param interval Interval of the timer.
 */
Result svcSetTimer(Handle timer, s64 initial, s64 interval);

/**
 * @brief Cancels a timer.
 * @param timer Handle of the timer to cancel.
 */
Result svcCancelTimer(Handle timer);

/**
 * @brief Clears a timer.
 * @param timer Handle of the timer to clear.
 */
Result svcClearTimer(Handle timer);

/**
 * @brief Gets the current system tick.
 * @return The current system tick.
 */
u64    svcGetSystemTick(void);
///@}

///@name System
///@{
/**
 * @brief Closes a handle.
 * @param handle Handle to close.
 */
Result svcCloseHandle(Handle handle);

/**
 * @brief Duplicates a handle.
 * @param[out] out Pointer to output the duplicated handle to.
 * @param original Handle to duplicate.
 */
Result svcDuplicateHandle(Handle* out, Handle original);

/**
 * @brief Gets the system info.
 * @param[out] out Pointer to output the system info to.
 * @param type Type of system info to retrieve.
 * @param param Parameter clarifying the system info type.
 */
Result svcGetSystemInfo(s64* out, u32 type, s32 param);

/**
 * @brief Sets the current kernel state.
 * @param type Type of state to set.
 * @param param0 First parameter of the state.
 * @param param1 Second parameter of the state.
 * @param param2 Third parameter of the state.
 */
Result svcKernelSetState(u32 type, u32 param0, u32 param1, u32 param2);
///@}


///@name Debugging
///@{
/**
 * @brief Breaks execution.
 * @param breakReason Reason for breaking.
 */
void svcBreak(UserBreakType breakReason);

/**
 * @brief Outputs a debug string.
 * @param str String to output.
 * @param length Length of the string to output.
 */
Result svcOutputDebugString(const char* str, int length);
/**
 * @brief Creates a debug handle for an active process.
 * @param[out] debug Pointer to output the created debug handle to.
 * @param processId ID of the process to debug.
 */
Result svcDebugActiveProcess(Handle* debug, u32 processId);

/**
 * @brief Breaks a debugged process.
 * @param debug Debug handle of the process.
 */
Result svcBreakDebugProcess(Handle debug);

/**
 * @brief Terminates a debugged process.
 * @param debug Debug handle of the process.
 */
Result svcTerminateDebugProcess(Handle debug);

/**
 * @brief Gets the current debug event of a debugged process.
 * @param[out] info Pointer to output the debug event information to.
 * @param debug Debug handle of the process.
 */
Result svcGetProcessDebugEvent(DebugEventInfo* info, Handle debug);

/**
 * @brief Continues the current debug event of a debugged process.
 * @param debug Debug handle of the process.
 * @param flags Flags to continue with.
 */
Result svcContinueDebugEvent(Handle debug, u32 flags);
///@}

/**
 * @brief Executes a function in kernel mode.
 * @param callback Function to execute.
 */
Result svcBackdoor(s32 (*callback)(void));


