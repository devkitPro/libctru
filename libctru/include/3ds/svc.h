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
	MEMSTATE_LOCKED     = 11, ///< Locked memory
} MemState;

/// Memory permission flags
typedef enum {
	MEMPERM_READ     = 1,          ///< Readable
	MEMPERM_WRITE    = 2,          ///< Writable
	MEMPERM_EXECUTE  = 4,          ///< Executable
	MEMPERM_READWRITE = MEMPERM_READ | MEMPERM_WRITE, ///< Readable and writable
	MEMPERM_READEXECUTE = MEMPERM_READ | MEMPERM_EXECUTE, ///< Readable and executable
	MEMPERM_DONTCARE = 0x10000000, ///< Don't care
} MemPerm;

/// Memory regions.
typedef enum
{
	MEMREGION_ALL = 0,         ///< All regions.
	MEMREGION_APPLICATION = 1, ///< APPLICATION memory.
	MEMREGION_SYSTEM = 2,      ///< SYSTEM memory.
	MEMREGION_BASE = 3,        ///< BASE memory.
} MemRegion;

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

/// Reset types (for use with events and timers)
typedef enum {
	RESET_ONESHOT = 0, ///< When the primitive is signaled, it will wake up exactly one thread and will clear itself automatically.
	RESET_STICKY  = 1, ///< When the primitive is signaled, it will wake up all threads and it won't clear itself automatically.
	RESET_PULSE   = 2, ///< Only meaningful for timers: same as ONESHOT but it will periodically signal the timer instead of just once.
} ResetType;

/// Types of thread info.
typedef enum {
	THREADINFO_TYPE_UNKNOWN ///< Unknown.
} ThreadInfoType;

/// Types of resource limit
typedef enum {
	RESLIMIT_PRIORITY       = 0,        ///< Thread priority
	RESLIMIT_COMMIT         = 1,        ///< Quantity of allocatable memory
	RESLIMIT_THREAD         = 2,        ///< Number of threads
	RESLIMIT_EVENT          = 3,        ///< Number of events
	RESLIMIT_MUTEX          = 4,        ///< Number of mutexes
	RESLIMIT_SEMAPHORE      = 5,        ///< Number of semaphores
	RESLIMIT_TIMER          = 6,        ///< Number of timers
	RESLIMIT_SHAREDMEMORY   = 7,        ///< Number of shared memory objects, see @ref svcCreateMemoryBlock
	RESLIMIT_ADDRESSARBITER = 8,        ///< Number of address arbiters
	RESLIMIT_CPUTIME        = 9,        ///< CPU time. Value expressed in percentage regular until it reaches 90.

	RESLIMIT_BIT            = BIT(31),  ///< Forces enum size to be 32 bits
} ResourceLimitType;

/// Pseudo handle for the current thread
#define CUR_THREAD_HANDLE  0xFFFF8000

///@}

///@name Device drivers
///@{

/// DMA transfer state.
typedef enum {
	DMASTATE_STARTING = 0,  ///< DMA transfer involving at least one device is starting and has not reached DMAWFP yet.
	DMASTATE_WFP_DST = 1,   ///< DMA channel is in WFP state for the destination device (2nd loop iteration onwards).
	DMASTATE_WFP_SRC = 2,   ///< DMA channel is in WFP state for the source device (2nd loop iteration onwards).
	DMASTATE_RUNNING = 3,   ///< DMA transfer is running.
	DMASTATE_DONE = 4,      ///< DMA transfer is done.
} DmaState;

/// Configuration flags for \ref DmaConfig.
enum {
	DMACFG_SRC_IS_DEVICE        = BIT(0), ///< DMA source is a device/peripheral. Address will not auto-increment.
	DMACFG_DST_IS_DEVICE        = BIT(1), ///< DMA destination is a device/peripheral. Address will not auto-increment.
	DMACFG_WAIT_AVAILABLE       = BIT(2), ///< Make \ref svcStartInterProcessDma wait for the channel to be unlocked.
	DMACFG_KEEP_LOCKED          = BIT(3), ///< Keep the channel locked after the transfer. Required for \ref svcRestartDma.
	DMACFG_USE_SRC_CONFIG       = BIT(6), ///< Use the provided source device configuration even if the DMA source is not a device.
	DMACFG_USE_DST_CONFIG       = BIT(7), ///< Use the provided destination device configuration even if the DMA destination is not a device.
};

/// Configuration flags for \ref svcRestartDma.
enum {
	DMARST_UNLOCK           = BIT(0), ///< Unlock the channel after transfer.
	DMARST_RESUME_DEVICE    = BIT(1), ///< Replace DMAFLUSHP instructions by NOP (they may not be regenerated even if this flag is not set).
};

/**
 * @brief Device configuration structure, part of \ref DmaConfig.
 * @note
 * - if (and only if) src/dst is a device, then src/dst won't be auto-incremented.
 * - the kernel uses DMAMOV instead of DMAADNH, when having to decrement (possibly working around an erratum);
 * this forces all loops to be unrolled -- you need to keep that in mind when using negative increments, as the kernel
 * uses a limit of 100 DMA instruction bytes per channel.
 */
typedef struct {
	s8 deviceId;            ///< DMA device ID.
	s8 allowedAlignments;   ///< Mask of allowed access alignments (8, 4, 2, 1).
	s16 burstSize;          ///< Number of bytes transferred in a burst loop. Can be 0 (in which case the max allowed alignment is used as unit).
	s16 transferSize;       ///< Number of bytes transferred in a "transfer" loop (made of burst loops).
	s16 burstStride;        ///< Burst loop stride, can be <= 0.
	s16 transferStride;     ///< "Transfer" loop stride, can be <= 0.
} DmaDeviceConfig;

