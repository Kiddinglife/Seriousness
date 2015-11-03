/*!
 * \file ServerApplication.h
 * \author mengdi
 * \date Oct 18, 2015
 *\ Remember we never push null pointer to any kind of queue including lockfree queue
 *\ and normal queue so that we will never pop out a null pointer from any kind of queue
 */

#ifndef SERVERAPPLICATION_H_
#define SERVERAPPLICATION_H_

#include "DLLExport.h"
#include "ReliabilityLayer.h"
#include "IServerApplication.h"
#include "JackieStream.h"
//#include "SingleProducerConsumer.h"
#include "JackieSimpleMutex.h"
//#include "DS_OrderedList.h"
//#include "RakString.h"
#include "JACKIE_Thread.h"
//#include "RakNetSmartPtr.h"
#include "ThreadConditionSignalEvent.h"
#include "CompileFeatures.h"
//#include "SecureHandshake.h"
#include "JACKIE_Atomic.h"
#include "RingBufferQueue.h"
#include "LockFreeQueue.h"
#include "MemoryPool.h"
#include "IPlugin.h"
#include "RandomSeedCreator.h"
#include "JackieINetSocket.h"
using namespace DataStructures;

namespace JACKIE_INET
{
	class JACKIE_EXPORT ServerApplication : public IServerApplication
	{
		//private:
		public:
		STATIC_FACTORY_DECLARATIONS(ServerApplication);

#if LIBCAT_SECURITY == 1
		// Encryption and security
		bool _using_security, _require_client_public_key;
		char my_public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
		cat::ServerEasyHandshake *_server_handshake;
		cat::CookieJar *_cookie_jar;
		bool InitializeClientSecurity(RequestedConnectionStruct *rcs, const char *public_key);
#endif

		Ultils::RandomSeedCreator rnr;
		JackieStream sendBitStream;

		JACKIE_INet_GUID myGuid;
		JACKIE_INET_Address IPAddress[MAX_COUNT_LOCAL_IP_ADDR];
		JACKIE_INET_Address firstExternalID;


		/// Store the maximum number of peers allowed to connect
		unsigned int maxConnections;
		///Store the maximum incoming connection allowed 
		unsigned int maxIncomeConnections;


		char incomingPassword[256];
		unsigned char incomingPasswordLength;


		/// This is an array of pointers to RemoteEndPoint
		/// This allows us to preallocate the list when starting,
		/// so we don't have to allocate or delete at runtime.
		/// Another benefit is that is lets us add and remove active
		/// players simply by setting systemAddress
		/// and moving elements in the list by copying pointers variables
		/// without affecting running threads, even if they are in the reliability layer
		RemoteEndPoint* remoteSystemList;
		/// activeSystemList holds a list of pointers and is preallocated to be the same size as 
		/// remoteSystemList. It is updated only by the network thread, but read by both threads
		/// When the isActive member of RemoteEndPoint is set to true or false, that system is 
		/// added to this list of pointers. Threadsafe because RemoteEndPoint is preallocated, 
		/// and the list is only added to, not removed from
		RemoteEndPoint** activeSystemList;
		unsigned int activeSystemListSize;
		/// Use a hash, with binaryAddress plus port mod length as the index
		RemoteEndPointIndex **remoteSystemLookup;



		bool(*recvHandler)( JISRecvParams* );
		void(*userUpdateThreadPtr)( IServerApplication *, void * );
		void *userUpdateThreadData;
		bool(*incomeDatagramEventHandler)( JISRecvParams * );
		bool updateCycleIsRunning;
		volatile bool endThreads; ///Set this to true to terminate threads execution 
		volatile bool isNetworkUpdateThreadActive; ///true if the send thread is active. 
		JACKIE_ATOMIC_LONG isRecvPollingThreadActive; ///true if the recv thread is active. 
		ThreadConditionSignalEvent quitAndDataEvents;


