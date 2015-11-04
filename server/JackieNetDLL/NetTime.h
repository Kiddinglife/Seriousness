/*
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#ifndef NET_TIME_H
#define NET_TIME_H

#include "BasicTypes.h"
#include "DefaultNetDefines.h"
#include "DLLExport.h"

#if defined(_WIN32) && !defined(__GNUC__)  &&!defined(__GCCXML__)
#include <time.h>
struct JackieTimeZone
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
typedef timeval JackieTimeVal;
extern int JACKIE_EXPORT JackieGettimeofday(JackieTimeVal *tv, JackieTimeZone *tz = 0);
#else
#include <sys/time.h>
#include <unistd.h>
#endif

// Define USE_TIME_MS_64BITS if you want to use large types for GetTime
/// (takes more bandwidth when you transmit time though!)
// You would want to do this if your system is going to run long enough 
/// to overflow the millisecond counter (over a month)
#if USE_TIME_MS_64BITS ==1
typedef UInt64 Time;
typedef unsigned int TimeMS;
typedef UInt64 TimeUS;
#else
typedef unsigned int Time;
typedef unsigned int TimeMS;
typedef UInt64 TimeUS;
#endif

/// Same as GetTimeMS 
/// Holds the time in either a 32 or 64 bit variable, depending on __GET_TIME_64BIT
Time JACKIE_EXPORT GetTimeMS(void);

/// Return the time as 32 bit
/// \note The maximum delta between returned calls is 1 second - however, RakNet calls this constantly anyway. See NormalizeTime() in the cpp.
TimeMS JACKIE_EXPORT Get32BitsTimeMS(void);

/// Return the time as 64 bit
/// \note The maximum delta between returned calls is 1 second - however, RakNet calls this constantly anyway. See NormalizeTime() in the cpp.
TimeUS JACKIE_EXPORT Get64BitsTimeUS(void);

/// a > b?
extern  bool JACKIE_EXPORT GreaterThan(Time a, Time b);

/// a < b?
extern  bool JACKIE_EXPORT LessThan(Time a, Time b);

#endif