/// Configuration stucture for \ref svcStartInterProcessDma.
typedef struct {
	s8 channelId;           ///< Channel ID (Arm11: 0-7, Arm9: 0-1). Use -1 to auto-assign to a free channel (Arm11: 3-7, Arm9: 0-1).
	s8 endianSwapSize;      ///< Endian swap size (can be 0).
	u8 flags;               ///< DMACFG_* flags.
	u8 _padding;
	DmaDeviceConfig srcCfg; ///< Source device configuration, read if \ref DMACFG_SRC_IS_DEVICE and/or \ref DMACFG_USE_SRC_CONFIG are set.
	DmaDeviceConfig dstCfg; ///< Destination device configuration, read if \ref DMACFG_SRC_IS_DEVICE and/or \ref DMACFG_USE_SRC_CONFIG are set.
} DmaConfig;

///@}

///@name Debugging
///@{

/// Operations for \ref svcControlPerformanceCounter
typedef enum {
	PERFCOUNTEROP_ENABLE                        = 0,    ///< Enable and lock perfmon. functionality.
	PERFCOUNTEROP_DISABLE                       = 1,    ///< Disable and forcibly unlock perfmon. functionality.
	PERFCOUNTEROP_GET_VALUE                     = 2,    ///< Get the value of a counter register.
	PERFCOUNTEROP_SET_VALUE                     = 3,    ///< Set the value of a counter register.
	PERFCOUNTEROP_GET_OVERFLOW_FLAGS            = 4,    ///< Get the overflow flags for all CP15 and SCU counters.
	PERFCOUNTEROP_RESET                         = 5,    ///< Reset the value and/or overflow flags of selected counters.
	PERFCOUNTEROP_GET_EVENT                     = 6,    ///< Get the event ID associated to a particular counter.
	PERFCOUNTEROP_SET_EVENT                     = 7,    ///< Set the event ID associated to a paritcular counter.
	PERFCOUNTEROP_SET_VIRTUAL_COUNTER_ENABLED   = 8,    ///< (Dis)allow the kernel to track counter overflows and to use 64-bit counter values.
} PerfCounterOperation;

/// Performance counter register IDs (CP15 and SCU).
typedef enum
{
	// CP15 registers:
	PERFCOUNTERREG_CORE_BASE = 0,
	PERFCOUNTERREG_CORE_COUNT_REG_0 = PERFCOUNTERREG_CORE_BASE, ///< CP15 PMN0.
	PERFCOUNTERREG_CORE_COUNT_REG_1,   ///< CP15 PMN1.
	PERFCOUNTERREG_CORE_CYCLE_COUNTER, ///< CP15 CCNT.

	// SCU registers
	PERFCOUNTERREG_SCU_BASE = 0x10,
	PERFCOUNTERREG_SCU_0 = PERFCOUNTERREG_SCU_BASE, ///< SCU MN0.
	PERFCOUNTERREG_SCU_1,  ///< SCU MN1.
	PERFCOUNTERREG_SCU_2,  ///< SCU MN2.
	PERFCOUNTERREG_SCU_3,  ///< SCU MN3.
	PERFCOUNTERREG_SCU_4,  ///< SCU MN4. Prod-N3DS only. IRQ line missing.
	PERFCOUNTERREG_SCU_5,  ///< SCU MN5. Prod-N3DS only. IRQ line missing.
	PERFCOUNTERREG_SCU_6,  ///< SCU MN6. Prod-N3DS only. IRQ line missing.
	PERFCOUNTERREG_SCU_7,  ///< SCU MN7. Prod-N3DS only. IRQ line missing.
} PerfCounterRegister;

/**
 * @brief Performance counter event IDs (CP15 or SCU).
 * 
 * @note Refer to:
 *     - CP15: https://developer.arm.com/documentation/ddi0360/e/control-coprocessor-cp15/register-descriptions/c15--performance-monitor-control-register--pmnc-
 *     - SCU: https://developer.arm.com/documentation/ddi0360/e/mpcore-private-memory-region/about-the-mpcore-private-memory-region/performance-monitor-event-registers
 */
