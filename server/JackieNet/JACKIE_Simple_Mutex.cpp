#include "JACKIE_Simple_Mutex.h"
#include "DefaultNetDefines.h"

JackieSimpleMutex::JackieSimpleMutex() //: isInitialized(false)
{
	// Prior implementation of Initializing in Lock() was not threadsafe
	Init();
}

JackieSimpleMutex::~JackieSimpleMutex()
{
#ifdef _WIN32
	DeleteCriticalSection(&criticalSection);
#else
	pthread_mutex_destroy(&hMutex);
#endif
}

#ifdef _WIN32
#ifdef _DEBUG
#include <stdio.h>
#endif
#endif


void JackieSimpleMutex::Lock(void)
{
#ifdef _WIN32
	EnterCriticalSection(&criticalSection);
#else
	int error = pthread_mutex_lock(&hMutex);
	(void) error;
	assert(error == 0);
#endif
}


void JackieSimpleMutex::Unlock(void)
{
#ifdef _WIN32
	LeaveCriticalSection(&criticalSection);
#else
	int error = pthread_mutex_unlock(&hMutex);
	(void) error;
	assert(error == 0);
#endif
}

void JackieSimpleMutex::Init(void)
{
#if defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
	InitializeCriticalSectionEx(&criticalSection, 0, CRITICAL_SECTION_NO_DEBUG_INFO);
#elif defined(_WIN32)
	InitializeCriticalSection(&criticalSection);
#else
	int error = pthread_mutex_init(&hMutex, 0);
	(void) error;
	assert(error == 0);
#endif
}

