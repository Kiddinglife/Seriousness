/*!
 * \file ServerApplication.h
 * \author mengdi
 * \date Oct 18, 2015
 */

#ifndef SERVERAPPLICATION_H_
#define SERVERAPPLICATION_H_

#include "DLLExport.h"
#include "ReliabilityMgr.h"
#include "IServerApplication.h"
#include "BitStream.h"
//#include "SingleProducerConsumer.h"
#include "JACKIE_Simple_Mutex.h"
//#include "DS_OrderedList.h"
//#include "RakString.h"
#include "JACKIE_Thread.h"
//#include "RakNetSmartPtr.h"
#include "MemPoolAllocRingBufferQueue.h"
#include "ThreadConditionSignalEvent.h"
#include "CompileFeatures.h"
//#include "SecureHandshake.h"
#include "JACKIE_Atomic.h"
#include "RingBufferQueue.h"
#include "LockFreeQueue.h"
#include "MemoryPool.h"
#include "IPlugin.h"

using namespace DataStructures;

namespace JACKIE_INET
{
	class JACKIE_EXPORT ServerApplication : public IServerApplication
	{
		//private:
		public:
		STATIC_FACTORY_DECLARATIONS(ServerApplication);

		/////////////////////////////////////////////////////////////////////////////////////////////////////
#if LIBCAT_SECURITY == 1
		// Encryption and security
		bool _using_security, _require_client_public_key;
		char my_public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
		cat::ServerEasyHandshake *_server_handshake;
		cat::CookieJar *_cookie_jar;
		bool InitializeClientSecurity(RequestedConnectionStruct *rcs, const char *public_key);
#endif
		///////////////////////////////////////////////////////////////////////////////////////////////////////


		////////////////////////////////////////////////////////////////////////////
		JACKIE_INet_GUID myGuid;
		JACKIE_INET_Address IPAddress[MAX_COUNT_LOCAL_IP_ADDR];
		JACKIE_INET_Address firstExternalID;
		/////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////// CONNECTIONS //////////////////////////////
		/// Store the maximum number of peers allowed to connect
		unsigned int maxConnections;
		///Store the maximum incoming connection allowed 
		unsigned int maxIncomeConnections;
		//////////////////////////////////////////////////////////////////////////////////////


		////////////////////////////////// PASSWORD ////////////////////////////////
		char incomingPassword[256];
		unsigned char incomingPasswordLength;
		//////////////////////////////////////////////////////////////////////////////////


		////////////////////////////////////// RemoteEndPoint //////////////////////////////////////////
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
		/////////////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////// FUNC PTR & THREAD //////////////////////////
		bool(*recvHandler)( JISRecvParams* );
		void(*userUpdateThreadPtr)( IServerApplication *, void * );
		void *userUpdateThreadData;
		bool(*incomeDatagramEventHandler)( JISRecvParams * );
		bool updateCycleIsRunning;
		volatile bool endSendRecvThreads; ///Set this to true to terminate threads execution 
		volatile bool isSendPollingThreadActive; ///true if the send thread is active. 
		volatile bool isRecvPollingThreadActive; ///true if the recv thread is active. 
		ThreadConditionSignalEvent quitAndDataEvents;
		///////////////////////////////////////////////////////////////////////////////////////////


		////////////////////////////////////////// STATS ///////////////////////////////////////////////
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
		////////////////////////////////////////////////////////////////////////////////////////////////////


		///////////////////////////////// SEND RECIEPT SERIAL ////////////////////////////////
		/// This is used to return a number to the user 
		/// when they call Send identifying the message
		/// This number will be returned back with ID_SND_RECEIPT_ACKED
		/// or ID_SND_RECEIPT_LOSS and is only returned with the reliability 
		/// types that DOES NOT contain 'NOT' in the name
		JACKIE_Simple_Mutex sendReceiptSerialMutex;
		UInt32 sendReceiptSerial;
		//////////////////////////////////////////////////////////////////////////////////////////////


