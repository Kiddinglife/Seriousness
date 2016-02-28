#ifndef JACKIE_ATOMIC_H
#define JACKIE_ATOMIC_H

#include "DLLExport.h"
#include "BasicTypes.h"
#include "SockOSIncludes.h"


#if defined(ANDROID) || defined(__S3E__) || defined(__APPLE__)
// __sync_fetch_and_add not supported apparently
#include "JACKIE_Simple_Mutex.h"
#endif

struct JACKIE_EXPORT JackieAtomicLong
{
#ifdef _WIN32
	 LONG value;
#elif defined(ANDROID) || defined(__S3E__) || defined(__APPLE__)
	// __sync_fetch_and_add not supported apparently
	JackieSimpleMutex mutex;
	unsigned int value;
#else
	 unsigned int value;
#endif

	// Returns variable value after changing it
	unsigned int Increment(void);
	// Returns variable value after changing it
	unsigned int Decrement(void);
	unsigned int GetValue(void) const { return value; }
};
#endif