typedef enum
{
	// Core events:
	PERFCOUNTEREVT_CORE_BASE = 0x0,
	PERFCOUNTEREVT_CORE_INST_CACHE_MISS = PERFCOUNTEREVT_CORE_BASE,
	PERFCOUNTEREVT_CORE_STALL_BY_LACK_OF_INST,
	PERFCOUNTEREVT_CORE_STALL_BY_DATA_HAZARD,
	PERFCOUNTEREVT_CORE_INST_MICRO_TLB_MISS,
	PERFCOUNTEREVT_CORE_DATA_MICRO_TLB_MISS,
	PERFCOUNTEREVT_CORE_BRANCH_INST,
	PERFCOUNTEREVT_CORE_BRANCH_NOT_PREDICTED,
	PERFCOUNTEREVT_CORE_BRANCH_MISS_PREDICTED,
	PERFCOUNTEREVT_CORE_INST_EXECUTED,
	PERFCOUNTEREVT_CORE_FOLDED_INST_EXECUTED,
	PERFCOUNTEREVT_CORE_DATA_CACHE_READ,
	PERFCOUNTEREVT_CORE_DATA_CACHE_READ_MISS,
	PERFCOUNTEREVT_CORE_DATA_CACHE_WRITE,
	PERFCOUNTEREVT_CORE_DATA_CACHE_WRITE_MISS,
	PERFCOUNTEREVT_CORE_DATA_CACHE_LINE_EVICTION,
	PERFCOUNTEREVT_CORE_PC_CHANGED,
	PERFCOUNTEREVT_CORE_MAIN_TLB_MISS,
	PERFCOUNTEREVT_CORE_EXTERNAL_REQUEST,
	PERFCOUNTEREVT_CORE_STALL_BY_LSU_FULL,
	PERFCOUNTEREVT_CORE_STORE_BUFFER_DRAIN,
	PERFCOUNTEREVT_CORE_MERGE_IN_STORE_BUFFER,
	PERFCOUNTEREVT_CORE_CYCLE_COUNT = PERFCOUNTEREVT_CORE_BASE + 0xFF,     ///< One cycle elapsed.
	PERFCOUNTEREVT_CORE_CYCLE_COUNT_64 = PERFCOUNTEREVT_CORE_BASE + 0xFFF, ///< 64 cycles elapsed.


	PERFCOUNTEREVT_SCU_BASE = 0x1000,
	PERFCOUNTEREVT_SCU_DISABLED = PERFCOUNTEREVT_SCU_BASE,
	PERFCOUNTEREVT_SCU_LINEFILL_MISS_FROM_CORE0,
	PERFCOUNTEREVT_SCU_LINEFILL_MISS_FROM_CORE1,
	PERFCOUNTEREVT_SCU_LINEFILL_MISS_FROM_CORE2,
	PERFCOUNTEREVT_SCU_LINEFILL_MISS_FROM_CORE3,
	PERFCOUNTEREVT_SCU_LINEFILL_HIT_FROM_CORE0,
	PERFCOUNTEREVT_SCU_LINEFILL_HIT_FROM_CORE1,
	PERFCOUNTEREVT_SCU_LINEFILL_HIT_FROM_CORE2,
	PERFCOUNTEREVT_SCU_LINEFILL_HIT_FROM_CORE3,
	PERFCOUNTEREVT_SCU_LINE_MISSING_FROM_CORE0,
	PERFCOUNTEREVT_SCU_LINE_MISSING_FROM_CORE1,
	PERFCOUNTEREVT_SCU_LINE_MISSING_FROM_CORE2,
	PERFCOUNTEREVT_SCU_LINE_MISSING_FROM_CORE3,
	PERFCOUNTEREVT_SCU_LINE_MIGRATION,
	PERFCOUNTEREVT_SCU_READ_BUSY_PORT0,
	PERFCOUNTEREVT_SCU_READ_BUSY_PORT1,
	PERFCOUNTEREVT_SCU_WRITE_BUSY_PORT0,
	PERFCOUNTEREVT_SCU_WRITE_BUSY_PORT1,
	PERFCOUNTEREVT_SCU_EXTERNAL_READ,
	PERFCOUNTEREVT_SCU_EXTERNAL_WRITE,
	PERFCOUNTEREVT_SCU_CYCLE_COUNT = PERFCOUNTEREVT_SCU_BASE + 0x1F,
} PerfCounterEvent;

/// Event relating to the attachment of a process.
typedef struct {
	u64 program_id;       ///< ID of the program.
	char process_name[8]; ///< Name of the process.
	u32 process_id;       ///< ID of the process.
	u32 other_flags;      ///< Always 0
} AttachProcessEvent;

/// Reasons for an exit process event.
typedef enum {
	EXITPROCESS_EVENT_EXIT                = 0, ///< Process exited either normally or due to an uncaught exception.
	EXITPROCESS_EVENT_TERMINATE           = 1, ///< Process has been terminated by @ref svcTerminateProcess.
	EXITPROCESS_EVENT_DEBUG_TERMINATE     = 2, ///< Process has been terminated by @ref svcTerminateDebugProcess.
} ExitProcessEventReason;

/// Event relating to the exiting of a process.
typedef struct {
	ExitProcessEventReason reason; ///< Reason for exiting. See @ref ExitProcessEventReason
} ExitProcessEvent;

/// Event relating to the attachment of a thread.
typedef struct {
	u32 creator_thread_id;    ///< ID of the creating thread.
	u32 thread_local_storage; ///< Thread local storage.
	u32 entry_point;          ///< Entry point of the thread.
} AttachThreadEvent;

/// Reasons for an exit thread event.
typedef enum {
	EXITTHREAD_EVENT_EXIT              = 0, ///< Thread exited.
	EXITTHREAD_EVENT_TERMINATE         = 1, ///< Thread terminated.
	EXITTHREAD_EVENT_EXIT_PROCESS      = 2, ///< Process exited either normally or due to an uncaught exception.
	EXITTHREAD_EVENT_TERMINATE_PROCESS = 3, ///< Process has been terminated by @ref svcTerminateProcess.
} ExitThreadEventReason;

/// Event relating to the exiting of a thread.
typedef struct {
	ExitThreadEventReason reason; ///< Reason for exiting. See @ref ExitThreadEventReason
} ExitThreadEvent;

/// Reasons for a user break.
typedef enum {
	USERBREAK_PANIC         = 0, ///< Panic.
	USERBREAK_ASSERT        = 1, ///< Assertion failed.
	USERBREAK_USER          = 2, ///< User related.
	USERBREAK_LOAD_RO       = 3, ///< Load RO.
	USERBREAK_UNLOAD_RO     = 4, ///< Unload RO.
} UserBreakType;

