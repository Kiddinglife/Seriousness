#include "JACKIE_Atomic.h"

unsigned int JackieAtomicLong::Increment(void)
{
#ifdef _WIN32
	return (unsigned int) InterlockedIncrement(&value);
#elif defined(ANDROID) || defined(__S3E__) || defined(__APPLE__)
	unsigned int v;
	mutex.Lock();
	++value;
	v = value;
	mutex.Unlock();
	return v;
#else
	return __sync_fetch_and_add(&value, (unsigned int) 1);
#endif
}
unsigned int JackieAtomicLong::Decrement(void)
{
#ifdef _WIN32
	return (unsigned int) InterlockedDecrement(&value);
#elif defined(ANDROID) || defined(__S3E__) || defined(__APPLE__)
	unsigned int v;
	mutex.Lock();
	--value;
	v = value;
	mutex.Unlock();
	return v;
#else
	return __sync_fetch_and_add(&value, (Int32) -1);
#endif
}

