/*
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#ifndef NET_TIME_H_
#define NET_TIME_H_

#include "BasicTypes.h"
#include "DefaultNetDefines.h"
#include "DLLExport.h"

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

static bool initialized = false;

// Define USE_TIME_MS_64BITS if you want to use large types for GetTime
/// (takes more bandwidth when you transmit time though!)
// You would want to do this if your system is going to run long enough 
/// to overflow the millisecond counter (over a month)
#if USE_TIME_MS_64BITS ==1
typedef UInt64 Time;
typedef UInt32 TimeMS;
typedef UInt64 TimeUS;
#else
typedef UInt32 Time;
typedef UInt32 TimeMS;
typedef UInt64 TimeUS;
#endif

/// win32 soes not have gettimeofday(), we add it to make it compatiable in plateforms
#if defined(_WIN32) && !defined(__GNUC__)  &&!defined(__GCCXML__)
#include <time.h>
#include "WindowsIncludes.h"
#if !defined(WINDOWS_PHONE_8)
// To call timeGetTime, on Code::Blocks, this needs to be libwinmm.a instead
#pragma comment(lib, "Winmm.lib")
#endif
struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};
#if defined(WINDOWS_STORE_RT)
struct timeval
{
	long    tv_sec;
	long    tv_usec;
};
#endif
JACKIE_EXPORT extern int  gettimeofday(struct timeval *tv, struct timezone *tz);
static TimeUS GetTimeUS_Windows(void)
{
	if( initialized == false )
	{
		initialized = true;

		// Save the current process
#if !defined(_WIN32_WCE)
		//		HANDLE mProc = GetCurrentProcess();

		// Get the current Affinity
#if _MSC_VER >= 1400 && defined (_M_X64)
		//		GetProcessAffinityMask(mProc, (PDWORD_PTR)&mProcMask, (PDWORD_PTR)&mSysMask);
#else
		//		GetProcessAffinityMask(mProc, &mProcMask, &mSysMask);
#endif
		//		mThread = GetCurrentThread();

#endif // _WIN32_WCE
	}

	// 9/26/2010 In China running LuDaShi, QueryPerformanceFrequency has to be called every time because CPU clock speeds can be different
	TimeUS curTime;
	LARGE_INTEGER PerfVal;
	LARGE_INTEGER yo1;

	QueryPerformanceFrequency(&yo1);
	QueryPerformanceCounter(&PerfVal);

	__int64 quotient, remainder;
	quotient = ( ( PerfVal.QuadPart ) / yo1.QuadPart );
	remainder = ( ( PerfVal.QuadPart ) % yo1.QuadPart );
	curTime = (TimeUS) quotient*(TimeUS) 1000000 + ( remainder*(TimeUS) 1000000 / yo1.QuadPart );

#if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
	return NormalizeTime(curTime);
#else
	return curTime;
#endif // #if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
}
#else
#include <sys/time.h>
#include <unistd.h>
static TimeUS initialTime;
static TimeUS GetTimeUS_Linux(void)
{
	timeval tp;

	if( initialized == false )
	{
		gettimeofday(&tp, 0);
		initialized = true;
		// I do this because otherwise Time in milliseconds won't work as it will underflow when dividing by 1000 to do the conversion
		initialTime = ( tp.tv_sec ) * (TimeUS) 1000000 + ( tp.tv_usec );
	}

	// GCC
	TimeUS curTime;
	gettimeofday(&tp, 0);

	curTime = ( tp.tv_sec ) * (TimeUS) 1000000 + ( tp.tv_usec );

#if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
	return NormalizeTime(curTime - initialTime);
#else
	return curTime - initialTime;
#endif // #if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
}
#endif

/// Return the time as 64 bit
/// \note The maximum delta between returned calls is 1 second - however, RakNet calls this constantly anyway. See NormalizeTime() in the cpp.
inline JACKIE_EXPORT extern TimeUS Get64BitsTimeUS(void)
{
#if   defined(_WIN32)
	return GetTimeUS_Windows();
#else
	return GetTimeUS_Linux();
#endif
}

/// Same as GetTimeMS 
/// Holds the time in either a 32 or 64 bit variable, depending on __GET_TIME_64BIT
// GetTime
inline JACKIE_EXPORT extern Time  GetTimeMS(void)
{
	return (Time) ( Get64BitsTimeUS() / 1000 );
}

/// Return the time as 32 bit
/// \note The maximum delta between returned calls is 1 second - however, RakNet calls this constantly anyway. See NormalizeTime() in the cpp.
// GetTimeMS
inline JACKIE_EXPORT TimeMS Get32BitsTimeMS(void)
{
	return (TimeMS) ( Get64BitsTimeUS() / 1000 );
}

inline JACKIE_EXPORT extern bool  GreaterThan(Time a, Time b)
{
	// a > b?
	const Time halfSpan = (Time) ( ( (Time) (const Time) -1 ) / (Time) 2 );
	return b != a && b - a > halfSpan;
}
inline JACKIE_EXPORT extern bool  LessThan(Time a, Time b)
{
	// a < b?
	const Time halfSpan = ( (Time) (const Time) -1 ) / (Time) 2;
	return b != a && b - a < halfSpan;
}

JACKIE_EXPORT extern void JackieSleep(unsigned int ms);
#endif