/// Reasons for an exception event.
typedef enum {
	EXCEVENT_UNDEFINED_INSTRUCTION = 0, ///< Undefined instruction.
	EXCEVENT_PREFETCH_ABORT        = 1, ///< Prefetch abort.
	EXCEVENT_DATA_ABORT            = 2, ///< Data abort (other than the below kind).
	EXCEVENT_UNALIGNED_DATA_ACCESS = 3, ///< Unaligned data access.
	EXCEVENT_ATTACH_BREAK          = 4, ///< Attached break.
	EXCEVENT_STOP_POINT            = 5, ///< Stop point reached.
	EXCEVENT_USER_BREAK            = 6, ///< User break occurred.
	EXCEVENT_DEBUGGER_BREAK        = 7, ///< Debugger break occurred.
	EXCEVENT_UNDEFINED_SYSCALL     = 8, ///< Undefined syscall.
} ExceptionEventType;

/// Event relating to fault exceptions (CPU exceptions other than stop points and undefined syscalls).
typedef struct {
	u32 fault_information; ///< FAR (for DATA ABORT / UNALIGNED DATA ACCESS), attempted syscall or 0
} FaultExceptionEvent;

/// Stop point types
typedef enum {
	STOPPOINT_SVC_FF        = 0, ///< See @ref SVC_STOP_POINT.
	STOPPOINT_BREAKPOINT    = 1, ///< Breakpoint.
	STOPPOINT_WATCHPOINT    = 2, ///< Watchpoint.
} StopPointType;

/// Event relating to stop points
typedef struct {
	StopPointType type;    ///< Stop point type, see @ref StopPointType.
	u32 fault_information; ///< FAR for Watchpoints, otherwise 0.
} StopPointExceptionEvent;

/// Event relating to @ref svcBreak
typedef struct {
	UserBreakType type;   ///< User break type, see @ref UserBreakType.
	u32 croInfo;          ///< For LOAD_RO and UNLOAD_RO.
	u32 croInfoSize;      ///< For LOAD_RO and UNLOAD_RO.
} UserBreakExceptionEvent;

/// Event relating to @ref svcBreakDebugProcess
typedef struct {
	s32 thread_ids[4]; ///< IDs of the attached process's threads that were running on each core at the time of the @ref svcBreakDebugProcess call, or -1 (only the first 2 values are meaningful on O3DS).
} DebuggerBreakExceptionEvent;

/// Event relating to exceptions.
typedef struct {
	ExceptionEventType type; ///< Type of event. See @ref ExceptionEventType.
	u32 address;             ///< Address of the exception.
	union {
		FaultExceptionEvent fault;                  ///< Fault exception event data.
		StopPointExceptionEvent stop_point;         ///< Stop point exception event data.
		UserBreakExceptionEvent user_break;         ///< User break exception event data.
		DebuggerBreakExceptionEvent debugger_break; ///< Debugger break exception event data
	};
} ExceptionEvent;

/// Event relating to the scheduler.
typedef struct {
	u64 clock_tick; ///< Clock tick that the event occurred.
} ScheduleInOutEvent;

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
	u32 mapped_addr;   ///< Mapped address.
	u32 mapped_size;   ///< Mapped size.
	MemPerm memperm;   ///< Memory permissions. See @ref MemPerm.
	MemState memstate; ///< Memory state. See @ref MemState.
} MapEvent;

/// Debug event type.
typedef enum {
	DBGEVENT_ATTACH_PROCESS = 0,  ///< Process attached event.
	DBGEVENT_ATTACH_THREAD  = 1,  ///< Thread attached event.
	DBGEVENT_EXIT_THREAD    = 2,  ///< Thread exit event.
	DBGEVENT_EXIT_PROCESS   = 3,  ///< Process exit event.
	DBGEVENT_EXCEPTION      = 4,  ///< Exception event.
	DBGEVENT_DLL_LOAD       = 5,  ///< DLL load event.
	DBGEVENT_DLL_UNLOAD     = 6,  ///< DLL unload event.
	DBGEVENT_SCHEDULE_IN    = 7,  ///< Schedule in event.
	DBGEVENT_SCHEDULE_OUT   = 8,  ///< Schedule out event.
	DBGEVENT_SYSCALL_IN     = 9,  ///< Syscall in event.
	DBGEVENT_SYSCALL_OUT    = 10, ///< Syscall out event.
	DBGEVENT_OUTPUT_STRING  = 11, ///< Output string event.
	DBGEVENT_MAP            = 12, ///< Map event.
} DebugEventType;

/// Information about a debug event.
typedef struct {
	DebugEventType type; ///< Type of event. See @ref DebugEventType
	u32 thread_id;       ///< ID of the thread.
	u32 flags;           ///< Flags. Bit0 means that @ref svcContinueDebugEvent needs to be called for this event (except for EXIT PROCESS events, where this flag is disregarded).
	u8 remnants[4];      ///< Always 0.
	union {
		AttachProcessEvent attach_process; ///< Process attachment event data.
		AttachThreadEvent attach_thread;   ///< Thread attachment event data.
		ExitThreadEvent exit_thread;       ///< Thread exit event data.
		ExitProcessEvent exit_process;     ///< Process exit event data.
		ExceptionEvent exception;          ///< Exception event data.
		/* DLL_LOAD and DLL_UNLOAD do not seem to possess any event data */
		ScheduleInOutEvent scheduler;      ///< Schedule in/out event data.
		SyscallInOutEvent syscall;         ///< Syscall in/out event data.
		OutputStringEvent output_string;   ///< Output string event data.
		MapEvent map;                      ///< Map event data.
	};
} DebugEventInfo;

