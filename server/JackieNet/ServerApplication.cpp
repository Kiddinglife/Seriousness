#include "ServerApplication.h"
#include "WSAStartupSingleton.h"
#include "EasyLog.h"
#include "MessageID.h"

#if !defined ( __APPLE__ ) && !defined ( __APPLE_CC__ )
#include <stdlib.h> // malloc
#endif

#ifdef CAT_AUDIT
#define CAT_AUDIT_PRINTF(...) printf(__VA_ARGS__)
#else
#define CAT_AUDIT_PRINTF(...)
#endif

namespace JACKIE_INET
{
	////////////////////////////////////////////////// STATICS /////////////////////////////////////
	static const UInt32 RemoteEndPointLookupHashMutiple = 8;
	static const int mtuSizesCount = 3;
	static const int mtuSizes[mtuSizesCount] = { MAXIMUM_MTU_SIZE, 1200, 576 };
	/// I set this because I limit ID_CONNECTION_REQUEST to 512 bytes, 
	/// and the password is appended to that *incomePacket.
	static const UInt32 MAX_OFFLINE_DATA_LENGTH = 400;
	/// Used to distinguish between offline messages with data, 
	/// and messages from the reliability layer. Should be different 
	/// than any message that could result from messages from the reliability layer
	/// Make sure highest bit is 0, so isValid in DatagramHeaderFormat is false
	static const unsigned char OFFLINE_MESSAGE_DATA_ID[16] =
	{
		0x00, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0xFE,
		0xFD, 0xFD, 0xFD, 0xFD, 0x12, 0x34, 0x56, 0x78
	};
	//////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////
	ServerApplication::ServerApplication() : sendBitStream(MAXIMUM_MTU_SIZE
#if LIBCAT_SECURITY==1
		+ cat::AuthenticatedEncryption::OVERHEAD_BYTES
#endif
		)
	{
#if LIBCAT_SECURITY == 1
		// Encryption and security
		CAT_AUDIT_PRINTF("AUDIT: Initializing ServerApplication security flags: using_security = false, server_handshake = null, cookie_jar = null\n");
		_using_security = false;
		_server_handshake = 0;
		_cookie_jar = 0;
#endif

		// Dummy call to PacketLogger to ensure it's included in exported symbols.
		//PacketLogger::BaseIDTOString(0);
		//StringCompressor::AddReference();
		//RakNet::StringTable::AddReference();
		WSAStartupSingleton::AddRef();

		defaultMTUSize = mtuSizes[mtuSizesCount - 1];
		trackFrequencyTable = false;
		maxPassiveConnections = maxConnections = 0;
		bytesSentPerSecond = bytesReceivedPerSecond = 0;

		remoteSystemList = 0;
		remoteSystemLookup = 0;
		activeSystemList = 0;
		activeSystemListSize = 0;

		recvHandler = 0;
		userUpdateThreadPtr = 0;
		userUpdateThreadData = 0;
		incomeDatagramEventHandler = 0;

		allowInternalRouting = false;
		allowConnectionResponseIPMigration = false;
		incomingPasswordLength = 0;
		splitMessageProgressInterval = 0;
		unreliableTimeout = 1000;
		maxOutgoingBPS = 0;

		myGuid = JACKIE_NULL_GUID;
		firstExternalID = JACKIE_NULL_ADDRESS;

		for (UInt32 index = 0; index < MAX_COUNT_LOCAL_IP_ADDR; index++)
		{
			IPAddress[index] = JACKIE_NULL_ADDRESS;
		}

#ifdef _DEBUG
		// Wait longer to disconnect in debug so I don't get disconnected while tracing
		defaultTimeoutTime = 30000;
		_packetloss = 0.0;
		_minExtraPing = 0;
		_extraPingVariance = 0;
#else
		defaultTimeoutTime = 10000;
#endif

#if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
		occasionalPing = true;
#else
		occasionalPing = false;
#endif

		limitConnectionFrequencyFromTheSameIP = false;

#if USE_SINGLE_THREAD == 0
		quitAndDataEvents.Init();
#endif

		GenerateGUID();
		ResetSendReceipt();
	}
	ServerApplication::~ServerApplication()
	{
		JACKIE_INET::OP_DELETE_ARRAY(JISRecvParamsPool, TRACE_FILE_AND_LINE_);
	}


