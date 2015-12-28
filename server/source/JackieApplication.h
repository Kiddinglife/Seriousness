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
#include "JackieReliabler.h"
#include "IServerApplication.h"
#include "JackieBits.h"
//#include "SingleProducerConsumer.h"
#include "JackieSimpleMutex.h"
//#include "DS_OrderedList.h"
//#include "RakString.h"
#include "JACKIE_Thread.h"
//#include "RakNetSmartPtr.h"
#include "JackieWaitEvent.h"
#include "CompileFeatures.h"
#include "JACKIE_Atomic.h"
#include "JackieArraryQueue.h"
#include "JackieArray.h"
#include "JackieSPSCQueue.h"
#include "JackieMemoryPool.h"
#include "RandomSeedCreator.h"
#include "JackieINetSocket.h"
#if ENABLE_SECURE_HAND_SHAKE == 1
#include "SecurityHandShake.h"
#endif

using namespace DataStructures;
namespace JACKIE_INET
{
	struct JACKIE_EXPORT JackieIPlugin;

	class JACKIE_EXPORT JackieApplication  //: public IServerApplication
	{
	private:
#if ENABLE_SECURE_HAND_SHAKE == 1
		// Encryption and security
		bool secureIncomingConnectionEnabled, _require_client_public_key;
		char my_public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
		cat::ServerEasyHandshake *serverHandShaker;
		cat::CookieJar *serverCookie;
		bool InitializeClientSecurity(ConnectionRequest *rcs, const char *public_key);
#endif

		Ultils::RandomSeedCreator rnr;
		JackieBits sendBitStream;

		JackieGUID myGuid;
		JackieAddress localIPAddrs[MAX_COUNT_LOCAL_IP_ADDR];
		JackieAddress firstExternalID;


		/// Store the maximum number of peers allowed to connect
		UInt32 maxConnections;
		///Store the maximum incoming connection allowed 
		UInt32 maxIncomingConnections;


		char incomingPassword[256];
		unsigned char incomingPasswordLength;


		/// This is an array of pointers to RemoteEndPoint
		/// This allows us to preallocate the list when starting,
		/// so we don't have to allocate or delete at runtime.
		/// Another benefit is that is lets us add and remove active
		/// players simply by setting systemAddress
		/// and moving elements in the list by copying pointers variables
		/// without affecting running threads, even if they are in the reliability layer
		JackieRemoteSystem* remoteSystemList;

		/// activeSystemList holds a list of pointers and is preallocated to be the same size as 
		/// remoteSystemList. It is updated only by the network thread, but read by both threads
		/// When the isActive member of RemoteEndPoint is set to true or false, that system is 
		/// added to this list of pointers. Threadsafe because RemoteEndPoint is preallocated, 
		/// and the list is only added to, not removed from
		JackieRemoteSystem** activeSystemList;
		UInt32 activeSystemListSize;

		/// Use a hash, with binaryAddress plus port mod length as the index
		JackieRemoteIndex **remoteSystemLookup;

	public:
		bool(*recvHandler)(JISRecvParams*);
		void(*userUpdateThreadPtr)(IServerApplication *, void *);
		void *userUpdateThreadData;
		bool(*incomeDatagramEventHandler)(JISRecvParams *);

	private:
		bool updateCycleIsRunning;
		volatile bool endThreads; ///Set this to true to terminate threads execution 
		volatile bool isNetworkUpdateThreadActive; ///true if the send thread is active. 
		JackieAtomicLong isRecvPollingThreadActive; ///true if the recv thread is active. 
		JackieWaitEvent quitAndDataEvents;


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
		bool limitConnFrequencyOfSameClient;

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
		JackieMemoryPool<JackiePacket> packetPool;
		/// used only by ? thread to alloc RemoteEndPointIndex
		JackieMemoryPool<JackieRemoteIndex> remoteSystemIndexPool;
		/// in single thread app, default JISRecvParams pool is JISRecvParamsPool
		/// in Multi-threads app, used only by recv thread to alloc and dealloc JISRecvParams
		/// via anpothe
		JackieMemoryPool<JISRecvParams>* JISRecvParamsPool;
		//MemoryPool<JISRecvParams, 512, 8> JISRecvParamsPool;
		JackieMemoryPool<Command> commandPool;