/// Debug flags for an attached process, set by @ref svcContinueDebugEvent
typedef enum {
	DBG_INHIBIT_USER_CPU_EXCEPTION_HANDLERS   = BIT(0), ///< Inhibit user-defined CPU exception handlers (including watchpoints and breakpoints, regardless of any @ref svcKernelSetState call).
	DBG_SIGNAL_FAULT_EXCEPTION_EVENTS         = BIT(1), ///< Signal fault exception events. See @ref FaultExceptionEvent.
	DBG_SIGNAL_SCHEDULE_EVENTS                = BIT(2), ///< Signal schedule in/out events. See @ref ScheduleInOutEvent.
	DBG_SIGNAL_SYSCALL_EVENTS                 = BIT(3), ///< Signal syscall in/out events. See @ref SyscallInOutEvent.
	DBG_SIGNAL_MAP_EVENTS                     = BIT(4), ///< Signal map events. See @ref MapEvent.
} DebugFlags;

typedef struct {
	CpuRegisters cpu_registers; ///< CPU registers.
	FpuRegisters fpu_registers; ///< FPU registers.
} ThreadContext;

/// Control flags for @ref svcGetDebugThreadContext and @ref svcSetDebugThreadContext
typedef enum {
	THREADCONTEXT_CONTROL_CPU_GPRS  = BIT(0), ///< Control r0-r12.
	THREADCONTEXT_CONTROL_CPU_SPRS  = BIT(1), ///< Control sp, lr, pc, cpsr.
	THREADCONTEXT_CONTROL_FPU_GPRS  = BIT(2), ///< Control d0-d15 (or s0-s31).
	THREADCONTEXT_CONTROL_FPU_SPRS  = BIT(3), ///< Control fpscr, fpexc.

	THREADCONTEXT_CONTROL_CPU_REGS  = BIT(0) | BIT(1), ///< Control r0-r12, sp, lr, pc, cpsr.
	THREADCONTEXT_CONTROL_FPU_REGS  = BIT(2) | BIT(3), ///< Control d0-d15, fpscr, fpexc.

	THREADCONTEXT_CONTROL_ALL       = BIT(0) | BIT(1) | BIT(2) | BIT(3), ///< Control all of the above.
} ThreadContextControlFlags;

/// Thread parameter field for @ref svcGetDebugThreadParameter
typedef enum {
	DBGTHREAD_PARAMETER_PRIORITY            = 0, ///< Thread priority.
	DBGTHREAD_PARAMETER_SCHEDULING_MASK_LOW = 1, ///< Low scheduling mask.
	DBGTHREAD_PARAMETER_CPU_IDEAL           = 2, ///< Ideal processor.
	DBGTHREAD_PARAMETER_CPU_CREATOR         = 3, ///< Processor that created the threod.
} DebugThreadParameter;

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

///@name Device drivers
///@{

/// Writes the default DMA device config that the kernel uses when DMACFG_*_IS_DEVICE and DMACFG_*_USE_CFG are not set
static inline void dmaDeviceConfigInitDefault(DmaDeviceConfig *cfg)
{
	// Kernel uses this default instance if _IS_DEVICE and _USE_CFG are not set
	*cfg = (DmaDeviceConfig) {
		.deviceId = -1,
		.allowedAlignments = 8 | 4 | 2 | 1,
		.burstSize = 0x80,
		.transferSize = 0,
		.burstStride = 0x80,
		.transferStride = 0,
	};
}

/// Initializes a \ref DmaConfig instance with sane defaults for RAM<>RAM tranfers
static inline void dmaConfigInitDefault(DmaConfig *cfg)
{
	*cfg = (DmaConfig) {
		.channelId = -1,
		.endianSwapSize = 0,
		.flags = DMACFG_WAIT_AVAILABLE,
		._padding = 0,
		.srcCfg = {},
		.dstCfg = {},
	};
}

///@}

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
 * @brief Maps a block of process memory, starting from address 0x00100000.
 * @param process Handle of the process.
 * @param destAddress Address of the block of memory to map, in the current (destination) process.
 * @param size Size of the block of memory to map (truncated to a multiple of 0x1000 bytes).
 */
Result svcMapProcessMemory(Handle process, u32 destAddress, u32 size);

/**
 * @brief Unmaps a block of process memory, starting from address 0x00100000.
 * @param process Handle of the process.
 * @param destAddress Address of the block of memory to unmap, in the current (destination) process.
 * @param size Size of the block of memory to unmap (truncated to a multiple of 0x1000 bytes).
 */
Result svcUnmapProcessMemory(Handle process, u32 destAddress, u32 size);

/**
 * @brief Unmaps a block of shared memory
 * @param memblock Handle of the block
 * @param addr Address of the memory to unmap, page-aligned. So its alignment must be 0x1000.
 */
Result svcUnmapMemoryBlock(Handle memblock, u32 addr);

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
void __attribute__((noreturn)) svcExitProcess();

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
 * @brief Gets a list of the threads of a process.
 * @param[out] threadCount Pointer to output the thread count to.
 * @param[out] threadIds Pointer to output the thread IDs to.
 * @param threadIdMaxCount Maximum number of thread IDs.
 * @param process Process handle to list the threads of.
 */