	JACKIE_INET::StartupResult ServerApplication::Start(UInt32 maxConn,
		BindSocket *bindLocalSockets,
		UInt32 bindLocalSocketsCount,
		Int32 threadPriority /*= -99999*/)
	{
		if (IsActive()) return StartupResult::ALREADY_STARTED;

		// If getting the guid failed in the constructor, try again
		if (myGuid.g == 0)
		{
			GenerateGUID();
			if (myGuid.g == 0) return StartupResult::COULD_NOT_GENERATE_GUID;
		}

		if (myGuid == JACKIE_NULL_GUID)
		{
			rnr.SeedMT((UInt32)((myGuid.g >> 32) ^ myGuid.g));
			myGuid.g = rnr.RandomMT();
		}

		if (threadPriority == -99999)
		{
#if  defined(_WIN32)
			threadPriority = 0;
#else
			threadPriority = 1000;
#endif
		}

		InitIPAddress();

		assert(bindLocalSockets && bindLocalSocketsCount >= 1);
		if (bindLocalSockets == 0 || bindLocalSocketsCount < 1)
			return INVALID_JACKIE_LOCAL_SOCKET;

		assert(maxConn > 0); if (maxConn <= 0) return INVALID_MAX_CONNECTIONS;

		/// Start to bind given sockets 
		UInt32 index;
		JackieINetSocket* sock;
		JISBerkleyBindParams berkleyBindParams;
		JISBindResult bindResult;
		DeallocBindedSockets();
		for (index = 0; index < bindLocalSocketsCount; index++)
		{
			do { sock = JISAllocator::AllocJIS(); } while (sock == 0);
			DCHECK_EQ(bindedSockets.PushTail(sock), true);

#if defined(__native_client__)
			NativeClientBindParameters ncbp;
			RNS2_NativeClient * nativeClientSocket = (RNS2_NativeClient*) r2;
			ncbp.eventHandler = this;
			ncbp.forceHostAddress = (char*) bindLocalSockets[index].hostAddress;
			ncbp.is_ipv6 = bindLocalSockets[index].socketFamily == AF_INET6;
			ncbp.nativeClientInstance = bindLocalSockets[index].chromeInstance;
			ncbp.port = bindLocalSockets[index].port;
			nativeClientSocket->Bind(&ncbp, _FILE_AND_LINE_);
#elif defined(WINDOWS_STORE_RT)
			RNS2BindResult br;
			( (RNS2_WindowsStore8*) r2 )->SetRecvEventHandler(this);
			br = ( (RNS2_WindowsStore8*) r2 )->Bind(ref new Platform::String());
			if( br != BR_SUCCESS )
			{
				RakNetSocket2Allocator::DeallocRNS2(r2);
				DerefAllSockets();
				return SOCKET_FAILED_TO_BIND;
			}
#else
			if (sock->IsBerkleySocket())
			{
				berkleyBindParams.port = bindLocalSockets[index].port;
				berkleyBindParams.hostAddress = (char*)bindLocalSockets[index].hostAddress;
				berkleyBindParams.addressFamily = bindLocalSockets[index].socketFamily;
				berkleyBindParams.type = SOCK_DGRAM;
				berkleyBindParams.protocol = bindLocalSockets[index].extraSocketOptions;
				berkleyBindParams.isBroadcast = true;
				berkleyBindParams.setIPHdrIncl = false;
				berkleyBindParams.doNotFragment = false;
				berkleyBindParams.pollingThreadPriority = threadPriority;
				berkleyBindParams.eventHandler = this;
				berkleyBindParams.remotePortJackieNetWasStartedOn_PS3_PS4_PSP2 =
					bindLocalSockets[index].remotePortWasStartedOn_PS3_PSP2;

#if USE_SINGLE_THREAD == 0
				/// multi-threads app can use either non-blobk or blobk socket
				/// false = blobking, true = non-blocking
				berkleyBindParams.isBlocKing = bindLocalSockets[index].blockingSocket;
#else
				///  single thread app will always use non-blobking socket 
				berkleyBindParams.isBlocKing = true;
#endif

				bindResult = ((JISBerkley*)sock)->Bind(&berkleyBindParams,
					TRACE_FILE_AND_LINE_);

				if (
#if NET_SUPPORT_IPV6 ==0
					bindLocalSockets[index].socketFamily != AF_INET ||
#endif
					bindResult == JISBindResult_REQUIRES_NET_SUPPORT_IPV6_DEFINED)
				{
					JISAllocator::DeallocJIS(sock);
					DeallocBindedSockets();
					JERROR << "Bind Failed (REQUIRES_NET_SUPPORT_IPV6_DEFINED) ! ";
					return SOCKET_FAMILY_NOT_SUPPORTED;
				}

				switch (bindResult)
				{
				case JISBindResult_FAILED_BIND_SOCKET:
					DeallocBindedSockets();
					JERROR << "Bind Failed (FAILED_BIND_SOCKET) ! ";
					return SOCKET_PORT_ALREADY_IN_USE;
					break;

				case JISBindResult_FAILED_SEND_RECV_TEST:
					DeallocBindedSockets();
					JERROR << "Bind Failed (FAILED_SEND_RECV_TEST) ! ";
					return SOCKET_FAILED_TEST_SEND_RECV;
					break;

				default:
					assert(bindResult == JISBindResult_SUCCESS);
					JDEBUG << "Bind [" << sock->GetBoundAddress().ToString() << "] Successfully";
					sock->SetUserConnectionSocketIndex(index);
					break;
				}
			}
			else
			{
				JWARNING << "Bind Failed (@TO-DO UNKNOWN BERKELY SOCKET) ! ";
				assert("@TO-DO UNKNOWN BERKELY SOCKET" && 0);
			}
#endif
		}

		assert(bindedSockets.Size() == bindLocalSocketsCount);

		/// after binding, assign IPAddress port number
#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
		if (bindedSockets[0]->IsBerkleySocket())
		{
			for (index = 0; index < MAX_COUNT_LOCAL_IP_ADDR; index++)
			{
				if (IPAddress[index] == JACKIE_NULL_ADDRESS) break;
				IPAddress[index].SetPortHostOrder(((JISBerkley*)bindedSockets[0])->GetBoundAddress().GetPortHostOrder());
			}
		}
#endif

		JISRecvParamsPool = JACKIE_INET::OP_NEW_ARRAY < MemoryPool < JISRecvParams >>(bindedSockets.Size(), TRACE_FILE_AND_LINE_);
#if USE_SINGLE_THREAD == 0
		deAllocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < LockFreeQueue <
			JISRecvParams* >> (bindedSockets.Size(), TRACE_FILE_AND_LINE_);
		allocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < LockFreeQueue <
			JISRecvParams* >> (bindedSockets.Size(), TRACE_FILE_AND_LINE_);
#else
		deAllocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < ArraryQueue
			< JISRecvParams* >> ( bindedSockets.Size(), TRACE_FILE_AND_LINE_ );
		allocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < ArraryQueue <
			JISRecvParams* >> ( bindedSockets.Size(), TRACE_FILE_AND_LINE_ );
#endif

		/// setup connections list
		if (maxConnections == 0)
		{
			// Don't allow more incoming connections than we have peers.
			if (maxPassiveConnections > maxConn) maxPassiveConnections = maxConn;
			maxConnections = maxConn;

			remoteSystemList = JACKIE_INET::OP_NEW_ARRAY<RemoteEndPoint>(maxConnections, TRACE_FILE_AND_LINE_);

			// All entries in activeSystemList have valid pointers all the time.
			activeSystemList = JACKIE_INET::OP_NEW_ARRAY<RemoteEndPoint*>(maxConnections, TRACE_FILE_AND_LINE_);

			index = maxConnections*RemoteEndPointLookupHashMutiple;
			remoteSystemLookup = JACKIE_INET::OP_NEW_ARRAY<RemoteEndPointIndex*>(index, TRACE_FILE_AND_LINE_);
			memset((void**)remoteSystemLookup, 0, index*sizeof(RemoteEndPointIndex*));

			for (index = 0; index < maxConnections; index++)
			{
				// remoteSystemList in Single thread
				remoteSystemList[index].isActive = false;
				remoteSystemList[index].systemAddress = JACKIE_NULL_ADDRESS;
				remoteSystemList[index].guid = JACKIE_NULL_GUID;
				remoteSystemList[index].myExternalSystemAddress = JACKIE_NULL_ADDRESS;
				remoteSystemList[index].status = RemoteEndPoint::NO_ACTION;
				remoteSystemList[index].MTUSize = defaultMTUSize;
				remoteSystemList[index].remoteSystemIndex = (SystemIndex)index;

#ifdef _DEBUG
				remoteSystemList[index].reliabilityLayer.ApplyNetworkSimulator(_packetloss, _minExtraPing, _extraPingVariance);
#endif
				activeSystemList[index] = &remoteSystemList[index];
			}
		}


		/// Setup Plugins 
		for (index = 0; index < pluginListTS.Size(); index++)
		{
			pluginListTS[index]->OnRakPeerStartup();
		}

		for (index = 0; index < pluginListNTS.Size(); index++)
		{
			pluginListNTS[index]->OnRakPeerStartup();
		}


		///  setup thread things
		endThreads = true;
		isNetworkUpdateThreadActive = false;
		if (endThreads)
		{
			ClearAllCommandQs();
			ClearSocketQueryOutputs();
			ClearAllRecvParamsQs();

			firstExternalID = JACKIE_NULL_ADDRESS;
			updateCycleIsRunning = false;
			endThreads = false;

			/// Create recv threads
#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
#if USE_SINGLE_THREAD == 0
			/// this will create @bindLocalSocketsCount number of of recv threads 
			/// That is if you have two NICs, will create two recv threads to handle
			/// each of socket
			for (index = 0; index < bindLocalSocketsCount; index++)
			{
				if (bindedSockets[index]->IsBerkleySocket())
				{
					if (CreateRecvPollingThread(threadPriority, index) != 0)
					{
						End(0);
						return FAILED_TO_CREATE_RECV_THREAD;
					}
				}
			}

			/// Wait for the threads to activate. When they are active they will set these variables to true
			while (!isRecvPollingThreadActive.GetValue()) JackieSleep(10);
#else
			/// we handle recv in this thread, that is we only have two threads in the app this recv thread and th other send thread
			isRecvPollingThreadActive.Increment();
#endif
#endif

			/// use another thread to charge of sending
			if (!isNetworkUpdateThreadActive)
			{
#if USE_SINGLE_THREAD == 0
				if (CreateNetworkUpdateThread(threadPriority) != 0)
				{
					End(0);
					JERROR << "ServerApplication::Start() Failed (FAILED_TO_CREATE_SEND_THREAD) ! ";
					return FAILED_TO_CREATE_NETWORK_UPDATE_THREAD;
				}
				/// Wait for the threads to activate. When they are active they will set these variables to true
				while (!isNetworkUpdateThreadActive) JackieSleep(10);
#else
				/// we only have one thread to handle recv and send so just simply set it to true
				isNetworkUpdateThreadActive = true;
#endif
			}
		}

		//#ifdef USE_THREADED_SEND
		//		RakNet::SendToThread::AddRef();
		//#endif

		JDEBUG << "Startup Application Succeeds....";
		return ALREADY_STARTED;
	}

