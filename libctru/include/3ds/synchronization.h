#pragma once

typedef s32 LightLock;

typedef struct
{
	LightLock lock;
	u32 thread_tag;
	u32 counter;
} RecursiveLock;

void LightLock_Init(LightLock* lock);
void LightLock_Lock(LightLock* lock);
void LightLock_Unlock(LightLock* lock);

void RecursiveLock_Init(RecursiveLock* lock);
void RecursiveLock_Lock(RecursiveLock* lock);
void RecursiveLock_Unlock(RecursiveLock* lock);
