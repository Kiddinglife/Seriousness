/*!
 * \file ServerApplication.h
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
#include "CompileFeatures.h"
//#include "SecureHandshake.h"
#include "JACKIE_Atomic.h"
//#include "DS_Queue.h"

namespace JACKIE_INET
{

	class ServerApplication : public IServerApplication
	{
		public:

#ifdef USE_SINGLE_THREAD_TO_SEND_AND_RECV
		volatile bool endThreads; ///Set this to true to terminate the thread execution 
		volatile bool isMainLoopThreadActive; ///true if the peer thread is active. 
#else
		bool endThreads; bool isMainLoopThreadActive;
#endif

#if LIBCAT_SECURITY == 1
		// Encryption and security
		bool _using_security, _require_client_public_key;
		char my_public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
		cat::ServerEasyHandshake *_server_handshake;
		cat::CookieJar *_cookie_jar;
		bool InitializeClientSecurity(RequestedConnectionStruct *rcs, const char *public_key);
#endif

		JACKIE_INet_GUID myGuid;
		int defaultMTUSize;
		bool trackFrequencyTable;

		/// Store the maximum number of peers allowed to connect
		unsigned int maximumNumberOfPeers;
		///Store the maximum incoming connection allowed 
		unsigned int maximumIncomingConnections;

		char incomingPassword[256];
		unsigned char incomingPasswordLength;

		//////////////////////////////////////////////////////////////////////////
		/// This is an array of pointers to RemoteEndPoint
		/// This allows us to preallocate the list when starting,
		/// so we don't have to allocate or delete at runtime.
		/// Another benefit is that is lets us add and remove active
		/// players simply by setting systemAddress
		/// and moving elements in the list by copying pointers variables
		/// without affecting running threads, even if they are in the reliability layer
		//////////////////////////////////////////////////////////////////////////
		RemoteEndPoint* remoteSystemList;

		//////////////////////////////////////////////////////////////////////////
		/// activeSystemList holds a list of pointers and is preallocated to be the same size as 
		/// remoteSystemList. It is updated only by the network thread, but read by both threads
		/// When the isActive member of RemoteEndPoint is set to true or false, that system is 
		/// added to this list of pointers. Threadsafe because RemoteEndPoint is preallocated, 
		/// and the list is only added to, not removed from
		//////////////////////////////////////////////////////////////////////////
		RemoteEndPoint** activeSystemList;

		unsigned int activeSystemListSize;

		/// Use a hash, with binaryAddress plus port mod length as the index
		RemoteEndPointIndex **remoteSystemLookup;

		unsigned int bytesSentPerSecond, bytesReceivedPerSecond;

		bool(*recvHandler)( JISRecvParams* );
		void(*userUpdateThreadPtr)( IServerApplication *, void * );
		void *userUpdateThreadData;

		TimeMS defaultTimeoutTime;

		/// Do we occasionally ping the other systems?
		bool occasionalPing;
		bool allowInternalRouting;

		JACKIE_INET_Address IPAddress[MAX_COUNT_LOCAL_IP_ADDR];

		/// How long it has been since things were updated by a call
		/// to receiveUpdate thread uses this to determine how long to sleep for
		/// unsigned int lastUserUpdateCycle;
		/// True to allow connection accepted packets from anyone. 
		/// False to only allow these packets from servers we requested a connection to.
		bool allowConnectionResponseIPMigration;


		int splitMessageProgressInterval;
		TimeMS unreliableTimeout;
		unsigned int maxOutgoingBPS;
		JACKIE_INET_Address firstExternalID;

		/// This is used to return a number to the user 
		/// when they call Send identifying the message
		/// This number will be returned back with ID_SND_RECEIPT_ACKED
		/// or ID_SND_RECEIPT_LOSS and is only returned with the reliability 
		/// types that DOES NOT contain 'NOT' in the name
		JACKIE_Simple_Mutex sendReceiptSerialMutex;
		UInt32 sendReceiptSerial;

		// Nobody would use the internet simulator in a final build.
#ifdef _DEBUG
		double _packetloss;
		unsigned short _minExtraPing;
		unsigned short _extraPingVariance;
		bool limitConnectionFrequencyFromTheSameIP;
#endif

		// Generate and store a unique GUID
		void GenerateGUID(void);
		unsigned int GetSystemIndexFromGuid(const JACKIE_INet_GUID& input) const;

		ServerApplication();
		virtual ~ServerApplication();

		bool IsActive(void) const { return endThreads == false; }
		virtual void OnJISRecv(JISRecvParams *recvStruct) override;
		virtual void DeallocJISRecvParams(JISRecvParams *s, const char *file, UInt32 line) override;
		virtual JISRecvParams * AllocJISRecvParams(const char *file, UInt32 line) override;

		virtual StartupResult Start(UInt32 maxConnections, JACKIE_LOCAL_SOCKET *socketDescriptors, UInt32 socketDescriptorCount, Int32 threadPriority = -99999) override;

		void ResetSendReceipt(void);
	};
}

#endif 

