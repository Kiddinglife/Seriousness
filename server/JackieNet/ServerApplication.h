/*!
 * \file ServerApplication.h
 *
 * \author mengdi
 * \date Oct 18, 2015
 */
#ifndef SERVERAPPLICATION_H_
#define SERVERAPPLICATION_H_

//#include "ReliabilityLayer.h"
#include "IServerApplication.h"
//#include "BitStream.h"
//#include "SingleProducerConsumer.h"
#include "JACKIE_Simple_Mutex.h"
//#include "DS_OrderedList.h"
#include "DLLExport.h"
//#include "RakString.h"
//#include "RakThread.h"
//#include "RakNetSmartPtr.h"
//#include "DS_ThreadsafeAllocatingQueue.h"
//#include "SignaledEvent.h"
//#include "NativeFeatureIncludes.h"
//#include "SecureHandshake.h"
#include "JACKIE_Atomic.h"
//#include "DS_Queue.h"

namespace JACKIE_INET
{
	class ServerApplication : public IServerApplication
	{
		public:
		ServerApplication();
		virtual ~ServerApplication();

		virtual StartupResult Start(UInt32 maxConnections, JACKIE_LOCAL_SOCKET *socketDescriptors, UInt32 socketDescriptorCount, Int32 threadPriority = -99999) override { return STARTED; }

	};
}

#endif 

