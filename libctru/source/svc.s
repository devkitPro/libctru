.arm
.align 4

.global svcControlMemory
.type svcControlMemory, %function
svcControlMemory:
	push {r0, r4}
	ldr  r0, [sp, #0x8]
	ldr  r4, [sp, #0x8+0x4]
	svc  0x01
	ldr  r2, [sp], #4
	str  r1, [r2]
	ldr  r4, [sp], #4
	bx   lr

.global svcQueryMemory
.type svcQueryMemory, %function
svcQueryMemory:
	push {r0, r1, r4-r6}
	svc  0x02
	ldr  r6, [sp]
	str  r1, [r6]
	str  r2, [r6, #4]
	str  r3, [r6, #8]
	str  r4, [r6, #0xc]
	ldr  r6, [sp, #4]
	str  r5, [r6]
	add  sp, sp, #8
	pop  {r4-r6}
	bx   lr

.global svcExitProcess
.type svcExitProcess, %function
svcExitProcess:
	svc 0x03
	bx  lr

.global svcCreateThread
.type svcCreateThread, %function
svcCreateThread:
	push {r0, r4}
	ldr  r0, [sp, #0x8]
	ldr  r4, [sp, #0x8+0x4]
	svc  0x08
	ldr  r2, [sp], #4
	str  r1, [r2]
	ldr  r4, [sp], #4
	bx   lr

.global svcExitThread
.type svcExitThread, %function
svcExitThread:
	svc 0x09
	bx  lr

.global svcSleepThread
.type svcSleepThread, %function
svcSleepThread:
	svc 0x0A
	bx  lr

.global svcGetThreadPriority
.type svcGetThreadPriority, %function
svcGetThreadPriority:
	str r0, [sp, #-0x4]!
	svc 0x0B
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr
	
.global svcSetThreadPriority
.type svcSetThreadPriority, %function
svcSetThreadPriority:
	svc 0x0C
	bx  lr
	
.global svcGetThreadAffinityMask
.type svcGetThreadAffinityMask, %function
svcGetThreadAffinityMask:
	svc 0x0D
	bx  lr
	
.global svcSetThreadAffinityMask
.type svcSetThreadAffinityMask, %function
svcSetThreadAffinityMask:
	svc 0x0E
	bx  lr
	
.global svcGetThreadIdealProcessor
.type svcGetThreadIdealProcessor, %function
svcGetThreadIdealProcessor:
	str r0, [sp, #-0x4]!
	svc 0x0F
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr
	
.global svcSetThreadIdealProcessor
.type svcSetThreadIdealProcessor, %function
svcSetThreadIdealProcessor:
	svc 0x10
	bx  lr

.global svcGetProcessorID
.type svcGetProcessorID, %function
svcGetProcessorID:
	svc 0x11
	bx  lr

.global svcCreateMutex
.type svcCreateMutex, %function
svcCreateMutex:
	str r0, [sp, #-4]!
	svc 0x13
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

.global svcReleaseMutex
.type svcReleaseMutex, %function
svcReleaseMutex:
	svc 0x14
	bx  lr

.global svcCreateSemaphore
.type svcCreateSemaphore, %function
svcCreateSemaphore:
	push {r0}
	svc 0x15
	pop {r3}
	str r1, [r3]
	bx  lr

.global svcReleaseSemaphore
.type svcReleaseSemaphore, %function
svcReleaseSemaphore:
	push {r0}
	svc  0x16
	pop  {r3}
	str  r1, [r3]
	bx   lr

.global svcCreateEvent
.type svcCreateEvent, %function
svcCreateEvent:
	str r0, [sp, #-4]!
	svc 0x17
	ldr r2, [sp], #4
	str r1, [r2]
	bx  lr

.global svcSignalEvent
.type svcSignalEvent, %function
svcSignalEvent:
	svc 0x18
	bx  lr

.global svcClearEvent
.type svcClearEvent, %function
svcClearEvent:
	svc 0x19
	bx  lr

.global svcCreateTimer
.type svcCreateTimer, %function
svcCreateTimer:
	str r0, [sp, #-4]!
	svc 0x1A
	ldr r2, [sp], #4
	str r1, [r2]
	bx  lr

.global svcSetTimer
.type svcSetTimer, %function
svcSetTimer:
	svc 0x1B
	bx  lr

.global svcCancelTimer
.type svcCancelTimer, %function
svcCancelTimer:
	svc 0x1C
	bx  lr

.global svcClearTimer
.type svcClearTimer, %function
svcClearTimer:
	svc 0x1D
	bx  lr

.global svcCreateMemoryBlock
.type svcCreateMemoryBlock, %function
svcCreateMemoryBlock:
	str r0, [sp, #-4]!
	ldr r0, [sp, #4]
	svc 0x1E
	ldr r2, [sp], #4
	str r1, [r2]
	bx  lr

.global svcMapMemoryBlock
.type svcMapMemoryBlock, %function
svcMapMemoryBlock:
	svc 0x1F
	bx  lr

.global svcUnmapMemoryBlock
.type svcUnmapMemoryBlock, %function
svcUnmapMemoryBlock:
	svc 0x20
	bx  lr

.global svcCreateAddressArbiter
.type svcCreateAddressArbiter, %function
svcCreateAddressArbiter:
	push {r0}
	svc 0x21
	pop {r2}
	str r1, [r2]
	bx  lr

.global svcArbitrateAddress
.type svcArbitrateAddress, %function
svcArbitrateAddress:
	push {r4, r5}
	add sp, #8
	ldr r5, [sp]
	ldr r4, [sp, #4]
	sub sp, #8
	svc 0x22
	pop {r4, r5}
	bx  lr

.global svcCloseHandle
.type svcCloseHandle, %function
svcCloseHandle:
	svc 0x23
	bx  lr

.global svcWaitSynchronization
.type svcWaitSynchronization, %function
svcWaitSynchronization:
	svc 0x24
	bx  lr

.global svcWaitSynchronizationN
.type svcWaitSynchronizationN, %function
svcWaitSynchronizationN:
	str r5, [sp, #-4]!
	str r4, [sp, #-4]!
	mov r5, r0
	ldr r0, [sp, #0x8]
	ldr r4, [sp, #0x8+0x4]
	svc 0x25
	str r1, [r5]
	ldr r4, [sp], #4
	ldr r5, [sp], #4
	bx  lr

.global svcDuplicateHandle
.type svcDuplicateHandle, %function
svcDuplicateHandle:
	str r0, [sp, #-0x4]!
	svc 0x27
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

.global svcGetSystemTick
.type svcGetSystemTick, %function
svcGetSystemTick:
	svc 0x28
	bx  lr

.global svcGetSystemInfo
.type svcGetSystemInfo, %function
svcGetSystemInfo:
	str r0, [sp, #-0x4]!
	svc 0x2A
	ldr r3, [sp], #4
	str r1, [r3]
	str r2, [r3,#4]
	bx  lr

.global svcGetProcessInfo
.type svcGetProcessInfo, %function
svcGetProcessInfo:
	str r0, [sp, #-0x4]!
	svc 0x2B
	ldr r3, [sp], #4
	str r1, [r3]
	str r2, [r3,#4]
	bx  lr

.global svcGetThreadInfo
.type svcGetThreadInfo, %function
svcGetThreadInfo:
	str r0, [sp, #-0x4]!
	svc 0x2C
	ldr r3, [sp], #4
	str r1, [r3]
	str r2, [r3,#4]
	bx  lr

.global svcConnectToPort
.type svcConnectToPort, %function
svcConnectToPort:
	str r0, [sp, #-0x4]!
	svc 0x2D
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

.global svcSendSyncRequest
.type svcSendSyncRequest, %function
svcSendSyncRequest:
	svc 0x32
	bx  lr

.global svcOpenProcess
.type svcOpenProcess, %function
svcOpenProcess:
	push {r0}
	svc 0x33
	pop {r2}
	str r1, [r2]
	bx  lr

.global svcOpenThread
.type svcOpenThread, %function
svcOpenThread:
	push {r0}
	svc 0x34
	pop {r2}
	str r1, [r2]
	bx  lr
	
.global svcGetProcessId
.type svcGetProcessId, %function
svcGetProcessId:
	str r0, [sp, #-0x4]!
	svc 0x35
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

.global svcGetProcessIdOfThread
.type svcGetProcessIdOfThread, %function
svcGetProcessIdOfThread:
	str r0, [sp, #-0x4]!
	svc 0x36
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr
	
.global svcGetThreadId
.type svcGetThreadId, %function
svcGetThreadId:
	str r0, [sp, #-0x4]!
	svc 0x37
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

.global svcBreak
.type svcBreak, %function
svcBreak:
	svc 0x3C
	bx  lr

.global svcOutputDebugString
.type svcOutputDebugString, %function
svcOutputDebugString:
	svc 0x3D
	bx  lr

.global svcCreatePort
.type svcCreatePort, %function
svcCreatePort:
	push {r0, r1}
	svc 0x47
	ldr r3, [sp, #0]
	str r1, [r3]
	ldr r3, [sp, #4]
	str r2, [r3]
	add sp, sp, #8
	bx  lr

.global svcAcceptSession
.type svcAcceptSession, %function
svcAcceptSession:
	str r0, [sp, #-4]!
	svc 0x4A
	ldr r2, [sp]
	str r1, [r2]
	add sp, sp, #4
	bx  lr

.global svcReplyAndReceive
.type svcReplyAndReceive, %function
svcReplyAndReceive:
	str r0, [sp, #-4]!
	svc 0x4F
	ldr r2, [sp]
	str r1, [r2]
	add sp, sp, #4
	bx  lr

.global svcInvalidateProcessDataCache
.type svcInvalidateProcessDataCache, %function
svcInvalidateProcessDataCache:
	svc 0x52
	bx  lr

.global svcFlushProcessDataCache
.type svcFlushProcessDataCache, %function
svcFlushProcessDataCache:
	svc 0x54
	bx  lr

.global svcStartInterProcessDma
.type svcStartInterProcessDma, %function
svcStartInterProcessDma:
	stmfd sp!, {r0, r4, r5}
	ldr r0, [sp, #0xC]
	ldr r4, [sp, #0x10]
	ldr r5, [sp, #0x14]
	svc 0x55
	ldmfd sp!, {r2, r4, r5}
	str r1, [r2]
	bx  lr

.global svcStopDma
.type svcStopDma, %function
svcStopDma:
	svc 0x56
	bx  lr

.global svcGetDmaState
.type svcGetDmaState, %function
svcGetDmaState:
	str r0, [sp, #-4]!
	svc 0x57
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

.global svcDebugActiveProcess
.type svcDebugActiveProcess, %function
svcDebugActiveProcess:
	push {r0}
	svc 0x60
	pop {r2}
	str r1, [r2]
	bx  lr

.global svcBreakDebugProcess
.type svcBreakDebugProcess, %function
svcBreakDebugProcess:
	svc 0x61
	bx  lr

.global svcTerminateDebugProcess
.type svcTerminateDebugProcess, %function
svcTerminateDebugProcess:
	svc 0x62
	bx  lr

.global svcGetProcessDebugEvent
.type svcGetProcessDebugEvent, %function
svcGetProcessDebugEvent:
	svc 0x63
	bx  lr
	
.global svcContinueDebugEvent
.type svcContinueDebugEvent, %function
svcContinueDebugEvent:
	svc 0x64
	bx  lr

.global svcGetProcessList
.type svcGetProcessList, %function
svcGetProcessList:
	push {r0, r1}
	svc 0x65
	ldr r3, [sp, #0]
	str r1, [r3]
	ldr r3, [sp, #4]
	str r2, [r3]
	add sp, sp, #8
	bx  lr

.global svcReadProcessMemory
.type svcReadProcessMemory, %function
svcReadProcessMemory:
	svc 0x6A
	bx  lr

.global svcWriteProcessMemory
.type svcWriteProcessMemory, %function
svcWriteProcessMemory:
	svc 0x6B
	bx  lr

.global svcControlProcessMemory
.type svcControlProcessMemory, %function
svcControlProcessMemory:
	push {r4-r5}
	ldr r4, [sp, #0x8]
	ldr r5, [sp, #0xC]
	svc 0x70
	pop {r4-r5}
	bx  lr

.global svcMapProcessMemory
.type svcMapProcessMemory, %function
svcMapProcessMemory:
	svc 0x71
	bx  lr

.global svcUnmapProcessMemory
.type svcUnmapProcessMemory, %function
svcUnmapProcessMemory:
	svc 0x72
	bx  lr

.global svcTerminateProcess
.type svcTerminateProcess, %function
svcTerminateProcess:
	svc 0x76
	bx  lr

.global svcBackdoor
.type svcBackdoor, %function
svcBackdoor:
	svc 0x7B
	bx  lr

.global svcKernelSetState
.type svcKernelSetState, %function
svcKernelSetState:
	svc 0x7C
	bx  lr

.global svcQueryProcessMemory
.type svcQueryProcessMemory, %function
svcQueryProcessMemory:
	push {r0, r1, r4-r6}
	svc 0x7D
	ldr r6, [sp]
	stm r6, {r1-r4}
	ldr r6, [sp, #4]
	str r5, [r6]
	add sp, sp, #8
	pop {r4-r6}
	bx  lr
