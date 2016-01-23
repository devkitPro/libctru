.arm
.align 4

.macro SVC_BEGIN name
	.section .text.\name, "ax", %progbits
	.global \name
	.type \name, %function
	.align 2
\name:
.endm

SVC_BEGIN svcControlMemory
	push {r0, r4}
	ldr  r0, [sp, #0x8]
	ldr  r4, [sp, #0x8+0x4]
	svc  0x01
	ldr  r2, [sp], #4
	str  r1, [r2]
	ldr  r4, [sp], #4
	bx   lr

SVC_BEGIN svcQueryMemory
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

SVC_BEGIN svcExitProcess
	svc 0x03
	bx  lr

SVC_BEGIN svcCreateThread
	push {r0, r4}
	ldr  r0, [sp, #0x8]
	ldr  r4, [sp, #0x8+0x4]
	svc  0x08
	ldr  r2, [sp], #4
	str  r1, [r2]
	ldr  r4, [sp], #4
	bx   lr

SVC_BEGIN svcExitThread
	svc 0x09
	bx  lr

SVC_BEGIN svcSleepThread
	svc 0x0A
	bx  lr

SVC_BEGIN svcGetThreadPriority
	str r0, [sp, #-0x4]!
	svc 0x0B
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr
	
SVC_BEGIN svcSetThreadPriority
	svc 0x0C
	bx  lr
	
SVC_BEGIN svcGetThreadAffinityMask
	svc 0x0D
	bx  lr

SVC_BEGIN svcSetThreadAffinityMask
	svc 0x0E
	bx  lr
	
SVC_BEGIN svcGetThreadIdealProcessor
	str r0, [sp, #-0x4]!
	svc 0x0F
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr
	
SVC_BEGIN svcSetThreadIdealProcessor
	svc 0x10
	bx  lr

SVC_BEGIN svcGetProcessorID
	svc 0x11
	bx  lr

SVC_BEGIN svcCreateMutex
	str r0, [sp, #-4]!
	svc 0x13
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

SVC_BEGIN svcReleaseMutex
	svc 0x14
	bx  lr

SVC_BEGIN svcCreateSemaphore
	push {r0}
	svc 0x15
	pop {r3}
	str r1, [r3]
	bx  lr

SVC_BEGIN svcReleaseSemaphore
	push {r0}
	svc  0x16
	pop  {r3}
	str  r1, [r3]
	bx   lr

SVC_BEGIN svcCreateEvent
	str r0, [sp, #-4]!
	svc 0x17
	ldr r2, [sp], #4
	str r1, [r2]
	bx  lr

SVC_BEGIN svcSignalEvent
	svc 0x18
	bx  lr

SVC_BEGIN svcClearEvent
	svc 0x19
	bx  lr

SVC_BEGIN svcCreateTimer
	str r0, [sp, #-4]!
	svc 0x1A
	ldr r2, [sp], #4
	str r1, [r2]
	bx  lr

SVC_BEGIN svcSetTimer
	str r4, [sp, #-4]!
	ldr r1, [sp, #4]
	ldr r4, [sp, #8]
	svc 0x1B
	ldr r4, [sp], #4
	bx  lr

SVC_BEGIN svcCancelTimer
	svc 0x1C
	bx  lr

SVC_BEGIN svcClearTimer
	svc 0x1D
	bx  lr

SVC_BEGIN svcCreateMemoryBlock
	str r0, [sp, #-4]!
	ldr r0, [sp, #4]
	svc 0x1E
	ldr r2, [sp], #4
	str r1, [r2]
	bx  lr

SVC_BEGIN svcMapMemoryBlock
	svc 0x1F
	bx  lr

SVC_BEGIN svcUnmapMemoryBlock
	svc 0x20
	bx  lr

SVC_BEGIN svcCreateAddressArbiter
	push {r0}
	svc 0x21
	pop {r2}
	str r1, [r2]
	bx  lr

SVC_BEGIN svcArbitrateAddress
	push {r4, r5}
	ldr r4, [sp, #8]
	ldr r5, [sp, #12]
	svc 0x22
	pop {r4, r5}
	bx  lr

SVC_BEGIN svcCloseHandle
	svc 0x23
	bx  lr

SVC_BEGIN svcWaitSynchronization
	svc 0x24
	bx  lr

SVC_BEGIN svcWaitSynchronizationN
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

SVC_BEGIN svcDuplicateHandle
	str r0, [sp, #-0x4]!
	svc 0x27
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

SVC_BEGIN svcGetSystemTick
	svc 0x28
	bx  lr

SVC_BEGIN svcGetSystemInfo
	str r0, [sp, #-0x4]!
	svc 0x2A
	ldr r3, [sp], #4
	str r1, [r3]
	str r2, [r3,#4]
	bx  lr

SVC_BEGIN svcGetProcessInfo
	str r0, [sp, #-0x4]!
	svc 0x2B
	ldr r3, [sp], #4
	str r1, [r3]
	str r2, [r3,#4]
	bx  lr

SVC_BEGIN svcGetThreadInfo
	str r0, [sp, #-0x4]!
	svc 0x2C
	ldr r3, [sp], #4
	str r1, [r3]
	str r2, [r3,#4]
	bx  lr

SVC_BEGIN svcConnectToPort
	str r0, [sp, #-0x4]!
	svc 0x2D
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

SVC_BEGIN svcSendSyncRequest
	svc 0x32
	bx  lr

SVC_BEGIN svcOpenProcess
	push {r0}
	svc 0x33
	pop {r2}
	str r1, [r2]
	bx  lr

SVC_BEGIN svcOpenThread
	push {r0}
	svc 0x34
	pop {r2}
	str r1, [r2]
	bx  lr
	
SVC_BEGIN svcGetProcessId
	str r0, [sp, #-0x4]!
	svc 0x35
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

SVC_BEGIN svcGetProcessIdOfThread
	str r0, [sp, #-0x4]!
	svc 0x36
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr
	
SVC_BEGIN svcGetThreadId
	str r0, [sp, #-0x4]!
	svc 0x37
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

SVC_BEGIN svcBreak
	svc 0x3C
	bx  lr

SVC_BEGIN svcOutputDebugString
	svc 0x3D
	bx  lr

SVC_BEGIN svcCreatePort
	push {r0, r1}
	svc 0x47
	ldr r3, [sp, #0]
	str r1, [r3]
	ldr r3, [sp, #4]
	str r2, [r3]
	add sp, sp, #8
	bx  lr

SVC_BEGIN svcAcceptSession
	str r0, [sp, #-4]!
	svc 0x4A
	ldr r2, [sp]
	str r1, [r2]
	add sp, sp, #4
	bx  lr

SVC_BEGIN svcReplyAndReceive
	str r0, [sp, #-4]!
	svc 0x4F
	ldr r2, [sp]
	str r1, [r2]
	add sp, sp, #4
	bx  lr

SVC_BEGIN svcInvalidateProcessDataCache
	svc 0x52
	bx  lr

SVC_BEGIN svcFlushProcessDataCache
	svc 0x54
	bx  lr

SVC_BEGIN svcStartInterProcessDma
	stmfd sp!, {r0, r4, r5}
	ldr r0, [sp, #0xC]
	ldr r4, [sp, #0x10]
	ldr r5, [sp, #0x14]
	svc 0x55
	ldmfd sp!, {r2, r4, r5}
	str r1, [r2]
	bx  lr

SVC_BEGIN svcStopDma
	svc 0x56
	bx  lr

SVC_BEGIN svcGetDmaState
	str r0, [sp, #-4]!
	svc 0x57
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr

SVC_BEGIN svcDebugActiveProcess
	push {r0}
	svc 0x60
	pop {r2}
	str r1, [r2]
	bx  lr

SVC_BEGIN svcBreakDebugProcess
	svc 0x61
	bx  lr

SVC_BEGIN svcTerminateDebugProcess
	svc 0x62
	bx  lr

SVC_BEGIN svcGetProcessDebugEvent
	svc 0x63
	bx  lr
	
SVC_BEGIN svcContinueDebugEvent
	svc 0x64
	bx  lr

SVC_BEGIN svcGetProcessList
	push {r0, r1}
	svc 0x65
	ldr r3, [sp, #0]
	str r1, [r3]
	ldr r3, [sp, #4]
	str r2, [r3]
	add sp, sp, #8
	bx  lr

SVC_BEGIN svcReadProcessMemory
	svc 0x6A
	bx  lr

SVC_BEGIN svcWriteProcessMemory
	svc 0x6B
	bx  lr

SVC_BEGIN svcControlProcessMemory
	push {r4-r5}
	ldr r4, [sp, #0x8]
	ldr r5, [sp, #0xC]
	svc 0x70
	pop {r4-r5}
	bx  lr

SVC_BEGIN svcMapProcessMemory
	svc 0x71
	bx  lr

SVC_BEGIN svcUnmapProcessMemory
	svc 0x72
	bx  lr

SVC_BEGIN svcCreateCodeSet
	str r0, [sp, #-0x4]!
	ldr r0, [sp, #4]
	svc 0x73
	ldr r2, [sp, #0]
	str r1, [r2]
	add sp, sp, #4
	bx  lr

SVC_BEGIN svcCreateProcess
	str r0, [sp, #-0x4]!
	svc 0x75
	ldr r2, [sp, #0]
	str r1, [r2]
	add sp, sp, #4
	bx  lr

SVC_BEGIN svcTerminateProcess
	svc 0x76
	bx  lr

SVC_BEGIN svcBackdoor
	svc 0x7B
	bx  lr

SVC_BEGIN svcKernelSetState
	svc 0x7C
	bx  lr

SVC_BEGIN svcQueryProcessMemory
	push {r0, r1, r4-r6}
	svc 0x7D
	ldr r6, [sp]
	stm r6, {r1-r4}
	ldr r6, [sp, #4]
	str r5, [r6]
	add sp, sp, #8
	pop {r4-r6}
	bx  lr
