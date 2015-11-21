/**
 * @file thread.h
 * @brief Provides functions to use threads.
 */
#pragma once
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/synchronization.h>
#include <3ds/svc.h>

typedef struct Thread_tag* Thread;

Thread threadCreate(ThreadFunc entrypoint, void* arg, size_t stack_size, int prio, int affinity, bool detached);
Handle threadGetHandle(Thread thread);
int threadGetExitCode(Thread thread);
void threadFree(Thread thread);
Result threadJoin(Thread thread, u64 timeout_ns);

Thread threadGetCurrent(void);
void threadExit(int rc) __attribute__((noreturn));
