.arm

.align 4

.global getThreadCommandBuffer
.type getThreadCommandBuffer, %function
getThreadCommandBuffer:
	mrc p15, 0, r0, c13, c0, 3
	add r0, #0x80
	bx lr


.global svc_controlMemory
.type svc_controlMemory, %function
svc_controlMemory:
	stmfd sp!, {r0, r4}
	ldr r0, [sp, #0x8]
	ldr r4, [sp, #0x8+0x4]
	svc 0x01
	ldr r2, [sp], #4
	str r1, [r2]
	ldr r4, [sp], #4
	bx lr

.global svc_exitProcess
.type svc_exitProcess, %function
svc_exitProcess:
	svc 0x03
	bx lr

.global svc_createThread
.type svc_createThread, %function
svc_createThread:
	stmfd sp!, {r0, r4}
	ldr r0, [sp, #0x8]
	ldr r4, [sp, #0x8+0x4]
	svc 0x08
	ldr r2, [sp], #4
	str r1, [r2]
	ldr r4, [sp], #4
	bx lr

.global svc_exitThread
.type svc_exitThread, %function
svc_exitThread:
	svc 0x09
	bx lr

.global svc_sleepThread
.type svc_sleepThread, %function
svc_sleepThread:
	svc 0x0A
	bx lr

.global svc_createMutex
.type svc_createMutex, %function
svc_createMutex:
	str r0, [sp, #-4]!
	svc 0x13
	ldr r3, [sp], #4
	str r1, [r3]
	bx lr

.global svc_releaseMutex
.type svc_releaseMutex, %function
svc_releaseMutex:
	svc 0x14
	bx lr

.global svc_createEvent
.type svc_createEvent, %function
svc_createEvent:
	str r0, [sp,#-4]!
	svc 0x17
	ldr r2, [sp], #4
	str r1, [r2]
	bx lr

.global svc_signalEvent
.type svc_signalEvent, %function
svc_signalEvent:
	svc 0x18
	bx lr

.global svc_clearEvent
.type svc_clearEvent, %function
svc_clearEvent:
	svc 0x19
	bx lr

.global svc_createMemoryBlock
.type svc_createMemoryBlock, %function
svc_createMemoryBlock:
	str r0, [sp, #-4]!
	ldr r0, [sp, #4]
	svc 0x1E
	ldr r2, [sp], #4
	str r1, [r2]
	bx lr

.global svc_mapMemoryBlock
.type svc_mapMemoryBlock, %function
svc_mapMemoryBlock:
	svc 0x1F
	bx lr

.global svc_unmapMemoryBlock
.type svc_unmapMemoryBlock, %function
svc_unmapMemoryBlock:
	svc 0x20
	bx lr

.global svc_closeHandle
.type svc_closeHandle, %function
svc_closeHandle:
	svc 0x23
	bx lr

.global svc_waitSynchronization1
.type svc_waitSynchronization1, %function
svc_waitSynchronization1:
	svc 0x24
	bx lr

.global svc_waitSynchronizationN
.type svc_waitSynchronizationN, %function
svc_waitSynchronizationN:
	str r5, [sp, #-4]!
	mov r5, r0
	ldr r0, [sp, #0x4]
	ldr r4, [sp, #0x4+0x4]
	svc 0x25
	str r1, [r5]
	ldr r5, [sp], #4
	bx lr

.global svc_getSystemTick
.type svc_getSystemTick, %function
svc_getSystemTick:
	svc 0x28
	bx lr

.global svc_getSystemInfo
.type svc_getSystemInfo, %function
svc_getSystemInfo:
	stmfd sp!, {r0, r4}
	svc 0x2A
	ldr r4, [sp], #4
	str r1, [r4]
	str r2, [r4, #4]
	# str r3, [r4, #8] # ?
	ldr r4, [sp], #4
	bx lr

.global svc_connectToPort
.type svc_connectToPort, %function
svc_connectToPort:
	str r0, [sp,#-0x4]!
	svc 0x2D
	ldr r3, [sp], #4
	str r1, [r3]
	bx lr

.global svc_sendSyncRequest
.type svc_sendSyncRequest, %function
svc_sendSyncRequest:
	svc 0x32
	bx lr

.global svc_getProcessId
.type svc_getProcessId, %function
svc_getProcessId:
	str r0, [sp,#-0x4]!
	svc 0x35
	ldr r3, [sp], #4
	str r1, [r3]
	bx lr
