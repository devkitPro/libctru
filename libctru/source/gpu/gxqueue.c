#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/gpu/gx.h>
#include <3ds/services/gspgpu.h>

#define MAX_PARALLEL_CMDS 3

extern u32* gxCmdBuf;
static gxCmdQueue_s* curQueue;
static bool isActive, isRunning, shouldStop;
static LightLock queueLock = 1;

static void gxCmdQueueDoCommands(void)
{
	if (shouldStop)
		return;
	int batchSize = curQueue->lastEntry+MAX_PARALLEL_CMDS-curQueue->curEntry;
	while (curQueue->curEntry < curQueue->numEntries && batchSize--)
	{
		gxCmdEntry_s* entry = &curQueue->entries[curQueue->curEntry++];
		gspSubmitGxCommand(gxCmdBuf, entry->data);
	}
}

void gxCmdQueueInterrupt(GSPGPU_Event irq)
{
	if (!isRunning || irq==GSPGPU_EVENT_PSC1 || irq==GSPGPU_EVENT_VBlank0 || irq==GSPGPU_EVENT_VBlank1)
		return;
	gxCmdQueue_s* runCb = NULL;
	LightLock_Lock(&queueLock);
	curQueue->lastEntry++;
	if (shouldStop)
	{
		curQueue = NULL;
		isActive = false;
		isRunning = false;
		shouldStop = false;
	}
	else if (curQueue->lastEntry < curQueue->numEntries)
		gxCmdQueueDoCommands();
	else
	{
		runCb = curQueue;
		isRunning = false;
	}
	LightLock_Unlock(&queueLock);
	if (runCb && runCb->callback)
		runCb->callback(runCb);
}

void gxCmdQueueClear(gxCmdQueue_s* queue)
{
	if (queue==curQueue && isRunning)
		svcBreak(USERBREAK_PANIC); // Shouldn't happen.
	queue->numEntries = 0;
	queue->curEntry = 0;
	queue->lastEntry = 0;
}

void gxCmdQueueAdd(gxCmdQueue_s* queue, const gxCmdEntry_s* entry)
{
	if (queue->numEntries == queue->maxEntries)
		svcBreak(USERBREAK_PANIC); // Shouldn't happen.
	memcpy(&queue->entries[queue->numEntries], entry, sizeof(gxCmdEntry_s));
	LightLock_Lock(&queueLock);
	queue->numEntries++;
	if (queue==curQueue && isActive && !isRunning)
	{
		isRunning = true;
		gxCmdQueueDoCommands();
	}
	LightLock_Unlock(&queueLock);
}

void gxCmdQueueRun(gxCmdQueue_s* queue)
{
	if (isRunning)
		return;
	curQueue = queue;
	isActive = true;
	if (queue->lastEntry < queue->numEntries)
	{
		isRunning = true;
		LightLock_Lock(&queueLock);
		gxCmdQueueDoCommands();
		LightLock_Unlock(&queueLock);
	} else
		isRunning = false;
}

void gxCmdQueueStop(gxCmdQueue_s* queue)
{
	if (!curQueue)
		return;
	LightLock_Lock(&queueLock);
	if (!isRunning)
	{
		curQueue = NULL;
		isActive = false;
	} else
		shouldStop = true;
	LightLock_Unlock(&queueLock);
}

bool gxCmdQueueWait(gxCmdQueue_s* queue, s64 timeout)
{
	u64 deadline = U64_MAX;
	if (timeout >= 0)
		deadline = svcGetSystemTick() + timeout;
	while (isRunning)
	{
		if (timeout >= 0 && (s64)(u64)(svcGetSystemTick()-deadline) >= 0)
			return false;
		gspWaitForAnyEvent();
	}
	return true;
}