		struct PacketFollowedByData { JackiePacket p; unsigned char data[1]; };
		//struct SocketQueryOutput{	RingBufferQueue<JACKIE_INet_Socket*> sockets;};
		JackieMemoryPool <JackieArraryQueue<JackieINetSocket*>, 8, 4> socketQueryOutput;


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
		JackieSPSCQueue<JISRecvParams*>* allocRecvParamQ;
		/// shared by recv and send thread to store JISRecvParams PTR that is being dellacated
		JackieSPSCQueue<JISRecvParams*>* deAllocRecvParamQ;

		JackieSPSCQueue<Command*> allocCommandQ;
		JackieSPSCQueue<Command*> deAllocCommandQ;

		/// shared by recv thread and send thread
		JackieSPSCQueue<JackiePacket*> allocPacketQ;
		/// shared by recv thread and send thread
		JackieSPSCQueue<JackiePacket*> deAllocPacketQ;
#else

		/// shared between recv and send thread
		JackieArraryQueue<JISRecvParams*>* allocRecvParamQ;
		/// shared by recv and send thread to  store JISRecvParams PTR that is being dellacated
		JackieArraryQueue<JISRecvParams*>* deAllocRecvParamQ;

		JackieArraryQueue<Command*> allocCommandQ;
		JackieArraryQueue<Command*> deAllocCommandQ;

		/// shared by recv thread and send thread
		JackieArraryQueue<JackiePacket*> allocPacketQ;
		/// shared by recv thread and send thread
		JackieArraryQueue<JackiePacket*> deAllocPacketQ;

#endif


		JackieArraryQueue<JackieAddress, 8> connReqCancelQ;
		JackieSimpleMutex connReqCancelQLock;
		JackieArraryQueue<ConnectionRequest*, 8> connReqQ;
		JackieSimpleMutex connReqQLock;

		// Threadsafe, and not thread safe
		JackieArray<JackieIPlugin*> pluginListTS;
		JackieArray<JackieIPlugin*> pluginListNTS;

	public:
		/// only user thread pushtail into the queue, other threads only read it so no need lock
		JackieArray<JackieINetSocket*, 8 > bindedSockets;

	public:
		STATIC_FACTORY_DECLARATIONS(JackieApplication);
		JackieApplication();
		virtual ~JackieApplication();

		void InitIPAddress(void);
		void DeallocBindedSockets(void);
		void ResetSendReceipt(void);
		JackiePacket* GetPacketOnce(void);

	private:
		void ProcessOneRecvParam(JISRecvParams* recvParams);
		void IsOfflineRecvParams(JISRecvParams* recvParams,
			bool* isOfflinerecvParams);
		void ProcessBufferedCommand(JISRecvParams* recvParams, JackieBits &updateBitStream);
		void ProcessConnectionRequestCancelQ(void); /// @Done
		void ProcessAllocJISRecvParamsQ(void);
		void ProcessAllocCommandQ(TimeUS& timeUS, TimeMS& timeMS);
		void ProcessConnectionRequestQ(TimeUS& timeUS, TimeMS& timeMS);	/// @Done
		void AdjustTimestamp(JackiePacket*& incomePacket) const;

	public:
		virtual StartupResult Start(BindSocket *socketDescriptors,
			UInt32 maxConnections = 8, UInt32 socketDescriptorCount = 1,
			Int32 threadPriority = -99999);
		void End(UInt32 blockDuration, unsigned char orderingChannel = 0,
			PacketSendPriority disconnectionNotificationPriority = BUFFERED_THIRDLY_SEND);


		/// Generate and store a unique GUID
		void GenerateGUID(void) { myGuid.g = CreateUniqueRandness(); }
		/// Mac address is a poor solution because 
		/// you can't have multiple connections from the same system
		UInt64 CreateUniqueRandness(void);
		UInt32 GetSystemIndexFromGuid(const JackieGUID& input) const;
		const JackieGUID& GetGuidFromSystemAddress(const JackieAddress input) const;

		UInt32 MaxConnections() const { return maxConnections; }
		/// return how many client initiats a connection to us
		UInt32 GetIncomingConnectionsCount(void) const;
		bool CanAcceptIncomingConnection(void) const
		{
			return GetIncomingConnectionsCount() < maxConnections;
		}
		/// to check if this is loop back address of local host
		bool IsLoopbackAddress(const JackieAddressGuidWrapper &systemIdentifier,
			bool matchPort) const;
		/// @Function CancelConnectionRequest 
		/// @Brief  
		/// Cancel a pending connection attempt
		//  If we are already connected, the connection stays open
		/// @Access  public  
		/// @Param [in] [const JackieAddress & target] 
		///  Which remote server the connection being cancelled to
		/// @Author mengdi[Jackie]
		void CancelConnectionRequest(const JackieAddress& target);

