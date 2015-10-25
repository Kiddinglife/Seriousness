#ifndef ThreadConditionSignalEvent_H_
#define ThreadConditionSignalEvent_H_

#include "DLLExport.h"

#ifdef _WIN32
#include "WindowsIncludes.h"
#else
#include <pthread.h>
#include <sys/types.h>
#include "JACKIE_Simple_Mutex.h"
#endif
namespace JACKIE_INET
{
	class JACKIE_EXPORT ThreadConditionSignalEvent
	{
		private:
#ifdef _WIN32
		HANDLE eventList;
#else
		JACKIE_Simple_Mutex isSignaledMutex;
		bool isSignaled;
#if !defined(ANDROID)
		pthread_condattr_t condAttr;
#endif
		pthread_cond_t eventList;
		pthread_mutex_t hMutex;
		pthread_mutexattr_t mutexAttr;
#endif

		public:
		ThreadConditionSignalEvent();
		~ThreadConditionSignalEvent();

		void Init(void);
		void Close(void);
		void TriggerEvent(void);
		void WaitEvent(int timeoutMs);
	};
}
#endif