		int defaultMTUSize;
		bool trackFrequencyTable;
		unsigned int bytesSentPerSecond;
		unsigned int  bytesReceivedPerSecond;
		/// Do we occasionally ping the other systems?
		bool occasionalPing;
		bool allowInternalRouting;
		/// How long it has been since things were updated by a call
		/// to receiveUpdate thread uses this to determine how long to sleep for
		/// unsigned int lastUserUpdateCycle;
		/// True to allow connection accepted packets from anyone. 
		/// False to only allow these packets from servers we requested a connection to.
		bool allowConnectionResponseIPMigration;
		int splitMessageProgressInterval;
		TimeMS unreliableTimeout;
		TimeMS defaultTimeoutTime;
		unsigned int maxOutgoingBPS;
		bool limitConnectionFrequencyFromTheSameIP;
		// Nobody would use the internet simulator in a final build.
#ifdef _DEBUG
		double _packetloss;
		unsigned short _minExtraPing;
		unsigned short _extraPingVariance;
#endif


		/// This is used to return a number to the user 
		/// when they call Send identifying the message
		/// This number will be returned back with ID_SND_RECEIPT_ACKED
		/// or ID_SND_RECEIPT_LOSS and is only returned with the reliability 
		/// types that DOES NOT contain 'NOT' in the name
		JackieSimpleMutex sendReceiptSerialMutex;
		unsigned int sendReceiptSerial;


		/// in Multi-threads app, used only by send thread to alloc packet
		MemoryPool<Packet, 512, 8> packetPool;
		/// used only by ? thread to alloc RemoteEndPointIndex
		MemoryPool<RemoteEndPointIndex, 1024, 8> remoteSystemIndexPool;
		/// in single thread app, default JISRecvParams pool is JISRecvParamsPool
		/// in Multi-threads app, used only by recv thread to alloc and dealloc JISRecvParams
		/// via anpothe
		MemoryPool<JISRecvParams, 512, 8>* JISRecvParamsPool;
		//MemoryPool<JISRecvParams, 512, 8> JISRecvParamsPool;
		MemoryPool<Command, 512, 8> commandPool;


		struct PacketFollowedByData { Packet p; unsigned char data[1]; };
		//struct SocketQueryOutput{	RingBufferQueue<JACKIE_INet_Socket*> sockets;};
		MemoryPool <RingBufferQueue<JackieINetSocket*>, 8, 4> socketQueryOutput;


#if USE_SINGLE_THREAD == 0
		// it works in this way:
		// recv thread uses pool to allocate JISRecvParams* ptr and push it to  
		// bufferedAllocatedRecvParamQueue while send thread pops ptr out from 
		// bufferedAllocatedRecvParamQueue and use it do something, when finished, 
		// send thread . then push this ptr into bufferedDeallocatedRecvParamQueue
		// then recv thread will pop this ptr out from bufferedDeallocatedRecvParamQueue
		// and use this ptr to deallocate. In this way, we do not need locks the pool but using 
		// lockfree queue

		/// shared between recv and send thread to store allocated JISRecvParams PTR
		LockFreeQueue<JISRecvParams*, SERVER_QUEUE_PTR_SIZE>* allocRecvParamQ;
		/// shared by recv and send thread to store JISRecvParams PTR that is being dellacated
		LockFreeQueue<JISRecvParams*, SERVER_QUEUE_PTR_SIZE>* deAllocRecvParamQ;

		LockFreeQueue<Command*, SERVER_QUEUE_PTR_SIZE> allocCommandQ;
		LockFreeQueue<Command*, SERVER_QUEUE_PTR_SIZE> deAllocCommandQ;

		/// shared by recv thread and send thread
		LockFreeQueue<Packet*, SERVER_QUEUE_PTR_SIZE> allocPacketQ;
		/// shared by recv thread and send thread
		LockFreeQueue<Packet*, SERVER_QUEUE_PTR_SIZE> deAllocPacketQ;