		bool GenerateConnectionRequestChallenge(ConnectionRequest *connectionRequest, JackieSHSKey *jackiePublicKey);

		/// Returns the number of IP addresses we have
		inline unsigned int GetLocalIPAddrCount(void)
		{
			if (!IsActive())
			{
				InitIPAddress();
			}
			int i = 0;
			while (localIPAddrs[i] != JACKIE_NULL_ADDRESS)
				i++;
			return i;
		}

		// Returns an IP address at index 0 to GetNumberOfAddresses-1
		// \param[in] index index into the list of IP addresses
		// \return The local IP address at this index
		const char* GetLocalIPAddr(unsigned int index)
		{
			if (!IsActive())
			{
				InitIPAddress();
			}
			static char str[65];
			localIPAddrs[index].ToString(false, str);
			return str;
		}

		/// Sets the password incoming connections must match in the call to Connect
		/// (defaults to none) Pass 0 to passwordData to specify no password
		/// passwd: A data block that incoming connections must match.
		/// This can be just a password, or can be a stream of data.
		/// Specify 0 for no password data
		/// passwdLength: The length in bytes of passwordData
		void SetIncomingConnectionsPasswd(const char passwd[255], int passwdLength)
		{
			//if (passwordDataLength > MAX_OFFLINE_DATA_LENGTH)
			//	passwordDataLength=MAX_OFFLINE_DATA_LENGTH;

			if (passwdLength > 255)
				passwdLength = 255;

			if (passwd == 0)
				passwdLength = 0;

			// Not threadsafe but it's not important enough to lock.  
			// Who is going to change the password a lot during runtime?
			// It won't overflow because @incomingPasswordLength is an unsigned char
			if (passwd != 0 && passwdLength > 0)
				memcpy(incomingPassword, passwd, passwdLength);
			incomingPasswordLength = (unsigned char)passwdLength;
		}

		///  Must be called while offline
		/// If you are allowed to be connected by others,
		/// you must call this or else security will not be enabled for incoming connections.
		/// This feature requires more round trips, bandwidth, and CPU time for the connection
		/// handshake.  x64 builds require under 25% of the CPU time of other builds
		/// Parameters:
		/// publicKey = A pointer to the public key for accepting new connections
		/// privateKey = A pointer to the private key for accepting new connections
		/// If the private keys are 0, then a new key will be generated when this function is called
		/// bRequireClientKey: Should be set to false for most servers.  Allows the server to accept
		/// a public key from connecting clients as a proof of identity but eats twice as much CPU time as a normal connection
		bool EnableSecureIncomingConnections(const char *public_key,
			const char *private_key, bool requireClientPublicKey);

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
		JackiePacket* AllocPacket(UInt32 dataSize);
		JackiePacket* AllocPacket(UInt32 dataSize, UInt8 *data);
		/// send thread will take charge of dealloc packet in multi-threads env
		void ReclaimAllPackets(void);

		/// recv thread will reclaim all commands  into command pool in multi-threads env
		void ReclaimAllCommands();

		void RunNetworkUpdateCycleOnce(void);
		void RunRecvCycleOnce(UInt32 in = 0);
		JackiePacket* RunGetPacketCycleOnce(void);

		/// function  CreateRecvPollingThread 
		/// Access  private  
		/// Param [in] [int threadPriority]
		/// Returns int
		/// Remarks this function is not used because because , a thread 
		/// that calls the Start() func plays recv thread
		/// author mengdi[Jackie]
		int CreateRecvPollingThread(int threadPriority, UInt32 index);
		int CreateNetworkUpdateThread(int threadPriority);

		void PacketGoThroughPluginCBs(JackiePacket*& incomePacket);
		void PacketGoThroughPlugins(JackiePacket*& incomePacket);
		void UpdatePlugins(void);

	public:
		/// recv thread will push tail this packet to buffered dealloc queue in multi-threads env
		void ReclaimPacket(JackiePacket *packet);
		/// only recv thread will take charge of alloc packet in multi-threads env
		Command* AllocCommand();
		void PostComand(Command* cmd) { allocCommandQ.PushTail(cmd); };