Result svcGetThreadList(s32* threadCount, u32* threadIds, s32 threadIdMaxCount, Handle process);

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
 * @brief Gets a process's affinity mask.
 * @param[out] affinitymask Pointer to store the affinity masks.
 * @param process Handle of the process.
 * @param processorcount Number of processors.
 */
Result svcGetProcessAffinityMask(u8* affinitymask, Handle process, s32 processorcount);

/**
 * @brief Sets a process's affinity mask.
 * @param process Handle of the process.
 * @param affinitymask Pointer to retrieve the affinity masks from.
 * @param processorcount Number of processors.
 */
Result svcSetProcessAffinityMask(Handle process, const u8* affinitymask, s32 processorcount);

/**
 * Gets a process's ideal processor.
 * @param[out] processorid Pointer to store the ID of the process's ideal processor.
 * @param process Handle of the process.
 */
Result svcGetProcessIdealProcessor(s32 *processorid, Handle process);

/**
 * Sets a process's ideal processor.
 * @param process Handle of the process.
 * @param processorid ID of the process's ideal processor.
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
Result svcGetResourceLimitLimitValues(s64* values, Handle resourceLimit, ResourceLimitType* names, s32 nameCount);

/**
 * @brief Gets the values of a resource limit set.
 * @param[out] values Pointer to output the values to.
 * @param resourceLimit Resource limit set to use.
 * @param names Resource limit names to get the values of.
 * @param nameCount Number of resource limit names.
 */
Result svcGetResourceLimitCurrentValues(s64* values, Handle resourceLimit, ResourceLimitType* names, s32 nameCount);

/**
 * @brief Sets the resource limit set of a process.
 * @param process Process to set the resource limit set to.
 * @param resourceLimit Resource limit set handle.
 */
Result svcSetProcessResourceLimits(Handle process, Handle resourceLimit);

/**
 * @brief Creates a resource limit set.
 * @param[out] resourceLimit Pointer to output the resource limit set handle to.
 */
Result svcCreateResourceLimit(Handle* resourceLimit);

/**
 * @brief Sets the value limits of a resource limit set.
 * @param resourceLimit Resource limit set to use.
 * @param names Resource limit names to set the limits of.
 * @param values Value limits to set. The high 32 bits of RESLIMIT_COMMIT are used to
                 set APPMEMALLOC in configuration memory, otherwise those bits are unused.
 * @param nameCount Number of resource limit names.
 */
Result svcSetResourceLimitValues(Handle resourceLimit, const ResourceLimitType* names, const s64* values, s32 nameCount);

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
 * @param reset_type Type of reset the event uses (RESET_ONESHOT/RESET_STICKY).
 */
Result svcCreateEvent(Handle* event, ResetType reset_type);

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
Result svcWaitSynchronizationN(s32* out, const Handle* handles, s32 handles_num, bool wait_all, s64 nanoseconds);

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
 * @param timeout_ns Optional timeout in nanoseconds when using TIMEOUT actions, ignored otherwise. If not needed, use \ref svcArbitrateAddressNoTimeout instead.
 * @note Usage of this syscall entails an implicit Data Memory Barrier (dmb).
 * @warning Please use \ref syncArbitrateAddressWithTimeout instead.
 */
Result svcArbitrateAddress(Handle arbiter, u32 addr, ArbitrationType type, s32 value, s64 timeout_ns);

/**
 * @brief Same as \ref svcArbitrateAddress but with the timeout_ns parameter undefined.
 * @param arbiter Handle of the arbiter
 * @param addr A pointer to a s32 value.
 * @param type Type of action to be performed by the arbiter
 * @param value Number of threads to signal if using @ref ARBITRATION_SIGNAL, or the value used for comparison.
 * @note Usage of this syscall entails an implicit Data Memory Barrier (dmb).
 * @warning Please use \ref syncArbitrateAddress instead.
 */
Result svcArbitrateAddressNoTimeout(Handle arbiter, u32 addr, ArbitrationType type, s32 value);

/**
 * @brief Sends a synchronized request to a session handle.
 * @param session Handle of the session.
 */
Result svcSendSyncRequest(Handle session);

/**
 * @brief Connects to a port via a handle.
 * @param[out] clientSession Pointer to output the client session handle to.
 * @param clientPort Port client endpoint to connect to.
 */
Result svcCreateSessionToPort(Handle* clientSession, Handle clientPort);

/**
 * @brief Creates a linked pair of session endpoints.
 * @param[out] serverSession Pointer to output the created server endpoint handle to.
 * @param[out] clientSession Pointer to output the created client endpoint handle to.
 */
Result svcCreateSession(Handle* serverSession, Handle* clientSession);

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
Result svcReplyAndReceive(s32* index, const Handle* handles, s32 handleCount, Handle replyTarget);

///@}

///@name Time
///@{
/**
 * @brief Creates a timer.
 * @param[out] timer Pointer to output the handle of the created timer to.
 * @param reset_type Type of reset to perform on the timer.
 */
Result svcCreateTimer(Handle* timer, ResetType reset_type);

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
 * @brief Gets a handle info.
 * @param[out] out Pointer to output the handle info to.
 * @param handle Handle to get the info for.
 * @param param Parameter clarifying the handle info type.
 */
Result svcGetHandleInfo(s64* out, Handle handle, u32 param);

