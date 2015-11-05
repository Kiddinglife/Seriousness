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
#include "ArraryQueue.h"
#include "LockFreeQueue.h"
#include "MemoryPool.h"
#include "IPlugin.h"
#include "RandomSeedCreator.h"
#include "JackieINetSocket.h"

using namespace DataStructures;

namespace JACKIE_INET
{
	class JACKIE_EXPORT ServerApplication  //: public IServerApplication
	{
		private:
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

		JackieGUID myGuid;
		JackieAddress IPAddress[MAX_COUNT_LOCAL_IP_ADDR];
		JackieAddress firstExternalID;


		/// Store the maximum number of peers allowed to connect
		UInt32 maxConnections;
		///Store the maximum incoming connection allowed 
		UInt32 maxPassiveConnections;


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
		UInt32 activeSystemListSize;

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
		UInt32 bytesSentPerSecond;
		UInt32  bytesReceivedPerSecond;
		/// Do we occasionally ping the other systems?
		bool occasionalPing;
		bool allowInternalRouting;
		/// How long it has been since things were updated by a call
		/// to receiveUpdate thread uses this to determine how long to sleep for
		/// UInt32 lastUserUpdateCycle;
		/// True to allow connection accepted packets from anyone. 
		/// False to only allow these packets from servers we requested a connection to.
		bool allowConnectionResponseIPMigration;
		int splitMessageProgressInterval;
		TimeMS unreliableTimeout;
		TimeMS defaultTimeoutTime;
		UInt32 maxOutgoingBPS;
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
		UInt32 sendReceiptSerial;


		/// in Multi-threads app, used only by send thread to alloc packet
		MemoryPool<Packet> packetPool;
		/// used only by ? thread to alloc RemoteEndPointIndex
		MemoryPool<RemoteEndPointIndex> remoteSystemIndexPool;
		/// in single thread app, default JISRecvParams pool is JISRecvParamsPool
		/// in Multi-threads app, used only by recv thread to alloc and dealloc JISRecvParams
		/// via anpothe
		MemoryPool<JISRecvParams>* JISRecvParamsPool;
		//MemoryPool<JISRecvParams, 512, 8> JISRecvParamsPool;
		MemoryPool<Command> commandPool;


		struct PacketFollowedByData { Packet p; unsigned char data[1]; };
		//struct SocketQueryOutput{	RingBufferQueue<JACKIE_INet_Socket*> sockets;};
		MemoryPool <ArraryQueue<JackieINetSocket*>, 8, 4> socketQueryOutput;


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
		LockFreeQueue<JISRecvParams*>* allocRecvParamQ;
		/// shared by recv and send thread to store JISRecvParams PTR that is being dellacated
		LockFreeQueue<JISRecvParams*>* deAllocRecvParamQ;

		LockFreeQueue<Command*> allocCommandQ;
		LockFreeQueue<Command*> deAllocCommandQ;

		/// shared by recv thread and send thread
		LockFreeQueue<Packet*> allocPacketQ;
		/// shared by recv thread and send thread
		LockFreeQueue<Packet*> deAllocPacketQ;
#else

		/// shared between recv and send thread
		ArraryQueue<JISRecvParams*>* allocRecvParamQ;
		/// shared by recv and send thread to  store JISRecvParams PTR that is being dellacated
		ArraryQueue<JISRecvParams*>* deAllocRecvParamQ;

		ArraryQueue<Command*> allocCommandQ;
		ArraryQueue<Command*> deAllocCommandQ;

		/// shared by recv thread and send thread
		ArraryQueue<Packet*> allocPacketQ;
		/// shared by recv thread and send thread
		ArraryQueue<Packet*> deAllocPacketQ;

#endif


		ArraryQueue<JackieAddress, 8> connReqCancelQ;
		JackieSimpleMutex connReqCancelQLock;
		ArraryQueue<ConnectionRequest*, 8> connReqQ;
		JackieSimpleMutex connReqQLock;

		// Threadsafe, and not thread safe
		LockFreeQueue<IPlugin*> pluginListTS;
		ArraryQueue<IPlugin*> pluginListNTS;

		public:
		/// only user thread pushtail into the queue, other threads only read it so no need lock
		ArraryQueue<JackieINetSocket*, 8 > bindedSockets;

		public:
		STATIC_FACTORY_DECLARATIONS(ServerApplication);
		ServerApplication();
		virtual ~ServerApplication();

		void InitIPAddress(void);
		void DeallocBindedSockets(void);
		void ResetSendReceipt(void);
		Packet* GetPacketOnce(void);

		private:
		void ProcessOneRecvParam(JISRecvParams* recvParams);
		bool ProcessOneOfflineRecvParam(JISRecvParams* recvParams,
			bool* isOfflinerecvParams);
		void ProcessBufferedCommand(JISRecvParams* recvParams, JackieStream &updateBitStream);
		void ProcessConnectionRequestCancelQ(void);
		void ProcessAllocJISRecvParamsQ(void);
		void ProcessAllocCommandQ(TimeUS& timeUS, TimeMS& timeMS);
		void ProcessConnectionRequestQ(TimeUS& timeUS, TimeMS& timeMS);
		void AdjustTimestamp(Packet*& incomePacket) const;

		public:
		virtual StartupResult Start(UInt32 maxConnections,
			BindSocket *socketDescriptors,
			UInt32 socketDescriptorCount, Int32 threadPriority = -99999);
		void End(UInt32 blockDuration, unsigned char orderingChannel = 0,
			PacketSendPriority disconnectionNotificationPriority = BUFFERED_THIRDLY_SEND);