		////////////////////////////////// MEMPOOLS ///////////////////////////////////////////
		/// in single thread app, default packet pool is recvThreadPacketAllocationPool
		/// in Multi-threads app, used only by recv thread to alloc packet
		MemoryPool<Packet, 512, 8> recvThreadPacketAllocationPool;
		/// in Multi-threads app, used only by send thread to alloc packet
		MemoryPool<Packet, 512, 8> sendThreadPacketAllocationPool;
		/// used only by ? thread to alloc RemoteEndPointIndex
		MemoryPool<RemoteEndPointIndex, 1024, 8> remoteSystemIndexPool;
		/// in single thread app, default JISRecvParams pool is JISRecvParamsPool
		/// in Multi-threads app, used only by recv thread to alloc and dealloc JISRecvParams
		/// via anpothe
		MemoryPool<JISRecvParams, 512, 8> JISRecvParamsPool;
		///////////////////////////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////// QUEUES /////////////////////////////////////////
		struct PacketFollowedByData { Packet p; unsigned char data[1]; };
		//struct SocketQueryOutput{	RingBufferQueue<JACKIE_INet_Socket*> sockets;};
		MemPoolAllocRingBufferQueueCtorDtor <RingBufferQueue<JACKIE_INet_Socket*>,
			8, 4, 8> socketQueryOutput;
		/// used only by send thread to alloc BufferedCommand
		MemPoolAllocRingBufferQueue<BufferedCommand, 512, 8, 512>
			bufferedCommands;

#if USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
		/// it works in this way:
		/// recv thread uses pool to allocate JISRecvParams* ptr and push it to  
		/// bufferedAllocatedRecvParamQueue while send thread pops ptr out from 
		/// bufferedAllocatedRecvParamQueue and use it do something, when finished, 
		/// send thread . then push this ptr into bufferedDeallocatedRecvParamQueue
		/// then recv thread will pop this ptr out from bufferedDeallocatedRecvParamQueue
		/// and use this ptr to deallocate. In this way, we do not need locks the pool but using 
		/// lockfree queue
		/// shared  between recv and send thread to store allocated JISRecvParams PTR
		LockFreeQueue<JISRecvParams*, SERVER_QUEUE_PTR_SIZE> bufferedAllocatedRecvParamQueue;
		/// shared between recv and send thread to 
		/// store JISRecvParams PTR that is being dellacated
		LockFreeQueue<JISRecvParams*, SERVER_QUEUE_PTR_SIZE> bufferedDeallocatedRecvParamQueue;
		/// shared by recv thread and send thread
		LockFreeQueue<Packet*, SERVER_QUEUE_PTR_SIZE>
			bufferedPacketsQueue;
#else
		/// shared between recv and send thread
		RingBufferQueue<JISRecvParams*, SERVER_QUEUE_PTR_SIZE> bufferedRecvParamQueue;
		/// shared between recv and send thread to 
		/// store JISRecvParams PTR that is being dellacated
		RingBufferQueue<JISRecvParams*, SERVER_QUEUE_PTR_SIZE> bufferedDeallocatedRecvParamQueue;
		/// shared by user thread and send thread
		RingBufferQueue<Packet*, SERVER_QUEUE_PTR_SIZE>
			bufferedPacketsQueue;
#endif
		/// only user thread pushtail into the queue, other threads only read it so no need lock
		RingBufferQueue<JACKIE_INet_Socket*, 8 > JISList;
		// Threadsafe, and not thread safe
		LockFreeQueue<IPlugin*, 16> pluginListTS;
		RingBufferQueue<IPlugin*, 16> pluginListNTS;
		////////////////////////////////////////////////////////////////////////////////////////////////


		public:
		ServerApplication();
		virtual ~ServerApplication();

		void InitIPAddress(void);
		void DeallocJISList(void);
		void ResetSendReceipt(void);

		Packet* FetchOnePacket(void);
		void ProcessOneRecvParam(JISRecvParams* recvParams, BitStream
			&updateBitStream);
		bool ProcessOneOfflineRecvParam(JISRecvParams* recvParams,
			bool* isOfflinerecvParams);
		void ProcessBufferedCommand(JISRecvParams* recvParams,
			BitStream &updateBitStream);
		void AdjustTimestamp(Packet*& incomePacket) const;

