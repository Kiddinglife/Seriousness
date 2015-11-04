#if defined(_WIN32)
#include "WindowsIncludes.h"
#if !defined(WINDOWS_PHONE_8)
// To call timeGetTime, on Code::Blocks, this needs to be libwinmm.a instead
#pragma comment(lib, "Winmm.lib")
#endif
#endif

#if defined(_WIN32)
//DWORD mProcMask;
//DWORD mSysMask;
//HANDLE mThread;
#else
#include <sys/time.h>
#include <unistd.h>
TimeUS initialTime;
#endif

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#include "NetTime.h"

static bool initialized = false;

#if defined(GET_TIME_SPIKE_LIMIT)&&GET_TIME_SPIKE_LIMIT > 0
#include "JackieSimpleMutex.h"
static TimeUS lastNormalizedReturnedValue = 0;
static TimeUS lastNormalizedInputValue = 0;
/// This constraints timer forward jumps to 1 second, and does not let it jump backwards
/// See http://support.microsoft.com/kb/274323 where the timer can sometimes jump forward by hours or days
/// This also has the effect where debugging a sending system won't treat the time spent halted past 1 second as elapsed network time
TimeUS NormalizeTime(TimeUS timeIn)
{
	TimeUS diff, lastNormalizedReturnedValueCopy;
	static JackieSimpleMutex mutex;

	mutex.Lock();
	if( timeIn >= lastNormalizedInputValue )
	{
		diff = timeIn - lastNormalizedInputValue;
		if( diff > GET_TIME_SPIKE_LIMIT )
			lastNormalizedReturnedValue += GET_TIME_SPIKE_LIMIT;
		else
			lastNormalizedReturnedValue += diff;
	} else
		lastNormalizedReturnedValue += GET_TIME_SPIKE_LIMIT;

	lastNormalizedInputValue = timeIn;
	lastNormalizedReturnedValueCopy = lastNormalizedReturnedValue;
	mutex.Unlock();

	return lastNormalizedReturnedValueCopy;
}
#endif // #if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0

Time GetTimeMS(void)
{
	return (Time) ( Get64BitsTimeUS() / 1000 );
}

TimeMS Get32BitsTimeMS(void)
{
	return (TimeMS) ( Get64BitsTimeUS() / 1000 );
}

#if defined(_WIN32)

TimeUS GetTimeUS_Windows(void)
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
#elif defined(__GNUC__)  || defined(__GCCXML__) || defined(__S3E__)
TimeUS GetTimeUS_Linux(void)
{
	timeval tp;
	if( initialized == false )
	{
		gettimeofday(&tp, 0);
		initialized = true;
		// I do this because otherwise Time in milliseconds won't work as it will underflow when dividing by 1000 to do the conversion
		initialTime = ( tp.tv_sec ) * ( TimeUS ) 1000000 + ( tp.tv_usec );
	}

	// GCC
	TimeUS curTime;
	gettimeofday(&tp, 0);

	curTime = ( tp.tv_sec ) * ( TimeUS ) 1000000 + ( tp.tv_usec );

#if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
	return NormalizeTime(curTime - initialTime);
#else
	return curTime - initialTime;
#endif // #if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
}
#endif

TimeUS Get64BitsTimeUS(void)
{
#if   defined(_WIN32)
	return GetTimeUS_Windows();
#else
	return GetTimeUS_Linux();
#endif
}

bool GreaterThan(Time a, Time b)
{
	// a > b?
	const Time halfSpan = (Time) ( ( (Time) (const Time) -1 ) / (Time) 2 );
	return b != a && b - a > halfSpan;
}

bool LessThan(Time a, Time b)
{
	// a < b?
	const Time halfSpan = ( (Time) (const Time) -1 ) / (Time) 2;
	return b != a && b - a < halfSpan;
}

#if defined(_WIN32) && !defined(__GNUC__)  &&!defined(__GCCXML__)

int JackieGettimeofday(JackieTimeVal *tv, JackieTimeZone *tz)
{
#if defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
	// _tzset not supported
	(void) tv;
	(void) tz;
#else

	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if( 0 != tv )
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres /= 10;  /*convert into microseconds*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long) ( tmpres / 1000000UL );
		tv->tv_usec = (long) ( tmpres % 1000000UL );
	}

	if( 0 != tz )
	{
		if( !tzflag )
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

#endif

	return 0;
}
#endif