/**
 * @brief Gets the system info.
 * @param[out] out Pointer to output the system info to.
 * @param type Type of system info to retrieve.
 * @param param Parameter clarifying the system info type.
 */
Result svcGetSystemInfo(s64* out, u32 type, s32 param);

/**
 * @brief Sets the current kernel state.
 * @param type Type of state to set (the other parameters depend on it).
 */
Result svcKernelSetState(u32 type, ...);
///@}

///@name Device drivers
///@{

/**
 * @brief Binds an event or semaphore handle to an ARM11 interrupt.
 * @param interruptId Interrupt identfier (see https://www.3dbrew.org/wiki/ARM11_Interrupts).
 * @param eventOrSemaphore Event or semaphore handle to bind to the given interrupt.
 * @param priority Priority of the interrupt for the current process.
 * @param isManualClear Indicates whether the interrupt has to be manually cleared or not (= level-high active).
 */
Result svcBindInterrupt(u32 interruptId, Handle eventOrSemaphore, s32 priority, bool isManualClear);

/**
 * @brief Unbinds an event or semaphore handle from an ARM11 interrupt.
 * @param interruptId Interrupt identfier, see (see https://www.3dbrew.org/wiki/ARM11_Interrupts).
 * @param eventOrSemaphore Event or semaphore handle to unbind from the given interrupt.
 */
Result svcUnbindInterrupt(u32 interruptId, Handle eventOrSemaphore);

/**
 * @brief Invalidates a process's data cache.
 * @param process Handle of the process.
 * @param addr Address to invalidate.
 * @param size Size of the memory to invalidate.
 */
Result svcInvalidateProcessDataCache(Handle process, u32 addr, u32 size);

/**
 * @brief Cleans a process's data cache.
 * @param process Handle of the process.
 * @param addr Address to clean.
 * @param size Size of the memory to clean.
 */
Result svcStoreProcessDataCache(Handle process, u32 addr, u32 size);

/**
 * @brief Flushes (cleans and invalidates) a process's data cache.
 * @param process Handle of the process.
 * @param addr Address to flush.
 * @param size Size of the memory to flush.
 */
Result svcFlushProcessDataCache(Handle process, u32 addr, u32 size);

/**
 * @brief Begins an inter-process DMA transfer.
 * @param[out] dma Pointer to output the handle of the DMA channel object to.
 * @param dstProcess Destination process handle.
 * @param dstAddr Address in the destination process to write data to.
 * @param srcProcess Source process handle.
 * @param srcAddr Address in the source to read data from.
 * @param size Size of the data to transfer.
 * @param cfg Configuration structure.
 * @note The handle is signaled when the transfer finishes.
 */
Result svcStartInterProcessDma(Handle *dma, Handle dstProcess, u32 dstAddr, Handle srcProcess, u32 srcAddr, u32 size, const DmaConfig *cfg);

/**
 * @brief Stops an inter-process DMA transfer.
 * @param dma Handle of the DMA channel object.
 */
Result svcStopDma(Handle dma);

/**
 * @brief Gets the state of an inter-process DMA transfer.
 * @param[out] state Pointer to output the state of the DMA transfer to.
 * @param dma Handle of the DMA channel object.
 */
Result svcGetDmaState(DmaState *state, Handle dma);

/**
 * @brief Restarts a DMA transfer, using the same configuration as before.
 * @param[out] state Pointer to output the state of the DMA transfer to.
 * @param dma Handle of the DMA channel object.
 * @param dstAddr Address in the destination process to write data to.
 * @param srcAddr Address in the source to read data from.
 * @param size Size of the data to transfer.
 * @param flags Restart flags, \ref DMARST_UNLOCK and/or \ref DMARST_RESUME_DEVICE.
 * @note The first transfer has to be configured with \ref DMACFG_KEEP_LOCKED.
 */
Result svcRestartDma(Handle dma, u32 dstAddr, u32 srcAddr, u32 size, s8 flags);

/**
 * @brief Sets the GPU protection register to restrict the range of the GPU DMA. 11.3+ only.
 * @param useApplicationRestriction Whether to use the register value used for APPLICATION titles.
 */
Result svcSetGpuProt(bool useApplicationRestriction);

/**
 * @brief Enables or disables Wi-Fi. 11.4+ only.
 * @param enabled Whether to enable or disable Wi-Fi.
 */
Result svcSetWifiEnabled(bool enabled);

///@}

///@name Debugging
///@{
/**
 * @brief Breaks execution.
 * @param breakReason Reason for breaking.
 */
void svcBreak(UserBreakType breakReason);

/**
 * @brief Breaks execution (LOAD_RO and UNLOAD_RO).
 * @param breakReason Debug reason for breaking.
 * @param croInfo Library information.
 * @param croInfoSize Size of the above structure.
 */
void svcBreakRO(UserBreakType breakReason, const void* croInfo, u32 croInfoSize) __asm__("svcBreak");

/**
 * @brief Outputs a debug string.
 * @param str String to output.
 * @param length Length of the string to output, needs to be positive.
 */
Result svcOutputDebugString(const char* str, s32 length);