		LockFreeQueue<JACKIE_INET_Address> connReqCancelQ;

#else

		/// shared between recv and send thread
		RingBufferQueue<JISRecvParams*, SERVER_QUEUE_PTR_SIZE>* allocRecvParamQ;
		/// shared by recv and send thread to  store JISRecvParams PTR that is being dellacated
		RingBufferQueue<JISRecvParams*, SERVER_QUEUE_PTR_SIZE>* deAllocRecvParamQ;

		RingBufferQueue<Command*, SERVER_QUEUE_PTR_SIZE> allocCommandQ;
		RingBufferQueue<Command*, SERVER_QUEUE_PTR_SIZE> deAllocCommandQ;

		/// shared by recv thread and send thread
		RingBufferQueue<Packet*, SERVER_QUEUE_PTR_SIZE> allocPacketQ;
		/// shared by recv thread and send thread
		RingBufferQueue<Packet*, SERVER_QUEUE_PTR_SIZE> deAllocPacketQ;

		RingBufferQueue<JACKIE_INET_Address, SERVER_QUEUE_PTR_SIZE> connReqCancelQ;

#endif

		/// thread unsafe we cannot use lockfree because we have to 
		/// do removeAt(index) operation on this queue
		RingBufferQueue<ConnectionRequest*, SERVER_QUEUE_PTR_SIZE> connReqQ;
		JackieSimpleMutex connReqQMutex;

		/// only user thread pushtail into the queue, other threads only read it so no need lock
		RingBufferQueue<JackieINetSocket*, 8 > JISList;

		// Threadsafe, and not thread safe
		LockFreeQueue<IPlugin*, 16> pluginListTS;
		RingBufferQueue<IPlugin*, 16> pluginListNTS;



		public:
		ServerApplication();
		virtual ~ServerApplication();

		void InitIPAddress(void);
		void DeallocJISList(void);
		void ResetSendReceipt(void);
		Packet* GetPacketOnce(void);

		void ProcessOneRecvParam(JISRecvParams* recvParams);
		bool ProcessOneOfflineRecvParam(JISRecvParams* recvParams,
			bool* isOfflinerecvParams);
		void ProcessBufferedCommand(JISRecvParams* recvParams, JackieStream &updateBitStream);
		void ProcessConnectionRequestCancelQ(void);
		void ProcessAllocJISRecvParamsQ(void);
		void ProcessAllocCommandQ(TimeUS& timeUS, TimeMS& timeMS);
		void ProcessConnectionRequestQ(TimeUS& timeUS, TimeMS& timeMS);
		void AdjustTimestamp(Packet*& incomePacket) const;


		virtual StartupResult Start(unsigned int maxConnections,
			BindSocket *socketDescriptors,
			unsigned int socketDescriptorCount, Int32 threadPriority = -99999) override;
		void End(unsigned int blockDuration, unsigned char orderingChannel = 0,
			PacketSendPriority disconnectionNotificationPriority = BUFFERED_THIRDLY_SEND);


		/// In multi-threads app and single- thread app,these 3 functions
		/// are called only  by recv thread. the recvStruct will be obtained from 
		/// bufferedDeallocatedRecvParamQueue, so it is thread safe
		/// It is Caller's responsibility to make sure s != 0
		virtual void ReclaimOneJISRecvParams(JISRecvParams *s, UInt32 index) override;
		void ReclaimAllJISRecvParams(UInt32 deAlloclJISRecvParamsQIndex);
		virtual JISRecvParams * AllocJISRecvParams(UInt32 deAlloclJISRecvParamsQIndex) 
			override;


		/// Generate and store a unique GUID
		void GenerateGUID(void) { myGuid.g = Get64BitUniqueRandomNumber(); }
		/// Mac address is a poor solution because 
		/// you can't have multiple connections from the same system
		UInt64 Get64BitUniqueRandomNumber(void);
		unsigned int GetSystemIndexFromGuid(const JACKIE_INet_GUID& input) const;