		/// @Function Connect 
		/// @Brief Connect to a remote host
		/// @Access  public  
		/// @Param [in] [const char * host] Either a dotted IP address or a domain name
		/// @Param [in] [UInt16 port] Which port to connect to on the remote machine.
		/// @Param [in] [const char * passwd]  
		/// 1.Must match the passwd on the server.  
		/// 2.This can be just a password, or can be a stream of data
		/// @Param [in] [UInt32 passwdLength]  The length in bytes
		/// @Param [in] [JACKIE_Public_Key * jackiePublicKey]  
		/// @Param [in] [UInt32 localSocketIndex]  
		/// @Param [in] [UInt32 attemptTimes]  
		/// @Param [in] [UInt32 attemptIntervalMS]  
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
		ConnectionAttemptResult Connect(const char* host, UInt16 port,
			const char *passwd = 0, UInt32 passwdLength = 0,
			JackieSHSKey *jackiePublicKey = 0,
			UInt32 localSocketIndex = 0, UInt32 attemptTimes = 6,
			UInt32 attemptIntervalMS = 1000, TimeMS timeout = 0,
			UInt32 extraData = 0);


		/// function  StopRecvPollingThread 
		/// Access  public  
		/// Param [in] [void]  
		/// Returns void
		/// Remarks: because we use app main thread as recv thread, it 
		/// will never block so no need to use this
		/// author mengdi[Jackie]
		void StopRecvThread(void);
		void StopNetworkUpdateThread(void);
		bool IsActive(void) const { return endThreads == false; }


		const JackieGUID& GetMyGUID(void) const { return myGuid; }
		JackieRemoteSystem* GetRemoteSystem(const JackieAddress& sa,
			bool neededBySendThread, bool onlyWantActiveEndPoint) const;
		JackieRemoteSystem* GetRemoteSystem(const JackieAddress& sa)
			const;
		JackieRemoteSystem* GetRemoteSystem(const JackieAddressGuidWrapper&
			senderWrapper, bool neededBySendThread,
			bool onlyWantActiveEndPoint) const;
		JackieRemoteSystem* GetRemoteSystem(const JackieGUID& senderGUID,
			bool onlyWantActiveEndPoint) const;
		Int32 GetRemoteSystemIndex(const JackieAddress &sa) const;
		void RefRemoteEndPoint(const JackieAddress &sa, UInt32 index);
		void DeRefRemoteSystem(const JackieAddress &sa);

		/// \brief Given \a systemAddress, returns its index into remoteSystemList.
		/// \details Values range from 0 to the maximum number of players allowed-1.
		/// This includes systems which were formerly connected, but are now not connected.
		/// \param[in] systemAddress The SystemAddress we are referring to
		/// \return The index of this SystemAddress or -1 on system not found.
		Int32 GetRemoteSystemIndexGeneral(const JackieAddress& systemAddress,
			bool calledFromNetworkThread = false) const;
		Int32 GetRemoteSystemIndexGeneral(const JackieGUID& jackieGuid,
			bool calledFromNetworkThread = false) const;

		bool SendRightNow(TimeUS currentTime, bool useCallerAlloc,
			Command* bufferedCommand);


		void CloseConnectionInternally(bool sendDisconnectionNotification,
			bool performImmediate, Command* bufferedCommand);

		/// \brief Attaches a Plugin interface to an instance of the base class (RakPeer or PacketizedTCP) to run code automatically on message receipt in the Receive call.
		/// If the plugin returns false from PluginInterface::UsesReliabilityLayer(), which is the case for all plugins except PacketLogger, you can call AttachPlugin() and DetachPlugin() for this plugin while RakPeer is active.
		/// \param[in] messageHandler Pointer to the plugin to attach.
		void AttachOnePlugin(JackieIPlugin *plugin);
		bool SendImmediate(ReliableSendParams& sendParams);

		void AddToActiveSystemList(UInt32 index2use);
		bool IsInSecurityExceptionList(JackieAddress& jackieAddr);
		void Add2RemoteSystemList(JISRecvParams* recvParams, JackieRemoteSystem*& free_rs, bool& thisIPFloodsConnRequest, UInt32 mtu, JackieAddress& recvivedBoundAddrFromClient, JackieGUID& guid,
			bool clientSecureRequiredbyServer);

		friend JACKIE_THREAD_DECLARATION(RunNetworkUpdateCycleLoop);
		friend JACKIE_THREAD_DECLARATION(RunRecvCycleLoop);
		friend JACKIE_THREAD_DECLARATION(UDTConnect);
	};

}

#endif 