/**
 * @brief Controls performance monitoring on the CP15 interface and the SCU.
 * The meaning of the parameters depend on the operation.
 * @param[out] out Output.
 * @param op Operation, see details.
 * @param param1 First parameter.
 * @param param2 Second parameter.
 * @details The operations are the following:
 *     - \ref PERFCOUNTEROP_ENABLE (void) -> void, tries to enable and lock perfmon. functionality.
 *     - \ref PERFCOUNTEROP_DISABLE (void) -> void, disable and forcibly unlocks perfmon. functionality.
 *     - \ref PERFCOUNTEROP_GET_VALUE (\ref PerfCounterRegister reg) -> u64, gets the value of a particular counter register.
 *     - \ref PERFCOUNTEROP_SET_VALUE (\ref PerfCounterRegister reg, u64 value) -> void, sets the value of a particular counter register.
 *     - \ref PERFCOUNTEROP_GET_OVERFLOW_FLAGS (void) -> u32, gets the overflow flags of all CP15 and SCU registers.
 *         - Format is a bitfield of \ref PerfCounterRegister.
 *     - \ref PERFCOUNTEROP_RESET (u32 valueResetMask, u32 overflowFlagResetMask) -> void, resets the value and/or
 *     overflow flags of selected registers.
 *         - Format is two bitfields of \ref PerfCounterRegister.
 *     - \ref PERFCOUNTEROP_GET_EVENT (\ref PerfCounterRegister reg) -> \ref PerfCounterEvent, gets the event associated
 *     to a particular counter register.
 *     - \ref PERFCOUNTEROP_SET_EVENT (\ref PerfCounterRegister reg, \ref PerfCounterEvent) -> void, sets the event associated
 *     to a particular counter register.
 *     - \ref PERFCOUNTEROP_SET_VIRTUAL_COUNTER_ENABLED (bool enabled) -> void, (dis)allows the kernel to track counter overflows
 *     and to use 64-bit counter values.
 */
Result svcControlPerformanceCounter(u64 *out, PerfCounterOperation op, u32 param1, u64 param2);

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
 * @brief Continues the current debug event of a debugged process (not necessarily the same as @ref svcGetProcessDebugEvent).
 * @param debug Debug handle of the process.
 * @param flags Flags to continue with, see @ref DebugFlags.
 */
Result svcContinueDebugEvent(Handle debug, DebugFlags flags);

/**
 * @brief Fetches the saved registers of a thread, either inactive or awaiting @ref svcContinueDebugEvent, belonging to a debugged process.
 * @param[out] context Values of the registers to fetch, see @ref ThreadContext.
 * @param debug Debug handle of the parent process.
 * @param threadId ID of the thread to fetch the saved registers of.
 * @param controlFlags Which registers to fetch, see @ref ThreadContextControlFlags.
 */
Result svcGetDebugThreadContext(ThreadContext* context, Handle debug, u32 threadId, ThreadContextControlFlags controlFlags);

/**
 * @brief Updates the saved registers of a thread, either inactive or awaiting @ref svcContinueDebugEvent, belonging to a debugged process.
 * @param debug Debug handle of the parent process.
 * @param threadId ID of the thread to update the saved registers of.
 * @param context Values of the registers to update, see @ref ThreadContext.
 * @param controlFlags Which registers to update, see @ref ThreadContextControlFlags.
 */
Result svcSetDebugThreadContext(Handle debug, u32 threadId, ThreadContext* context, ThreadContextControlFlags controlFlags);

/**
 * @brief Queries memory information of a debugged process.
 * @param[out] info Pointer to output memory info to.
 * @param[out] out Pointer to output page info to.
 * @param debug Debug handle of the process to query memory from.
 * @param addr Virtual memory address to query.
 */
Result svcQueryDebugProcessMemory(MemInfo* info, PageInfo* out, Handle debug, u32 addr);

/**
 * @brief Reads from a debugged process's memory.
 * @param buffer Buffer to read data to.
 * @param debug Debug handle of the process.
 * @param addr Address to read from.
 * @param size Size of the memory to read.
 */
Result svcReadProcessMemory(void* buffer, Handle debug, u32 addr, u32 size);

/**
 * @brief Writes to a debugged process's memory.
 * @param debug Debug handle of the process.
 * @param buffer Buffer to write data from.
 * @param addr Address to write to.
 * @param size Size of the memory to write.
 */
Result svcWriteProcessMemory(Handle debug, const void* buffer, u32 addr, u32 size);

/**
 * @brief Sets an hardware breakpoint or watchpoint. This is an interface to the BRP/WRP registers, see http://infocenter.arm.com/help/topic/com.arm.doc.ddi0360f/CEGEBGFC.html .
 * @param registerId range 0..5 = breakpoints (BRP0-5), 0x100..0x101 = watchpoints (WRP0-1). The previous stop point for the register is disabled.
 * @param control Value of the control regiser.
 * @param value Value of the value register: either and address (if bit21 of control is clear) or the debug handle of a process to fetch the context ID of.
 */
Result svcSetHardwareBreakPoint(s32 registerId, u32 control, u32 value);

/**
 * @brief Gets a debugged thread's parameter.
 * @param[out] unused Unused.
 * @param[out] out Output value.
 * @param debug Debug handle of the process.
 * @param threadId ID of the thread
 * @param parameter Parameter to fetch, see @ref DebugThreadParameter.
 */
Result svcGetDebugThreadParam(s64* unused, u32* out, Handle debug, u32 threadId, DebugThreadParameter parameter);

///@}

/**
 * @brief Executes a function in supervisor mode.
 * @param callback Function to execute.
 */
Result svcBackdoor(s32 (*callback)(void));

/// Stop point, does nothing if the process is not attached (as opposed to 'bkpt' instructions)
#define SVC_STOP_POINT __asm__ volatile("svc 0xFF");
