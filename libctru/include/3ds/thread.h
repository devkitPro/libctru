/**
 * @file thread.h
 * @brief Provides functions to use threads.
 */
#pragma once
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/synchronization.h>
#include <3ds/svc.h>

/// libctru thread handle type
typedef struct Thread_tag* Thread;

/**
 * @brief Creates a new libctru thread.
 * @param entrypoint The function that will be called first upon thread creation
 * @param arg The argument passed to @p entrypoint
 * @param stack_size The size of the stack that will be allocated for the thread (will be rounded to a multiple of 8 bytes)
 * @param prio Low values gives the thread higher priority.
 *             For userland apps, this has to be within the range [0x18;0x3F].
 *             The main thread usually has a priority of 0x30, but not always. Use svcGetThreadPriority() if you need
 *             to create a thread with a priority that is explicitly greater or smaller than that of the main thread.
 * @param affinity The ID of the processor the thread should be ran on. Processor IDs are labeled starting from 0.
 *                 On Old3DS it must be <2, and on New3DS it must be <4.
 *                 Pass -1 to execute the thread on all CPUs and -2 to execute the thread on the default CPU (read from the Exheader).
 * @param detached When set to true, the thread is automatically freed when it finishes.
 * @return The libctru thread handle on success, NULL on failure.
 *
 * - Processor #0 is the application core. It is always possible to create a thread on this core.
 * - Processor #1 is the system core. If APT_SetAppCpuTimeLimit is used, it is possible to create a single thread on this core.
 * - Processor #2 is New3DS exclusive. Normal applications can create threads on this core if the exheader kernel flags bitmask has 0x2000 set.
 * - Processor #3 is New3DS exclusive. Normal applications cannot create threads on this core.
 * - Processes in the BASE memory region can always create threads on processors #2 and #3.
 *
 * @note Default exit code of a thread is 0.
 * @warning @ref svcExitThread should never be called from the thread, use @ref threadExit instead.
 */
Thread threadCreate(ThreadFunc entrypoint, void* arg, size_t stack_size, int prio, int affinity, bool detached);

/**
 * @brief Retrieves the OS thread handle of a libctru thread.
 * @param thread libctru thread handle
 * @return OS thread handle
 */
Handle threadGetHandle(Thread thread);

/**
 * @brief Retrieves the exit code of a finished libctru thread.
 * @param thread libctru thread handle
 * @return Exit code
 */
int threadGetExitCode(Thread thread);

/**
 * @brief Frees a finished libctru thread.
 * @param thread libctru thread handle
 */
void threadFree(Thread thread);

/**
 * @brief Waits for a libctru thread to finish (or returns immediately if it is already finished).
 * @param thread libctru thread handle
 * @param timeout_ns Timeout in nanoseconds. Pass U64_MAX if a timeout isn't desired
 */
Result threadJoin(Thread thread, u64 timeout_ns);

/**
 * @brief Retrieves the libctru thread handle of the current thread.
 * @return libctru thread handle of the current thread, or NULL for the main thread
 */
Thread threadGetCurrent(void);

/**
 * @brief Exits the current libctru thread with an exit code (not usable from the main thread).
 * @param rc Exit code
 */
void threadExit(int rc) __attribute__((noreturn));