		void ClearAllCommandQs(void);
		void ClearSocketQueryOutputs(void);
		void ClearAllRecvParamsQs(UInt32 index);


		/// send thread will push trail this packet to buffered alloc queue in multi-threads env
		/// for the furture use of recv thread by popout
		Packet* AllocPacket(unsigned int dataSize);
		Packet* AllocPacket(unsigned dataSize,  char *data);
		/// send thread will take charge of dealloc packet in multi-threads env
		void ReclaimAllPackets(void);
		/// recv thread will push tail this packet to buffered dealloc queue in multi-threads env
		void ReclaimOnePacket(Packet *packet);



		/// send thread will push trail this command PTR  to buffered command
		/// dealloc queue in multi-threads env
		void ReclaimOneCommand(Command* bufferedCommand);
		/// recv thread will reclaim all commands  into command pool in multi-threads env
		void ReclaimAllCommands();
		/// only recv thread will take charge of alloc packet in multi-threads env
		Command* AllocCommand();
		void ExecuteComand(Command* cmd) { allocCommandQ.PushTail(cmd); };


		bool RunNetworkUpdateCycleOnce(void);
		void RunRecvCycleOnce(UInt32 in = 0);
		Packet* RunGetPacketCycleOnce(void);


		///========================================
		/// function  CreateRecvPollingThread 
		/// Access  private  
		/// Param [in] [int threadPriority]
		/// Returns int
		/// Remarks this function is not used because because , a thread 
		/// that calls the Start() func plays recv thread
		/// author mengdi[Jackie]
		///========================================
		int CreateRecvPollingThread(int threadPriority, UInt32 index);
		int CreateNetworkUpdateThread(int threadPriority);
		///========================================
		/// function  StopRecvPollingThread 
		/// Access  public  
		/// Param [in] [void]  
		/// Returns void
		/// Remarks: because we use app main thread as recv thread, it 
		/// will never block so no need to use this
		/// author mengdi[Jackie]
		///========================================
		void StopRecvPollingThread(void);
		void StopNetworkUpdateThread(void);
		bool IsActive(void) const { return endThreads == false; }


		void PacketGoThroughPluginCBs(Packet*& incomePacket);
		void PacketGoThroughPlugins(Packet*& incomePacket);
		void UpdatePlugins(void);


		const JACKIE_INet_GUID& GetMyGUID(void) const { return myGuid; }
		RemoteEndPoint* GetRemoteEndPoint(const JACKIE_INET_Address& sa,
			bool neededBySendThread, bool onlyWantActiveEndPoint) const;
		RemoteEndPoint* GetRemoteEndPoint(const JACKIE_INET_Address& sa)
			const;
		RemoteEndPoint* GetRemoteEndPoint(const JACKIE_INET_Address_GUID_Wrapper&
			senderWrapper, bool neededBySendThread,
			bool onlyWantActiveEndPoint) const;
		RemoteEndPoint* GetRemoteEndPoint(const JACKIE_INet_GUID& senderGUID,
			bool onlyWantActiveEndPoint) const;
		Int32 GetRemoteEndPointIndex(const JACKIE_INET_Address &sa) const;
		void RefRemoteEndPoint(const JACKIE_INET_Address &sa, unsigned int index);
		void DeRefRemoteEndPoint(const JACKIE_INET_Address &sa);


		bool SendRightNow(TimeUS currentTime, bool useCallerAlloc,
			Command* bufferedCommand);
		void CloseConnectionInternally(bool sendDisconnectionNotification,
			bool performImmediate, Command* bufferedCommand);


		friend JACKIE_THREAD_DECLARATION(RunNetworkUpdateCycleLoop);
		friend JACKIE_THREAD_DECLARATION(RunRecvCycleLoop);
		friend JACKIE_THREAD_DECLARATION(UDTConnect);
	};
}

#endif 

