#pragma once

typedef s32 LightLock;

typedef struct
{
	LightLock lock;
	u32 thread_tag;
	u32 counter;
} RecursiveLock;

static inline void __clrex(void)
{
	__asm__ __volatile__("clrex");
}

static inline s32 __ldrex(s32* addr)
{
	s32 val;
	__asm__ __volatile__("ldrex %[val], %[addr]" : [val] "=r" (val) : [addr] "Q" (*addr));
	return val;
}

static inline bool __strex(s32* addr, s32 val)
{
	bool res;
	__asm__ __volatile__("strex %[res], %[val], %[addr]" : [res] "=&r" (res) : [val] "r" (val), [addr] "Q" (*addr));
	return res;
}

void LightLock_Init(LightLock* lock);
void LightLock_Lock(LightLock* lock);
void LightLock_Unlock(LightLock* lock);

void RecursiveLock_Init(RecursiveLock* lock);
void RecursiveLock_Lock(RecursiveLock* lock);
void RecursiveLock_Unlock(RecursiveLock* lock);
