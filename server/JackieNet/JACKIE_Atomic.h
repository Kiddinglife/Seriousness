#ifndef JACKIE_ATOMIC_H
#define JACKIE_ATOMIC_H

#include "DLLExport.h"
#include "BasicTypes.h"
#include "SockOSIncludes.h"


#if defined(ANDROID) || defined(__S3E__) || defined(__APPLE__)
// __sync_fetch_and_add not supported apparently
#include "JACKIE_Simple_Mutex.h"
#endif

class JACKIE_EXPORT JACKIE_ATOMIC_LONG
{
	protected:
#ifdef _WIN32
	volatile LONG value;
#elif defined(ANDROID) || defined(__S3E__) || defined(__APPLE__)
	// __sync_fetch_and_add not supported apparently
	JackieSimpleMutex mutex;
	unsigned int value;
#else
	volatile unsigned int value;
#endif

	public:
	JACKIE_ATOMIC_LONG();
	explicit JACKIE_ATOMIC_LONG(unsigned int initial);
	~JACKIE_ATOMIC_LONG();

	// Returns variable value after changing it
	unsigned int Increment(void);
	// Returns variable value after changing it
	unsigned int Decrement(void);
	unsigned int GetValue(void) const { return value; }
};
#endif
