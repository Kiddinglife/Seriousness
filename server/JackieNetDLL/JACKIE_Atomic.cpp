#include "JACKIE_Atomic.h"


JACKIE_ATOMIC_LONG::JACKIE_ATOMIC_LONG() : value(0) { }
JACKIE_ATOMIC_LONG::JACKIE_ATOMIC_LONG(UInt32 initial) : value(initial) { }
JACKIE_ATOMIC_LONG::~JACKIE_ATOMIC_LONG() { }

UInt32 JACKIE_ATOMIC_LONG::Increment(void)
{
#ifdef _WIN32
	return (UInt32) InterlockedIncrement(&value);
#elif defined(ANDROID) || defined(__S3E__) || defined(__APPLE__)
	UInt32 v;
	mutex.Lock();
	++value;
	v = value;
	mutex.Unlock();
	return v;
#else
	return __sync_fetch_and_add(&value, (UInt32) 1);
#endif
}
UInt32 JACKIE_ATOMIC_LONG::Decrement(void)
{
#ifdef _WIN32
	return (UInt32) InterlockedDecrement(&value);
#elif defined(ANDROID) || defined(__S3E__) || defined(__APPLE__)
	UInt32 v;
	mutex.Lock();
	--value;
	v = value;
	mutex.Unlock();
	return v;
#else
	return __sync_fetch_and_add(&value, (UInt32) -1);
#endif
}