	void ServerApplication::End(UInt32 blockDuration,
		unsigned char orderingChannel,
		PacketSendPriority disconnectionNotificationPriority)
	{

	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	inline void ServerApplication::ReclaimOneJISRecvParams(JISRecvParams *s, UInt32 index)
	{
		JDEBUG << "Network Thread Reclaims One JISRecvParams";
		DCHECK_EQ(deAllocRecvParamQ[index].PushTail(s), true);
	}
	void ServerApplication::ReclaimAllJISRecvParams(UInt32 Index)
	{
		JDEBUG << "Recv thread " << Index << " Reclaim All JISRecvParams";

		JISRecvParams* recvParams = 0;
		for (UInt32 index = 0; index < deAllocRecvParamQ[Index].Size(); index++)
		{
			DCHECK_EQ(deAllocRecvParamQ[Index].PopHead(recvParams), true);
			JISRecvParamsPool[Index].Reclaim(recvParams);
		}
	}
	inline JISRecvParams * ServerApplication::AllocJISRecvParams(UInt32 Index)
	{
		JDEBUG << "Recv Thread" << Index << " Alloc An JISRecvParams";
		JISRecvParams* ptr = 0;
		do { ptr = JISRecvParamsPool[Index].Allocate(); } while (ptr == 0);
		return ptr;
	}
	void ServerApplication::ClearAllRecvParamsQs()
	{
		for (UInt32 index = 0; index < bindedSockets.Size(); index++)
		{
			JISRecvParams *recvParams = 0;
			for (UInt32 i = 0; i < allocRecvParamQ[index].Size(); i++)
			{
				DCHECK_EQ(allocRecvParamQ[index].PopHead(recvParams), true);
				if (recvParams->data != 0) jackieFree_Ex(recvParams->data, TRACE_FILE_AND_LINE_);
				JISRecvParamsPool[index].Reclaim(recvParams);
			}
			for (UInt32 i = 0; i < deAllocRecvParamQ[index].Size(); i++)
			{
				DCHECK_EQ(deAllocRecvParamQ[index].PopHead(recvParams), true);
				if (recvParams->data != 0) jackieFree_Ex(recvParams->data, TRACE_FILE_AND_LINE_);
				JISRecvParamsPool[index].Reclaim(recvParams);
			}
			allocRecvParamQ[index].Clear();
			deAllocRecvParamQ[index].Clear();
			JISRecvParamsPool[index].Clear();
		}
	}


	void ServerApplication::ReclaimAllCommands()
	{
		JDEBUG << "User Thread Reclaims All Commands";

		Command* bufferedCommand = 0;
		for (UInt32 index = 0; index < deAllocCommandQ.Size(); index++)
		{
			DCHECK_EQ(
				deAllocCommandQ.PopHead(bufferedCommand),
				true);
			DCHECK_NOTNULL(bufferedCommand);
			commandPool.Reclaim(bufferedCommand);
		}
	}
	Command* ServerApplication::AllocCommand()
	{
		JDEBUG << "User Thread Alloc An Command";
		Command* ptr = 0;
		do { ptr = commandPool.Allocate(); } while (ptr == 0);
		return ptr;
	}
	void ServerApplication::ClearAllCommandQs(void)
	{
		Command *bcs = 0;

		/// first reclaim the elem in 
		for (UInt32 i = 0; i < allocCommandQ.Size(); i++)
		{
			DCHECK_EQ(allocCommandQ.PopHead(bcs), true);
			DCHECK_NOTNULL(bcs);
			if (bcs->data != 0) jackieFree_Ex(bcs->data, TRACE_FILE_AND_LINE_);
			commandPool.Reclaim(bcs);
		}
		for (UInt32 i = 0; i < deAllocCommandQ.Size(); i++)
		{
			DCHECK_EQ(deAllocCommandQ.PopHead(bcs), true);
			DCHECK_NOTNULL(bcs);
			if (bcs->data != 0) jackieFree_Ex(bcs->data, TRACE_FILE_AND_LINE_);
			commandPool.Reclaim(bcs);
		}
		deAllocCommandQ.Clear();
		allocCommandQ.Clear();
		commandPool.Clear();
	}


	void ServerApplication::InitIPAddress(void)
	{
		assert(IPAddress[0] == JACKIE_NULL_ADDRESS);
		JackieINetSocket::GetMyIP(IPAddress);

		// Sort the addresses from lowest to highest
		int startingIdx = 0;
		while (startingIdx < MAX_COUNT_LOCAL_IP_ADDR - 1 &&
			IPAddress[startingIdx] != JACKIE_NULL_ADDRESS)
		{
			int lowestIdx = startingIdx;
			for (int curIdx = startingIdx + 1; curIdx < MAX_COUNT_LOCAL_IP_ADDR - 1 && IPAddress[curIdx] != JACKIE_NULL_ADDRESS; curIdx++)
			{
				if (IPAddress[curIdx] < IPAddress[startingIdx])
				{
					lowestIdx = curIdx;
				}
			}
			if (startingIdx != lowestIdx)
			{
				JackieAddress temp = IPAddress[startingIdx];
				IPAddress[startingIdx] = IPAddress[lowestIdx];
				IPAddress[lowestIdx] = temp;
			}
			++startingIdx;
		}
	}

	void ServerApplication::DeallocBindedSockets(void)
	{
		for (UInt32 index = 0; index < bindedSockets.Size(); index++)
		{
			if (bindedSockets[index] != 0) JISAllocator::DeallocJIS(bindedSockets[index]);
		}
	}
	void ServerApplication::ClearSocketQueryOutputs(void)
	{
		socketQueryOutput.Clear();
	}


	Packet* ServerApplication::AllocPacket(UInt32 dataSize)
	{
		JDEBUG << "Network Thread Alloc One Packet";
		Packet *p = 0;
		do { p = packetPool.Allocate(); } while (p == 0);

		//p = new ( (void*) p ) Packet; we do not need call default ctor
		p->data = (unsigned char*)jackieMalloc_Ex(dataSize, TRACE_FILE_AND_LINE_);
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->freeInternalData = true;
		p->guid = JACKIE_NULL_GUID;
		p->wasGeneratedLocally = false;

		return p;
	}
	Packet* ServerApplication::AllocPacket(UInt32 dataSize, unsigned char *data)
	{
		JDEBUG << "Network Thread Alloc One Packet";
		Packet *p = 0;
		do { p = packetPool.Allocate(); } while (p == 0);

		//p = new ( (void*) p ) Packet; no custom ctor so no need to call default ctor
		p->data = (unsigned char*)data;
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->freeInternalData = true;
		p->guid = JACKIE_NULL_GUID;
		p->wasGeneratedLocally = false;

		return p;
	}
	void ServerApplication::ReclaimAllPackets()
	{
		JDEBUG << "Network Thread Reclaims All Packets";

		Packet* packet;
		for (UInt32 index = 0; index < deAllocPacketQ.Size(); index++)
		{
			DCHECK_EQ(deAllocPacketQ.PopHead(packet), true);
			if (packet->freeInternalData)
			{
				//packet->~Packet(); no custom dtor so no need to call default dtor
				jackieFree_Ex(packet->data, TRACE_FILE_AND_LINE_);
			}
			packetPool.Reclaim(packet);
		}
	}
	inline void ServerApplication::ReclaimPacket(Packet *packet)
	{
		JDEBUG << "User Thread Reclaims One Packet";
		DCHECK_EQ(deAllocPacketQ.PushTail(packet), true);
	}


	int ServerApplication::CreateRecvPollingThread(int threadPriority, UInt32 index)
	{
		char* arg = (char*)jackieMalloc_Ex(sizeof(ServerApplication*) + sizeof(index), TRACE_FILE_AND_LINE_);
		ServerApplication* serv = this;
		memcpy(arg, &serv, sizeof(ServerApplication*));
		memcpy(arg + sizeof(ServerApplication*), (char*)&index, sizeof(index));
		return JACKIE_Thread::Create(JACKIE_INET::RunRecvCycleLoop, arg, threadPriority);
	}
	int ServerApplication::CreateNetworkUpdateThread(int threadPriority)
	{
		return JACKIE_Thread::Create(JACKIE_INET::RunNetworkUpdateCycleLoop, this, threadPriority);
	}
	void ServerApplication::StopRecvThread()
	{
		endThreads = true;
#if USE_SINGLE_THREAD == 0
		for (UInt32 i = 0; i < bindedSockets.Size(); i++)
		{
			if (bindedSockets[i]->IsBerkleySocket())
			{
				JISBerkley* sock = (JISBerkley*)bindedSockets[i];
				if (sock->GetBindingParams()->isBlocKing == USE_BLOBKING_SOCKET)
				{
					/// try to send 0 data to let recv thread keep running
					/// to detect the isRecvPollingThreadActive === false so that stop the thread
					char zero[] = "This is used to Stop Recv Thread";
					JISSendParams sendParams = { zero, sizeof(zero), 0, sock->GetBoundAddress(), 0 };
					sock->Send(&sendParams, TRACE_FILE_AND_LINE_);
					TimeMS timeout = Get32BitsTimeMS() + 1000;
					while (isRecvPollingThreadActive.GetValue() > 0 && Get32BitsTimeMS() < timeout)
					{
						sock->Send(&sendParams, TRACE_FILE_AND_LINE_);
						JackieSleep(100);
					}
				}
			}
		}
#endif
	}
	void ServerApplication::StopNetworkUpdateThread()
	{
		endThreads = true;
		isNetworkUpdateThreadActive = false;
	}


	void ServerApplication::ProcessOneRecvParam(JISRecvParams* recvParams)
	{
		JDEBUG << "Process One RecvParam";

#if LIBCAT_SECURITY==1
#ifdef CAT_AUDIT
		printf("AUDIT: RECV ");
		for( int ii = 0; ii < length; ++ii )
		{
			printf("%02x", ( cat::u8 )data[ii]);
		}
		printf("\n");
#endif
#endif // LIBCAT_SECURITY

		DCHECK_NE(recvParams->senderINetAddress.GetPortHostOrder(), 0);

		bool isOfflinerecvParams = true;
		if (ProcessOneOfflineRecvParam(recvParams, &isOfflinerecvParams)) return;

		/// See if this datagram came from a connected system
		RemoteEndPoint* remoteEndPoint =
			GetRemoteEndPoint(recvParams->senderINetAddress, true, true);
		if (remoteEndPoint != 0) // if this datagram comes from connected system
		{
			if (!isOfflinerecvParams)
			{
				remoteEndPoint->reliabilityLayer.ProcessJISRecvParamsFromConnectedEndPoint(this, remoteEndPoint->MTUSize);
			}
		}
		else
		{
			char str[256];
			recvParams->senderINetAddress.ToString(true, str);
			JWARNING << "Packet from unconnected sender " << str;
		}
	}
	bool ServerApplication::ProcessOneOfflineRecvParam(JISRecvParams* recvParams, bool* isOfflinerecvParams)
	{
		RemoteEndPoint* remoteEndPoint;
		Packet* packet;
		UInt32 index;

		const char* strAddr = recvParams->senderINetAddress.ToString();

		return false;
	}


	void ServerApplication::ProcessConnectionRequestCancelQ(void)
	{
		JDEBUG << "Network Thread Process ConnectionRequest CancelQ";

		if (connReqCancelQ.IsEmpty())
		{
			JDEBUG << "ConnectionRequestCancelQ is EMPTY";
			return;
		}

		JackieAddress connReqCancelAddr;
		ConnectionRequest* connReq = 0;

		connReqCancelQLock.Lock();
		for (UInt32 index = 0; index < connReqCancelQ.Size(); index++)
		{
			/// Cancel pending connection attempt, if there is one
			connReqQLock.Lock();
			for (UInt32 i = 0; i < connReqQ.Size(); i++)
			{
				if (connReqQ[i]->receiverAddr == connReqCancelAddr)
				{
#if LIBCAT_SECURITY==1
					CAT_AUDIT_PRINTF("AUDIT: Deleting requestedConnectionQueue %i client_handshake %x\n", i, connReqQ[i]->client_handshake);
					JACKIE_INET::OP_DELETE(connReqQ[i]->client_handshake, TRACE_FILE_AND_LINE_);
#endif
					JACKIE_INET::OP_DELETE(connReqQ[i], TRACE_FILE_AND_LINE_);
					connReqQ.RemoveAtIndex(i);
					break;
				}
			}
			connReqQLock.Unlock();
		}
		connReqCancelQLock.Unlock();
	}

	/// @TO-DO
	void ServerApplication::ProcessConnectionRequestQ(TimeUS& timeUS, TimeMS& timeMS)
	{
		JDEBUG << "Network Thread Process ConnectionRequestQ";

		if (connReqQ.IsEmpty())
		{
			JDEBUG << "ConnectionRequestQ is EMPTY";
			return;
		}

		if (timeUS == 0)
		{
			timeUS = Get64BitsTimeUS();
			timeMS = (TimeMS)(timeUS / (TimeUS)1000);
		}

		ConnectionRequest *connReq;

		connReqQLock.Lock();
		for (UInt32 index = 0; index < connReqQ.Size(); index++)
		{
			connReq = connReqQ[index];
			if (connReq->nextRequestTime < timeMS)
			{
				/// reach the max try times 
				if (connReq->requestsMade == connReq->connAttemptTimes + 1)
				{

					/// free data inside conn req
					if (connReq->data != 0)
					{
						jackieFree_Ex(connReq->data, TRACE_FILE_AND_LINE_);
						connReq->data = 0;
					}

					/// Tell USER connection attempt failed
					static UInt8 msgid;
					Packet* packet = AllocPacket(sizeof(unsigned char), &msgid);
					packet->data[0] = ID_CONNECTION_ATTEMPT_FAILED;
					packet->systemAddress = connReq->receiverAddr;
					packet->freeInternalData = false;
					DCHECK_EQ(allocPacketQ.PushTail(packet), true);

#if LIBCAT_SECURITY==1
					CAT_AUDIT_PRINTF("AUDIT: Connection attempt FAILED so deleting rcs->client_handshake object %x\n", rcs->client_handshake);
					JACKIE_INET::OP_DELETE(connReq->client_handshake,
						TRACE_FILE_AND_LINE_);
#endif
					JACKIE_INET::OP_DELETE(connReq, TRACE_FILE_AND_LINE_);

					/// remove this conn request fron  queue
					connReqQ.RemoveAtIndex(index);
				}
				else /// try to connect again
				{
					/// more times try to request connection, less mtu used
					int MTUSizeIndex = connReq->requestsMade /
						(connReq->connAttemptTimes / mtuSizesCount);
					if (MTUSizeIndex >= mtuSizesCount)
						MTUSizeIndex = mtuSizesCount - 1;

					connReq->requestsMade++;
					connReq->nextRequestTime = timeMS + connReq->connAttemptIntervalMS;

					JackieBits bitStream;
					bitStream.Write((MessageID)ID_OPEN_CONNECTION_REQUEST_1);
					bitStream.WriteAlignedBytes((const unsigned char*)OFFLINE_MESSAGE_DATA_ID, sizeof(OFFLINE_MESSAGE_DATA_ID));
					bitStream.Write((MessageID)RAKNET_PROTOCOL_VERSION);
					bitStream.PadZeroAfterAlignedWRPos(mtuSizes[MTUSizeIndex] - UDP_HEADER_SIZE);

					JDEBUG << "The " << (int)connReq->requestsMade
						<< " times to try to connect to remote sever [" << connReq->receiverAddr.ToString() << "]";

					/// @TO-DO i am now in here

				}
			}
		}
		connReqQLock.Unlock();
	}

	void ServerApplication::ProcessAllocCommandQ(TimeUS& timeUS, TimeMS& timeMS)
	{
		JDEBUG << "Network Thread Process Alloc CommandQ";

		Command* cmd = 0;
		RemoteEndPoint* remoteEndPoint = 0;

		/// process command queue
		for (UInt32 index = 0; index < allocCommandQ.Size(); index++)
		{

			/// no need to check if bufferedCommand == 0, because we never push 0 pointer
			DCHECK_EQ(allocCommandQ.PopHead(cmd), true);
			DCHECK_NOTNULL(cmd);
			DCHECK_NOTNULL(cmd->data);

			switch (cmd->command)
			{
			case Command::BCS_SEND:
				JDEBUG << "BCS_SEND";
				/// GetTime is a very slow call so do it once and as late as possible
				if (timeUS == 0)
				{
					timeUS = Get64BitsTimeUS();
					timeMS = (TimeMS)(timeUS / (TimeUS)1000);
				}
				/// send data stored in this bc right now
				if (SendRightNow(timeUS, true, cmd) == false)
					jackieFree_Ex(cmd->data, TRACE_FILE_AND_LINE_);
				/// Set the new connection state AFTER we call sendImmediate in case we are 
				/// setting it to a disconnection state, which does not allow further sends
				if (cmd->repStatus != RemoteEndPoint::NO_ACTION)
				{
					remoteEndPoint = GetRemoteEndPoint(
						cmd->systemIdentifier, true, true);
					if (remoteEndPoint != 0)
						remoteEndPoint->status = cmd->repStatus;
				}
				break;
			case Command::BCS_CLOSE_CONNECTION:
				JDEBUG << "BCS_CLOSE_CONNECTION";
				CloseConnectionInternally(false, true, cmd);
				break;
			case Command::BCS_CHANGE_SYSTEM_ADDRESS: //re-rout
				remoteEndPoint = GetRemoteEndPoint(
					cmd->systemIdentifier, true, true);
				if (remoteEndPoint != 0)
				{
					Int32 existingSystemIndex =
						GetRemoteEndPointIndex(remoteEndPoint->systemAddress);
					RefRemoteEndPoint(
						cmd->systemIdentifier.systemAddress, existingSystemIndex);
				}
				break;
			case Command::BCS_GET_SOCKET:
				JDEBUG << "BCS_GET_SOCKET";
				break;
			default:
				JERROR << "Not Found Matched BufferedCommand";
				break;
			}

			JDEBUG << "Network Thread Reclaims One Command";
			DCHECK_EQ(deAllocCommandQ.PushTail(cmd), true);
		}

	}
	void ServerApplication::ProcessAllocJISRecvParamsQ(void)
	{
		JDEBUG << "Network Thread Process Alloc JISRecvParamsQ";

		JISRecvParams* recvParams = 0;
		for (UInt32 outter = 0; outter < bindedSockets.Size(); outter++)
		{
			for (UInt32 inner = 0; inner < allocRecvParamQ[outter].Size(); inner++)
			{
				/// no need to check if recvParams == 0, because we never push 0 pointer
				DCHECK_EQ(allocRecvParamQ[outter].PopHead(recvParams), true);
				ProcessOneRecvParam(recvParams);
				ReclaimOneJISRecvParams(recvParams, inner);
			}
		}
	}

	/// @TO-DO
	void ServerApplication::AdjustTimestamp(Packet*& incomePacket) const
	{
		JDEBUG << "@TO-DO AdjustTimestamp()";

		if ((unsigned char)incomePacket->data[0] == ID_TIMESTAMP)
		{
			if (incomePacket->length >= sizeof(char) + sizeof(Time))
			{
				char* data = (char*)&incomePacket->data[sizeof(char)];
				//#ifdef _DEBUG
				//				RakAssert(IsActive());
				//				RakAssert(data);
				//#endif
				//
				//				RakNet::BitStream timeBS(data, sizeof(RakNet::Time), false);
				//				timeBS.EndianSwapBytes(0, sizeof(RakNet::Time));
				//				RakNet::Time encodedTimestamp;
				//				timeBS.Read(encodedTimestamp);
				//
				//				encodedTimestamp = encodedTimestamp - GetBestClockDifferential(systemAddress);
				//				timeBS.SetWriteOffset(0);
				//				timeBS.Write(encodedTimestamp);
			}
		}
	}


	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const JackieAddress&
		sa, bool neededBySendThread, bool onlyWantActiveEndPoint) const
	{
		if (sa == JACKIE_NULL_ADDRESS) return 0;

		if (neededBySendThread)
		{
			Int32 index = GetRemoteEndPointIndex(sa);
			if (index != -1)
			{
				if (!onlyWantActiveEndPoint || remoteSystemList[index].isActive)
				{
					DCHECK_EQ(remoteSystemList[index].systemAddress, sa);
					return &remoteSystemList[index];
				}
			}
		}
		else
		{
			/// Active EndPoints take priority.  But if matched end point is inactice, 
			/// return the first EndPoint match found
			Int32 inActiveEndPointIndex = -1;
			for (UInt32 index = 0; index < maxConnections; index++)
			{
				if (remoteSystemList[index].systemAddress == sa)
				{
					if (remoteSystemList[index].isActive)
						return &remoteSystemList[index];
					else if (inActiveEndPointIndex == -1)
						inActiveEndPointIndex = index;
				}
			}

			/// matched end pint was found but it is inactive
			if (inActiveEndPointIndex != -1 && !onlyWantActiveEndPoint)
				return &remoteSystemList[inActiveEndPointIndex];
		}

		// no matched end point found
		return 0;
	}
	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const
		JACKIE_INET_Address_GUID_Wrapper& senderWrapper, bool neededBySendThread,
		bool onlyWantActiveEndPoint) const
	{
		if (senderWrapper.guid != JACKIE_NULL_GUID)
			return GetRemoteEndPoint(senderWrapper.guid, onlyWantActiveEndPoint);
		else
			return GetRemoteEndPoint(senderWrapper.systemAddress, neededBySendThread,
			onlyWantActiveEndPoint);
	}
	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const JackieGUID&
		senderGUID, bool onlyWantActiveEndPoint) const
	{
		if (senderGUID == JACKIE_NULL_GUID) return 0;
		for (UInt32 i = 0; i < maxConnections; i++)
		{
			if (remoteSystemList[i].guid == senderGUID &&
				(onlyWantActiveEndPoint == false || remoteSystemList[i].isActive))
			{
				return remoteSystemList + i;
			}
		}
		return 0;
	}
	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const JackieAddress& sa) const
	{
		Int32 index = GetRemoteEndPointIndex(sa);
		if (index == -1) return 0;
		return remoteSystemList + index;
	}
	Int32 ServerApplication::GetRemoteEndPointIndex(const JackieAddress &sa) const
	{
		UInt32 hashindex = JackieAddress::ToHashCode(sa);
		hashindex = hashindex % (maxConnections * RemoteEndPointLookupHashMutiple);
		RemoteEndPointIndex* curr = remoteSystemLookup[hashindex];
		while (curr != 0)
		{
			if (remoteSystemList[curr->index].systemAddress == sa)
				return curr->index;
			curr = curr->next;
		}
		return  -1;
	}


	void ServerApplication::RefRemoteEndPoint(const JackieAddress &sa, UInt32 index)
	{
		RemoteEndPoint* remote = remoteSystemList + index;
		JackieAddress old = remote->systemAddress;
		if (old != JACKIE_NULL_ADDRESS)
		{
			// The system might be active if rerouting
			DCHECK_EQ(remoteSystemList[index].isActive, false);
			// Remove the reference if the reference is pointing to this inactive system
			if (GetRemoteEndPoint(old) == remote)
			{
				DeRefRemoteEndPoint(old);
			}
		}

		DeRefRemoteEndPoint(sa);
		remoteSystemList[index].systemAddress = sa;

		UInt32 hashindex = JackieAddress::ToHashCode(sa);
		hashindex = hashindex % (maxConnections * RemoteEndPointLookupHashMutiple);

		RemoteEndPointIndex *rsi = 0;
		do { rsi = remoteSystemIndexPool.Allocate(); } while (rsi == 0);

		if (remoteSystemLookup[hashindex] == 0)
		{
			rsi->next = 0;
			rsi->index = index;
			remoteSystemLookup[hashindex] = rsi;
		}
		else
		{
			RemoteEndPointIndex *cur = remoteSystemLookup[hashindex];
			while (cur->next != 0) { cur = cur->next; } /// move to last one
			cur->next = rsi;
			rsi->next = 0;
			rsi->index = index;
		}

	}
	void ServerApplication::DeRefRemoteEndPoint(const JackieAddress &sa)
	{
		UInt32 hashindex = JackieAddress::ToHashCode(sa);
		hashindex = hashindex % (maxConnections * RemoteEndPointLookupHashMutiple);

		RemoteEndPointIndex *cur = remoteSystemLookup[hashindex];
		RemoteEndPointIndex *last = 0;

		while (cur != 0)
		{
			if (remoteSystemList[cur->index].systemAddress == sa)
			{
				if (last == 0)
				{
					remoteSystemLookup[hashindex] = cur->next;
				}
				else
				{
					last->next = cur->next;
				}
				remoteSystemIndexPool.Reclaim(cur);
				break;
			}
			last = cur;
			cur = cur->next;
		}
	}

	//@TO-DO
	bool ServerApplication::SendRightNow(TimeUS currentTime, bool useCallerAlloc, Command* bufferedCommand)
	{
		JDEBUG << "@TO-DO::SendRightNow()";
		return true;
	}
	//@TO-DO
	void ServerApplication::CloseConnectionInternally(bool sendDisconnectionNotification, bool performImmediate, Command* bufferedCommand)
	{
		JDEBUG << "@TO-DO::CloseConnectionInternally()";
	}



	void ServerApplication::PacketGoThroughPluginCBs(Packet*& incomePacket)
	{
		JDEBUG << "User Thread Packet Go Through PluginCBs with packet indentity " << (int)incomePacket->data[0];
		UInt32 i;
		for (i = 0; i < pluginListTS.Size(); i++)
		{
			switch ((UInt32)incomePacket->data[0])
			{
			case ID_DISCONNECTION_NOTIFICATION:
				pluginListTS[i]->OnClosedConnection(incomePacket->systemAddress, incomePacket->guid, LCR_DISCONNECTION_NOTIFICATION);
				break;
			case ID_CONNECTION_LOST:
				pluginListTS[i]->OnClosedConnection(incomePacket->systemAddress, incomePacket->guid, LCR_CONNECTION_LOST);
				break;
			case ID_NEW_INCOMING_CONNECTION:
				pluginListTS[i]->OnNewConnection(incomePacket->systemAddress, incomePacket->guid, true);
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				pluginListTS[i]->OnNewConnection(incomePacket->systemAddress, incomePacket->guid, false);
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_CONNECTION_ATTEMPT_FAILED);
				break;
			case ID_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY);
				break;
			case ID_OUR_SYSTEM_REQUIRES_SECURITY:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_OUR_SYSTEM_REQUIRES_SECURITY);
				break;
			case ID_PUBLIC_KEY_MISMATCH:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_PUBLIC_KEY_MISMATCH);
				break;
			case ID_ALREADY_CONNECTED:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_ALREADY_CONNECTED);
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_NO_FREE_INCOMING_CONNECTIONS);
				break;
			case ID_CONNECTION_BANNED:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_CONNECTION_BANNED);
				break;
			case ID_INVALID_PASSWORD:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_INVALID_PASSWORD);
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_INCOMPATIBLE_PROTOCOL);
				break;
			case ID_IP_RECENTLY_CONNECTED:
				pluginListTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_IP_RECENTLY_CONNECTED);
				break;
			}
		}

		for (i = 0; i < pluginListNTS.Size(); i++)
		{
			switch (incomePacket->data[0])
			{
			case ID_DISCONNECTION_NOTIFICATION:
				pluginListNTS[i]->OnClosedConnection(incomePacket->systemAddress, incomePacket->guid, LCR_DISCONNECTION_NOTIFICATION);
				break;
			case ID_CONNECTION_LOST:
				pluginListNTS[i]->OnClosedConnection(incomePacket->systemAddress, incomePacket->guid, LCR_CONNECTION_LOST);
				break;
			case ID_NEW_INCOMING_CONNECTION:
				pluginListNTS[i]->OnNewConnection(incomePacket->systemAddress, incomePacket->guid, true);
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				pluginListNTS[i]->OnNewConnection(incomePacket->systemAddress, incomePacket->guid, false);
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_CONNECTION_ATTEMPT_FAILED);
				break;
			case ID_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY);
				break;
			case ID_OUR_SYSTEM_REQUIRES_SECURITY:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_OUR_SYSTEM_REQUIRES_SECURITY);
				break;
			case ID_PUBLIC_KEY_MISMATCH:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_PUBLIC_KEY_MISMATCH);
				break;
			case ID_ALREADY_CONNECTED:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_ALREADY_CONNECTED);
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_NO_FREE_INCOMING_CONNECTIONS);
				break;
			case ID_CONNECTION_BANNED:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_CONNECTION_BANNED);
				break;
			case ID_INVALID_PASSWORD:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_INVALID_PASSWORD);
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_INCOMPATIBLE_PROTOCOL);
				break;
			case ID_IP_RECENTLY_CONNECTED:
				pluginListNTS[i]->OnFailedConnectionAttempt(incomePacket, CAFR_IP_RECENTLY_CONNECTED);
				break;
			}
		}

	}
	void ServerApplication::PacketGoThroughPlugins(Packet*& incomePacket)
	{

		JDEBUG << "User Thread Packet Go Through Plugin";

		UInt32 i;
		PluginActionType pluginResult;

		for (i = 0; i < pluginListTS.Size(); i++)
		{
			pluginResult = pluginListTS[i]->OnRecvPacket(incomePacket);
			if (pluginResult == PROCESSED_BY_ME_THEN_DEALLOC)
			{
				ReclaimPacket(incomePacket);
				// Will do the loop again and get another incomePacket
				incomePacket = 0;
				break; // break out of the enclosing forloop
			}
			else if (pluginResult == HOLD_ON_BY_ME_NOT_DEALLOC)
			{
				incomePacket = 0;
				break;
			}
		}

		for (i = 0; i < pluginListNTS.Size(); i++)
		{
			pluginResult = pluginListNTS[i]->OnRecvPacket(incomePacket);
			if (pluginResult == PROCESSED_BY_ME_THEN_DEALLOC)
			{
				ReclaimPacket(incomePacket);
				// Will do the loop again and get another incomePacket
				incomePacket = 0;
				break; // break out of the enclosing forloop
			}
			else if (pluginResult == HOLD_ON_BY_ME_NOT_DEALLOC)
			{
				incomePacket = 0;
				break;
			}
		}
	}
	void ServerApplication::UpdatePlugins(void)
	{
		JDEBUG << "User Thread Update Plugins";
		UInt32 i;
		for (i = 0; i < pluginListTS.Size(); i++)
		{
			pluginListTS[i]->Update();
		}
		for (i = 0; i < pluginListNTS.Size(); i++)
		{
			pluginListNTS[i]->Update();
		}
	}


	Packet* ServerApplication::GetPacketOnce(void)
	{
		TIMED_FUNC();

#if USE_SINGLE_THREAD == 0
		if (!(IsActive())) return 0;
#endif

#if USE_SINGLE_THREAD != 0
		RunRecvCycleOnce(0);
		RunNetworkUpdateCycleOnce();
#endif
		return RunGetPacketCycleOnce();
	}
	Packet* ServerApplication::RunGetPacketCycleOnce(void)
	{
		ReclaimAllCommands();

		/// UPDATE all plugins
		UpdatePlugins();

		Packet *incomePacket = 0;
		/// Pop out one Packet from queue
		if (allocPacketQ.Size() > 0)
		{
			//////////////////////////////////////////////////////////////////////////
			/// Get one income packet from bufferedPacketsQueue
			DCHECK_EQ(allocPacketQ.PopHead(incomePacket), true);
			DCHECK_NOTNULL(incomePacket->data);
			//////////////////////////////////////////////////////////////////////////
			AdjustTimestamp(incomePacket);
			//////////////////////////////////////////////////////////////////////////
			/// Some locally generated packets need to be processed by plugins,
			/// for example ID_FCM2_NEW_HOST. The plugin itself should intercept
			/// these messages generated remotely
			PacketGoThroughPluginCBs(incomePacket);
			PacketGoThroughPlugins(incomePacket);
			//////////////////////////////////////////////////////////////////////////
		}

		return incomePacket;
	}
	bool ServerApplication::RunNetworkUpdateCycleOnce()
	{
		//TIMED_FUNC();
		//// @NOTICE I moved this code to JISbEKELY::RecvFrom()
		//// UInt32 index;
		//static JISRecvParams recv;
		//#if !defined(WINDOWS_STORE_RT) && !defined(__native_client__)
		//
		//		for( index = 0; index < JISList.Size(); index++ )
		//		{
		//			if( JISList[index]->IsBerkleySocket() )
		//			{
		//				JISBerkley* berkelySock = ( (JISBerkley*) JISList[index] );
		//				if( berkelySock->GetSocketTransceiver() != 0 )
		//				{
		//					int len;
		//					char dataOut[MAXIMUM_MTU_SIZE];
		//					do
		//					{
		//						len = berkelySock->GetSocketTransceiver()->
		//							JackieINetRecvFrom(dataOut, &recv.senderINetAddress, true);
		//
		//						if( len > 0 )
		//						{
		//							ProcessOneRecvParam(&recv, updateBitStream);
		//						}
		//					} while( len > 0 );
		//
		//				}
		//			}
		//		}
		//#endif

		/// who alloc who dealloc
		ReclaimAllPackets();

		/// process buffered recv params
		ProcessAllocJISRecvParamsQ();

		TimeUS timeUS = 0;
		TimeMS timeMS = 0;
		/// process buffered commands
		ProcessAllocCommandQ(timeUS, timeMS);

		/// process buffered connection request and cance
		/// Cancel certain conn req before process Connection Request Q 
		ProcessConnectionRequestCancelQ();
		ProcessConnectionRequestQ(timeUS, timeMS);

		return 0;
	}
	void ServerApplication::RunRecvCycleOnce(UInt32 index)
	{
		TIMED_FUNC();

		JISRecvParams* recvParams = 0;

		ReclaimAllJISRecvParams(index);

		//do { recvParams = JISRecvParamsPool[index].Allocate(); } while( recvParams == 0 );
		recvParams = AllocJISRecvParams(index);
		recvParams->socket = bindedSockets[index];

		if (((JISBerkley*)bindedSockets[index])->RecvFrom(recvParams) > 0)
		{
			DCHECK_EQ(allocRecvParamQ[index].PushTail(recvParams), true);

			if (incomeDatagramEventHandler != 0)
			{
				if (!incomeDatagramEventHandler(recvParams)) JWARNING << "incomeDatagramEventHandler(recvStruct) Failed.";
			}

#if USE_SINGLE_THREAD == 0
			if (allocRecvParamQ[index].Size() > 0) quitAndDataEvents.TriggerEvent();
#endif

		}
		else
		{
			JISRecvParamsPool[index].Reclaim(recvParams);
		}

	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::RunRecvCycleLoop)
	{
		ServerApplication *serv = *(ServerApplication**)arguments;
		UInt32 index = *((UInt32*)((char*)arguments + sizeof(ServerApplication*)));

		serv->isRecvPollingThreadActive.Increment();

		JDEBUG << "Recv thread " << "is running in backend....";
		while (!serv->endThreads)
		{
			serv->RunRecvCycleOnce(index);
			JackieSleep(1000);
		}
		JDEBUG << "Recv polling thread Stops....";

		serv->isRecvPollingThreadActive.Decrement();
		jackieFree_Ex(arguments, TRACE_FILE_AND_LINE_);
		return 0;
	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::RunNetworkUpdateCycleLoop)
	{
		ServerApplication *serv = (ServerApplication*)arguments;
		serv->isNetworkUpdateThreadActive = true;

		JDEBUG << "Send polling thread is running in backend....";
		while (!serv->endThreads)
		{
			/// Normally, buffered sending packets go out every other 10 ms.
			/// or TriggerEvent() is called by recv thread
			serv->RunNetworkUpdateCycleOnce();
			serv->quitAndDataEvents.WaitEvent(5);
			JackieSleep(1000);
		}
		JDEBUG << "Send polling thread Stops....";

		serv->isNetworkUpdateThreadActive = false;
		return 0;
	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::UDTConnect) { return 0; }
	//STATIC_FACTORY_DEFINITIONS(IServerApplication, ServerApplication);
	STATIC_FACTORY_DEFINITIONS(ServerApplication, ServerApplication);


	void ServerApplication::ResetSendReceipt(void)
	{
		sendReceiptSerialMutex.Lock();
		sendReceiptSerial = 1;
		sendReceiptSerialMutex.Unlock();
	}
	UInt64 ServerApplication::CreateUniqueRandness(void)
	{
		UInt64 g = Get64BitsTimeUS();
		TimeUS lastTime, thisTime, diff;
		unsigned char diffByte = 0;
		// Sleep a small random time, then use the last 4 bits as a source of randomness
		for (int j = 0; j < 4; j++)
		{
			diffByte = 0;
			for (int index = 0; index < 4; index++)
			{
				lastTime = Get64BitsTimeUS();
				JackieSleep(1);
				thisTime = Get64BitsTimeUS();
				diff = thisTime - lastTime;
				diffByte ^= (unsigned char)((diff & 15) << (index * 2)); ///0xF = 1111 = 15
				if (index == 3) diffByte ^= (unsigned char)((diff & 15) >> 2);
			}
			((unsigned char*)&g)[4 + j] ^= diffByte;
		}
		return g;
	}

	void ServerApplication::CancelConnectionRequest(const JackieAddress& target)
	{
		JDEBUG << "User Thread Cancel Connection Request To " << target.ToString();
		connReqCancelQLock.Lock();
		DCHECK_EQ(connReqCancelQ.PushTail(target), true);
		connReqCancelQLock.Unlock();
	}

	JACKIE_INET::ConnectionAttemptResult ServerApplication::Connect(const char* host, UInt16 port, const char *pwd /*= 0*/, UInt32 pwdLen /*= 0*/, JACKIE_Public_Key *publicKey /*= 0*/, UInt32 ConnectionSocketIndex /*= 0*/, UInt32 ConnectionAttemptTimes /*= 6*/, UInt32 ConnectionAttemptIntervalMS /*= 1000*/, TimeMS timeout /*= 0*/, UInt32 extraData/*=0*/)
	{
		if (host == 0)
		{
			JERROR << "invalid host adress !";
			return INVALID_PARAM;
		}
		if (port == 0)
		{
			JERROR << "invalid port ! !";
			return INVALID_PARAM;
		}
		if (endThreads)
		{
			JERROR << "not call Start() !\n";
			return INVALID_PARAM;
		}

		if (pwdLen > 255) pwdLen = 255;
		if (pwd == 0) pwdLen = 0;

		bool found = false;
		for (UInt32 i = 0; i < bindedSockets.Size(); i++)
		{
			if (bindedSockets[i]->GetUserConnectionSocketIndex() == ConnectionSocketIndex)
				found = true;
		}
		if (!found)
		{
			JERROR << "invalid ConnectionSocketIndex";
			return INVALID_PARAM;
		}

		JackieAddress addr;
		bool ret = addr.FromString(host, port,
			bindedSockets[ConnectionSocketIndex]->GetBoundAddress().GetIPVersion());
		if (!ret || addr == JACKIE_NULL_ADDRESS) return CANNOT_RESOLVE_DOMAIN_NAME;

		if (GetRemoteEndPoint(addr, false, true) != 0)
			return CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS;


		if (GetRemoteEndPoint(addr, false, true) != 0)
			return ALREADY_CONNECTED_TO_ENDPOINT;

		ConnectionRequest* connReq = JACKIE_INET::OP_NEW<ConnectionRequest>(TRACE_FILE_AND_LINE_);
		connReq->receiverAddr = addr;
		connReq->nextRequestTime = GetTimeMS();
		connReq->requestsMade = 0;
		connReq->data = 0;
		connReq->socket = 0;
		connReq->extraData = extraData;
		connReq->socketIndex = ConnectionSocketIndex;
		connReq->actionToTake = ConnectionRequest::CONNECT;
		connReq->connAttemptTimes = ConnectionAttemptTimes;
		connReq->connAttemptIntervalMS = ConnectionAttemptIntervalMS;
		connReq->timeout = timeout;
		connReq->pwdLen = pwdLen;
		memcpy(connReq->pwd, pwd, pwdLen);

#if LIBCAT_SECURITY ==1
		CAT_AUDIT_PRINTF("AUDIT: In SendConnectionRequest()\n");
		if( !GenerateConnectionRequestChallenge(connReq, publicKey) )
			return SECURITY_INITIALIZATION_FAILED;
#else
		(void)publicKey;
#endif

		connReqCancelQLock.Lock();
		for (UInt32 Index = 0; Index < connReqQ.Size(); Index++)
		{
			if (connReqQ[Index]->receiverAddr == addr)
			{
				connReqCancelQLock.Unlock();
				JACKIE_INET::OP_DELETE(connReq, TRACE_FILE_AND_LINE_);
				return CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS;
			}
		}
		DCHECK_EQ(connReqQ.PushTail(connReq), true);
		connReqCancelQLock.Unlock();

		return CONNECTION_ATTEMPT_POSTED;
	}

}