		////////////////////////////////////// PUBLIC INTERFACES /////////////////////////////////////////
		virtual StartupResult Start(UInt32 maxConnections,
			JACKIE_LOCAL_SOCKET *socketDescriptors,
			UInt32 socketDescriptorCount, Int32 threadPriority = -99999) override;
		void End(unsigned int blockDuration, unsigned char orderingChannel = 0,
			PacketSendPriority disconnectionNotificationPriority = BUFFERED_THIRDLY_SEND);
		////////////////////////////////////////////////////////////////////////////////////////////////////////


		///////////////////////////////// JISRecv DE/ALLOC ///////////////////////////////////
		/// In multi-threads app and single- thread app,these 3 functions
		/// are called only  by recv thread. the recvStruct will be obtained from 
		/// bufferedDeallocatedRecvParamQueue, so it is thread safe
		virtual void OnJISRecv(JISRecvParams *recvStruct) override;
		virtual void ReclaimJISRecvParams(JISRecvParams *s) override;
		virtual JISRecvParams * AllocJISRecvParams() override;
		//////////////////////////////////////////////////////////////////////////////////////////


		///////////////////////////////// GUID /////////////////////////////////
		// Generate and store a unique GUID
		void GenerateGUID(void) { myGuid.g = Get64BitUniqueRandomNumber(); }
		/// Mac address is a poor solution because 
		/// you can't have multiple connections from the same system
		UInt64 Get64BitUniqueRandomNumber(void);
		unsigned int GetSystemIndexFromGuid(const JACKIE_INet_GUID& input) const;
		//////////////////////////////////////////////////////////////////////////


		//////////////////////////////// CLEAR QUEUE /////////////////////////////////
		void ClearBufferedCommands(void);
		void ClearSocketQueryOutputs(void);
		void ClearBufferedAllocatedRecvParamQueue(void);
		void ClearBufferedDeAllocatedRecvParamQueue(void);
		////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////// PACKET DE/ALLOC /////////////////////////////////
		/// recv thread type = 0, send thread type = 1
		Packet* AllocPacket(unsigned int dataSize, ThreadType threadType);
		Packet* AllocPacket(unsigned dataSize, unsigned  char *data, ThreadType threadType);
		/// user thread will take charge of dealloc packet in multi-threads env
		void DeallocatePacket(Packet *packet);
		///////////////////////////////////////////////////////////////////////////////////////////


		///////////////////////// RECV SEND ONCE ////////////////////////////////
		bool RunSendCycleOnce(BitStream &updateBitStream);
		void RunRecvCycleOnce(void);
		///////////////////////////////////////////////////////////////////////////////


		///////////////////////////////// Threads /////////////////////////////////
		int CreateRecvPollingThread(int threadPriority);
		int CreateSendPollingThread(int threadPriority);
		void BlockOnStopRecvPollingThread(JISBerkley* sock);
		void StopRecvPollingThread(void);
		void StopSendPollingThread(void);
		bool IsActive(void) const { return endSendRecvThreads == false; }
		//////////////////////////////////////////////////////////////////////////


		///////////////////////////////// PLUGINS //////////////////////////////////
		void PacketGoThroughPluginCBs(Packet*& incomePacket);
		void PacketGoThroughPlugins(Packet*& incomePacket);
		void UpdatePlugins(void);
		///////////////////////////////////////////////////////////////////////////////


		/////////////////////////////SETERS/////////////////////////////////////
		///
		//////////////////////////////////////////////////////////////////////////


		/////////////////////////////GETTERS/////////////////////////////////////
		const JACKIE_INet_GUID& GetMyGUID(void) const { return myGuid; }
		RemoteEndPoint* GetRemoteEndPoint(const JACKIE_INET_Address& senderAddress,
			bool neededBySendThread, bool onlyWantActiveEndPoint) const;
		Int32 GetRemoteEndPointIndex(const JACKIE_INET_Address &sa) const;
		//////////////////////////////////////////////////////////////////////////


		////////////////////////////// FRIEND FUNCS ////////////////////////
		friend JACKIE_THREAD_DECLARATION(RunSendCycleLoop);
		friend JACKIE_THREAD_DECLARATION(RunRecvCycleLoop);
		friend JACKIE_THREAD_DECLARATION(UDTConnect);
		//////////////////////////////////////////////////////////////////////////
	};
}

#endif 

