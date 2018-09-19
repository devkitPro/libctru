#pragma once

extern "C"
{
	#include <3ds/synchronization.h>
}

class LockGuard
{
public:
	~LockGuard()
	{
		LightLock_Unlock(&lock);
	}

	LockGuard(LightLock &lock) : lock(lock)
	{
		LightLock_Lock(&lock);
	}

private:
	LightLock &lock;
};