		/// Generate and store a unique GUID
		void GenerateGUID(void) { myGuid.g = CreateUniqueRandness(); }
		/// Mac address is a poor solution because 
		/// you can't have multiple connections from the same system
		UInt64 CreateUniqueRandness(void);
		UInt32 GetSystemIndexFromGuid(const JackieGUID& input) const;


		UInt32 MaxConnections() const { return maxConnections; }

		///========================================
		/// @Function CancelConnectionRequest 
		/// @Brief  
		/// Cancel a pending connection attempt
		//  If we are already connected, the connection stays open
		/// @Access  public  
		/// @Param [in] [const JackieAddress & target] 
		///  Which remote server the connection being cancelled to
		/// @Author mengdi[Jackie]
		///========================================
		void CancelConnectionRequest(const JackieAddress& target);

		private:
		void ClearAllCommandQs(void);
		void ClearSocketQueryOutputs(void);
		void ClearAllRecvParamsQs(void);

		/// In multi-threads app and single- thread app,these 3 functions
		/// are called only  by recv thread. the recvStruct will be obtained from 
		/// bufferedDeallocatedRecvParamQueue, so it is thread safe
		/// It is Caller's responsibility to make sure s != 0
		void ReclaimOneJISRecvParams(JISRecvParams *s, UInt32 index);
		void ReclaimAllJISRecvParams(UInt32 deAlloclJISRecvParamsQIndex);
		JISRecvParams * AllocJISRecvParams(UInt32 deAlloclJISRecvParamsQIndex);

		/// send thread will push trail this packet to buffered alloc queue in multi-threads env
		/// for the furture use of recv thread by popout
		Packet* AllocPacket(UInt32 dataSize);
		Packet* AllocPacket(UInt32 dataSize, UInt8 *data);
		/// send thread will take charge of dealloc packet in multi-threads env
		void ReclaimAllPackets(void);

		/// recv thread will reclaim all commands  into command pool in multi-threads env
		void ReclaimAllCommands();

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

		void PacketGoThroughPluginCBs(Packet*& incomePacket);
		void PacketGoThroughPlugins(Packet*& incomePacket);
		void UpdatePlugins(void);

		public:
		/// recv thread will push tail this packet to buffered dealloc queue in multi-threads env
		void ReclaimPacket(Packet *packet);
		/// only recv thread will take charge of alloc packet in multi-threads env
		Command* AllocCommand();
		void PostComand(Command* cmd) { allocCommandQ.PushTail(cmd); };


		///========================================
		/// @Function Connect 
		/// @Brief Connect to a remote host
		/// @Access  public  
		/// @Param [in] [const char * host] Either a dotted IP address or a domain name
		/// @Param [in] [UInt16 port] Which port to connect to on the remote machine.
		/// @Param [in] [const char * pwd]  
		/// 1.Must match the pwd on the server.  
		/// 2.This can be just a password, or can be a stream of data
		/// @Param [in] [UInt32 pwdLen]  The length in bytes
		/// @Param [in] [JACKIE_Public_Key * publicKey]  
		/// @Param [in] [UInt32 ConnectionSocketIndex]  
		/// @Param [in] [UInt32 ConnectionAttemptTimes]  
		/// @Param [in] [UInt32 ConnectionAttemptIntervalMS]  
		/// @Param [in] [TimeMS timeout]  
		/// @Returns [JACKIE_INET::ConnectionAttemptResult]
		/// 1.True on successful initiation. 
		/// 2.False on incorrect parameters, internal error, or too many existing peers
		/// @Remarks None
		/// @Notice
		/// 1.Calling Connect and not calling @SetMaxPassiveConnections() 
		/// acts as a dedicated client.  Calling both acts as a hyrid;
		/// 2.This is a non-blocking connection. So the connection gets successful
		/// when IsConnected() returns true or getting a packet with the type identifier 
		/// ID_CONNECTION_REQUEST_ACCEPTED. 
		/// @Author mengdi[Jackie]
		///========================================
		ConnectionAttemptResult Connect(const char* host, UInt16 port,
			const char *pwd = 0, UInt32 pwdLen = 0, JACKIE_Public_Key *publicKey = 0,
			UInt32 ConnectionSocketIndex = 0, UInt32 ConnectionAttemptTimes = 3,
			UInt32 ConnectionAttemptIntervalMS = 500, TimeMS timeout = 0,
			UInt32 extraData = 0);

		///========================================
		/// function  StopRecvPollingThread 
		/// Access  public  
		/// Param [in] [void]  
		/// Returns void
		/// Remarks: because we use app main thread as recv thread, it 
		/// will never block so no need to use this
		/// author mengdi[Jackie]
		///========================================
		void StopRecvThread(void);
		void StopNetworkUpdateThread(void);
		bool IsActive(void) const { return endThreads == false; }


		const JackieGUID& GetMyGUID(void) const { return myGuid; }
		RemoteEndPoint* GetRemoteEndPoint(const JackieAddress& sa,
			bool neededBySendThread, bool onlyWantActiveEndPoint) const;
		RemoteEndPoint* GetRemoteEndPoint(const JackieAddress& sa)
			const;
		RemoteEndPoint* GetRemoteEndPoint(const JACKIE_INET_Address_GUID_Wrapper&
			senderWrapper, bool neededBySendThread,
			bool onlyWantActiveEndPoint) const;
		RemoteEndPoint* GetRemoteEndPoint(const JackieGUID& senderGUID,
			bool onlyWantActiveEndPoint) const;
		Int32 GetRemoteEndPointIndex(const JackieAddress &sa) const;
		void RefRemoteEndPoint(const JackieAddress &sa, UInt32 index);
		void DeRefRemoteEndPoint(const JackieAddress &sa);


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

