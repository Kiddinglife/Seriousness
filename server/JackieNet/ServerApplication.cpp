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
	static const unsigned int RemoteEndPointLookupHashMutiple = 8;
	static const int mtuSizesCount = 3;
	static const int mtuSizes[mtuSizesCount] = { MAXIMUM_MTU_SIZE, 1200, 576 };
	/// I set this because I limit ID_CONNECTION_REQUEST to 512 bytes, 
	/// and the password is appended to that *incomePacket.
	static const unsigned int MAX_OFFLINE_DATA_LENGTH = 400;
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
		maxIncomeConnections = maxConnections = 0;
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

		myGuid = JACKIE_INet_GUID_Null;
		firstExternalID = JACKIE_INET_Address_Null;

		for( unsigned int index = 0; index < MAX_COUNT_LOCAL_IP_ADDR; index++ )
		{
			IPAddress[index] = JACKIE_INET_Address_Null;
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


	JACKIE_INET::StartupResult ServerApplication::Start(unsigned int maxConn,
		BindSocket *bindLocalSockets,
		unsigned int bindLocalSocketsCount,
		Int32 threadPriority /*= -99999*/)
	{

		if( IsActive() ) return StartupResult::ALREADY_STARTED;

		// If getting the guid failed in the constructor, try again
		if( myGuid.g == 0 )
		{
			GenerateGUID();
			if( myGuid.g == 0 ) return StartupResult::COULD_NOT_GENERATE_GUID;
		}

		if( myGuid == JACKIE_INet_GUID_Null )
		{
			rnr.SeedMT((unsigned int) ( ( myGuid.g >> 32 ) ^ myGuid.g ));
		}

		if( threadPriority == -99999 )
		{
#if  defined(_WIN32)
			threadPriority = 0;
#else
			threadPriority = 1000;
#endif
		}

		InitIPAddress();

		assert(bindLocalSockets && bindLocalSocketsCount >= 1);
		if( bindLocalSockets == 0 || bindLocalSocketsCount < 1 )
			return INVALID_JACKIE_LOCAL_SOCKET;

		assert(maxConn > 0); if( maxConn <= 0 ) return INVALID_MAX_CONNECTIONS;

		/// Start to bind given sockets 
		unsigned int index;
		JackieINetSocket* sock;
		JISBerkleyBindParams berkleyBindParams;
		JISBindResult bindResult;
		DeallocJISList(); /// Deref All Sockets
		for( index = 0; index < bindLocalSocketsCount; index++ )
		{
			do { sock = JISAllocator::AllocJIS(); } while( sock == 0 );
			sock->SetUserConnectionSocketIndex(index);
			JISList.PushTail(sock);

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
			if( sock->IsBerkleySocket() )
			{
				berkleyBindParams.port = bindLocalSockets[index].port;
				berkleyBindParams.hostAddress = (char*) bindLocalSockets[index].hostAddress;
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

				bindResult = ( (JISBerkley*) sock )->Bind(&berkleyBindParams,
					TRACE_FILE_AND_LINE_);

				if(
#if NET_SUPPORT_IPV6 ==0
					bindLocalSockets[index].socketFamily != AF_INET ||
#endif
					bindResult == JISBindResult_REQUIRES_NET_SUPPORT_IPV6_DEFINED )
				{
					JISAllocator::DeallocJIS(sock);
					DeallocJISList();
					JERROR << "Bind Failed (REQUIRES_NET_SUPPORT_IPV6_DEFINED) ! ";
					return SOCKET_FAMILY_NOT_SUPPORTED;
				}

				switch( bindResult )
				{
					case JISBindResult_FAILED_BIND_SOCKET:
						JISAllocator::DeallocJIS(sock);
						DeallocJISList();
						JERROR << "Bind Failed (FAILED_BIND_SOCKET) ! ";
						return SOCKET_PORT_ALREADY_IN_USE;
						break;

					case JISBindResult_FAILED_SEND_RECV_TEST:
						JISAllocator::DeallocJIS(sock);
						DeallocJISList();
						JERROR << "Bind Failed (FAILED_SEND_RECV_TEST) ! ";
						return SOCKET_FAILED_TEST_SEND_RECV;
						break;

					default:
						assert(bindResult == JISBindResult_SUCCESS);
						char str[256];
						sock->GetBoundAddress().ToString(true, str);
						JINFO << "Bind [" << str << "] Successfully";
						break;
				}
			} else
			{
				JWARNING << "Bind Failed (@TO-DO UNKNOWN BERKELY SOCKET) ! ";
				assert("@TO-DO UNKNOWN BERKELY SOCKET" && 0);
			}
#endif
		}

		assert(JISList.Size() == bindLocalSocketsCount);

		/// after binding, assign IPAddress port number
#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
		if( JISList[0]->IsBerkleySocket() )
		{
			for( index = 0; index < MAX_COUNT_LOCAL_IP_ADDR; index++ )
			{
				if( IPAddress[index] == JACKIE_INET_Address_Null ) break;
				IPAddress[index].SetPortHostOrder(( (JISBerkley*) JISList[0] )->GetBoundAddress().GetPortHostOrder());
			}
		}
#endif

		JISRecvParamsPool = JACKIE_INET::OP_NEW_ARRAY < MemoryPool < JISRecvParams,
			512, 8 >> ( JISList.Size(), TRACE_FILE_AND_LINE_ );
#if USE_SINGLE_THREAD == 0
		deAllocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < LockFreeQueue <
			JISRecvParams*, SERVER_QUEUE_PTR_SIZE >> ( JISList.Size(), TRACE_FILE_AND_LINE_ );
		allocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < LockFreeQueue <
			JISRecvParams*, SERVER_QUEUE_PTR_SIZE >> ( JISList.Size(), TRACE_FILE_AND_LINE_ );
#else
		deAllocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < RingBufferQueue
			< JISRecvParams*, SERVER_QUEUE_PTR_SIZE >> ( JISList.Size(), TRACE_FILE_AND_LINE_ );
		allocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < RingBufferQueue <
			JISRecvParams*, SERVER_QUEUE_PTR_SIZE >> ( JISList.Size(), TRACE_FILE_AND_LINE_ );
#endif

		/// setup connections list
		if( maxConnections == 0 )
		{
			// Don't allow more incoming connections than we have peers.
			if( maxIncomeConnections > maxConn ) maxIncomeConnections = maxConn;
			maxConnections = maxConn;

			remoteSystemList = JACKIE_INET::OP_NEW_ARRAY<RemoteEndPoint>(maxConnections, TRACE_FILE_AND_LINE_);

			// All entries in activeSystemList have valid pointers all the time.
			activeSystemList = JACKIE_INET::OP_NEW_ARRAY<RemoteEndPoint*>(maxConnections, TRACE_FILE_AND_LINE_);

			index = maxConnections*RemoteEndPointLookupHashMutiple;
			remoteSystemLookup = JACKIE_INET::OP_NEW_ARRAY<RemoteEndPointIndex*>(index, TRACE_FILE_AND_LINE_);
			memset((void**) remoteSystemLookup, 0, index*sizeof(RemoteEndPointIndex*));

			for( index = 0; index < maxConnections; index++ )
			{
				// remoteSystemList in Single thread
				remoteSystemList[index].isActive = false;
				remoteSystemList[index].systemAddress = JACKIE_INET_Address_Null;
				remoteSystemList[index].guid = JACKIE_INet_GUID_Null;
				remoteSystemList[index].myExternalSystemAddress = JACKIE_INET_Address_Null;
				remoteSystemList[index].status = RemoteEndPoint::NO_ACTION;
				remoteSystemList[index].MTUSize = defaultMTUSize;
				remoteSystemList[index].remoteSystemIndex = (SystemIndex) index;

#ifdef _DEBUG
				remoteSystemList[index].reliabilityLayer.ApplyNetworkSimulator(_packetloss, _minExtraPing, _extraPingVariance);
#endif
				activeSystemList[index] = &remoteSystemList[index];
			}
		}


		/// Setup Plugins 
		for( index = 0; index < pluginListTS.Size(); index++ )
		{
			pluginListTS[index]->OnRakPeerStartup();
		}
		for( index = 0; index < pluginListNTS.Size(); index++ )
		{
			pluginListNTS[index]->OnRakPeerStartup();
		}


		///  setup thread things
		endThreads = true;
		isNetworkUpdateThreadActive = false;
		if( endThreads )
		{
			ClearBufferedCommands();
			ClearSocketQueryOutputs();
			for( unsigned int Index = 0; Index < JISList.Size(); Index++ )
				ClearAllRecvParamsQs(index);

			firstExternalID = JACKIE_INET_Address_Null;
			updateCycleIsRunning = false;
			endThreads = false;

			/// Create recv threads
#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
#if USE_SINGLE_THREAD == 0
			/// this will create @bindLocalSocketsCount number of of recv threads 
			/// That is if you have two NICs, will create two recv threads to handle
			/// each of socket
			for( index = 0; index < bindLocalSocketsCount; index++ )
			{
				if( JISList[index]->IsBerkleySocket() )
				{
					if( CreateRecvPollingThread(threadPriority, index) != 0 )
					{
						End(0);
						return FAILED_TO_CREATE_RECV_THREAD;
					}
				}
			}

			/// Wait for the threads to activate. When they are active they will set these variables to true
			while( !isRecvPollingThreadActive.GetValue() ) JackieSleep(10);
#else
			/// we handle recv in this thread, that is we only have two threads in the app this recv thread and th other send thread
			isRecvPollingThreadActive.Increment();
#endif
#endif

			/// use another thread to charge of sending
			if( !isNetworkUpdateThreadActive )
			{
#if USE_SINGLE_THREAD == 0
				if( CreateNetworkUpdateThread(threadPriority) != 0 )
				{
					End(0);
					JERROR << "ServerApplication::Start() Failed (FAILED_TO_CREATE_SEND_THREAD) ! ";
					return FAILED_TO_CREATE_NETWORK_UPDATE_THREAD;
				}
				/// Wait for the threads to activate. When they are active they will set these variables to true
				while( !isNetworkUpdateThreadActive ) JackieSleep(10);
#else
				/// we only have one thread to handle recv and send so just simply set it to true
				isNetworkUpdateThreadActive = true;
#endif
			}
		}

		//#ifdef USE_THREADED_SEND
		//		RakNet::SendToThread::AddRef();
		//#endif

		JINFO << "Startup Application Succeeds....";
		return ALREADY_STARTED;
	}

	void ServerApplication::End(unsigned int blockDuration,
		unsigned char orderingChannel,
		PacketSendPriority disconnectionNotificationPriority)
	{

	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	inline void ServerApplication::ReclaimOneJISRecvParams(JISRecvParams *s, UInt32 index)
	{
		//JINFO << "Reclaim One JISRecvParams";
		CHECK_EQ(deAllocRecvParamQ[index].PushTail(s), true);
	}
	inline void ServerApplication::ReclaimAllJISRecvParams(UInt32 Index)
	{
		//JINFO << "Reclaim All JISRecvParams";
		JISRecvParams* recvParams = 0;
		for( unsigned int index = 0; index < deAllocRecvParamQ[Index].Size(); index++ )
		{
			CHECK_EQ(deAllocRecvParamQ[Index].PopHead(recvParams), true);
			JISRecvParamsPool[Index].Reclaim(recvParams);
		}
	}
	inline JISRecvParams * ServerApplication::AllocJISRecvParams(UInt32 Index)
	{
		//JINFO << "AllocJISRecvParams";
		JISRecvParams* ptr = 0;
		do { ptr = JISRecvParamsPool[Index].Allocate(); } while( ptr == 0 );
		return ptr;
	}
	void ServerApplication::ClearAllRecvParamsQs(UInt32 index)
	{
		JISRecvParams *recvParams = 0;
		for( unsigned int i = 0; i < allocRecvParamQ[index].Size(); i++ )
		{
			CHECK_EQ(allocRecvParamQ[index].PopHead(recvParams), true);
			if( recvParams->data != 0 ) jackieFree_Ex(recvParams->data, TRACE_FILE_AND_LINE_);
			JISRecvParamsPool[index].Reclaim(recvParams);
		}
		for( unsigned int i = 0; i < deAllocRecvParamQ[index].Size(); i++ )
		{
			CHECK_EQ(deAllocRecvParamQ[index].PopHead(recvParams), true);
			if( recvParams->data != 0 ) jackieFree_Ex(recvParams->data, TRACE_FILE_AND_LINE_);
			JISRecvParamsPool[index].Reclaim(recvParams);
		}
		allocRecvParamQ[index].Clear();
		deAllocRecvParamQ[index].Clear();
		JISRecvParamsPool[index].Clear();
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	inline void ServerApplication::ReclaimOneCommand(Command* s)
	{
		//JINFO << "Reclaim One Command";
		CHECK_EQ(deAllocCommandQ.PushTail(s), true);
	}
	void ServerApplication::ReclaimAllCommands()
	{
		//JINFO << "Reclaim All Commands";

		Command* bufferedCommand = 0;
		for( unsigned int index = 0; index < deAllocCommandQ.Size(); index++ )
		{
			CHECK_EQ(
				deAllocCommandQ.PopHead(bufferedCommand),
				true);
			CHECK_NOTNULL(bufferedCommand);
			commandPool.Reclaim(bufferedCommand);
		}
	}
	Command* ServerApplication::AllocCommand()
	{
		JINFO << "AllocBufferedCommand";
		Command* ptr = 0;
		do { ptr = commandPool.Allocate(); } while( ptr == 0 );
		return ptr;
	}
	void ServerApplication::ClearBufferedCommands(void)
	{
		Command *bcs = 0;

		/// first reclaim the elem in 
		for( unsigned int i = 0; i < allocCommandQ.Size(); i++ )
		{
			CHECK_EQ(allocCommandQ.PopHead(bcs), true);
			CHECK_NOTNULL(bcs);
			if( bcs->data != 0 ) jackieFree_Ex(bcs->data, TRACE_FILE_AND_LINE_);
			commandPool.Reclaim(bcs);
		}
		for( unsigned int i = 0; i < deAllocCommandQ.Size(); i++ )
		{
			CHECK_EQ(deAllocCommandQ.PopHead(bcs), true);
			CHECK_NOTNULL(bcs);
			if( bcs->data != 0 ) jackieFree_Ex(bcs->data, TRACE_FILE_AND_LINE_);
			commandPool.Reclaim(bcs);
		}
		deAllocCommandQ.Clear();
		allocCommandQ.Clear();
		commandPool.Clear();
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////// Clear and Allocate ////////////////////////////////////////////////
	void ServerApplication::InitIPAddress(void)
	{
		assert(IPAddress[0] == JACKIE_INET_Address_Null);
		JackieINetSocket::GetMyIP(IPAddress);

		// Sort the addresses from lowest to highest
		int startingIdx = 0;
		while( startingIdx < MAX_COUNT_LOCAL_IP_ADDR - 1 &&
			IPAddress[startingIdx] != JACKIE_INET_Address_Null )
		{
			int lowestIdx = startingIdx;
			for( int curIdx = startingIdx + 1; curIdx < MAX_COUNT_LOCAL_IP_ADDR - 1 && IPAddress[curIdx] != JACKIE_INET_Address_Null; curIdx++ )
			{
				if( IPAddress[curIdx] < IPAddress[startingIdx] )
				{
					lowestIdx = curIdx;
				}
			}
			if( startingIdx != lowestIdx )
			{
				JACKIE_INET_Address temp = IPAddress[startingIdx];
				IPAddress[startingIdx] = IPAddress[lowestIdx];
				IPAddress[lowestIdx] = temp;
			}
			++startingIdx;
		}
	}
	void ServerApplication::DeallocJISList(void)
	{
		for( unsigned int index = 0; index < JISList.Size(); index++ )
		{
			if( JISList[index] != 0 ) JISAllocator::DeallocJIS(JISList[index]);
		}
		JISList.Clear();
	}
	void ServerApplication::ClearSocketQueryOutputs(void)
	{
		socketQueryOutput.Clear();
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////// AllocPacket ///////////////////////////////////////////////
	Packet* ServerApplication::AllocPacket(unsigned int dataSize)
	{
		//JINFO << "Alloc Packet";
		Packet *p = 0;
		do { p = packetPool.Allocate(); } while( p == 0 );

		//p = new ( (void*) p ) Packet; we do not need call default ctor
		p->data = (char*) jackieMalloc_Ex(dataSize, TRACE_FILE_AND_LINE_);
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->isAllocatedFromPool = true;
		p->guid = JACKIE_INet_GUID_Null;
		p->wasGeneratedLocally = false;

		return p;
	}
	Packet* ServerApplication::AllocPacket(unsigned dataSize, char *data)
	{
		//JINFO << "Alloc Packet";
		Packet *p = 0;
		do { p = packetPool.Allocate(); } while( p == 0 );

		//p = new ( (void*) p ) Packet; no custom ctor so no need to call default ctor
		p->data = data;
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->isAllocatedFromPool = true;
		p->guid = JACKIE_INet_GUID_Null;
		p->wasGeneratedLocally = false;

		return p;
	}
	void ServerApplication::ReclaimAllPackets()
	{
		//JINFO << "Reclaim All Packets";

		Packet* packet;
		for( unsigned int index = 0; index < deAllocPacketQ.Size(); index++ )
		{
			CHECK_EQ(deAllocPacketQ.PopHead(packet), true);
			if( packet->isAllocatedFromPool )
			{
				jackieFree_Ex(packet->data, TRACE_FILE_AND_LINE_);
				//packet->~Packet(); no custom dtor so no need to call default dtor
				packetPool.Reclaim(packet);
			} else
			{
				jackieFree_Ex(packet, TRACE_FILE_AND_LINE_);
			}
		}
	}
	void ServerApplication::ReclaimOnePacket(Packet *packet)
	{
		//JINFO << "Reclaim One Packet";
		CHECK_EQ(deAllocPacketQ.PushTail(packet), true);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////// thread ///////////////////////////////////////////////////
	int ServerApplication::CreateRecvPollingThread(int threadPriority, UInt32 index)
	{
		char* arg = (char*) jackieMalloc_Ex(sizeof(ServerApplication*) + sizeof(index), TRACE_FILE_AND_LINE_);
		ServerApplication* serv = this;
		memcpy(arg, &serv, sizeof(ServerApplication*));
		memcpy(arg + sizeof(ServerApplication*), (char*) &index, sizeof(index));
		return JACKIE_Thread::Create(JACKIE_INET::RunRecvCycleLoop, arg, threadPriority);
	}
	int ServerApplication::CreateNetworkUpdateThread(int threadPriority)
	{
		return JACKIE_Thread::Create(JACKIE_INET::RunNetworkUpdateCycleLoop, this, threadPriority);
	}
	void ServerApplication::StopRecvPollingThread()
	{
		endThreads = true;
#if USE_SINGLE_THREAD == 0
		for( UInt32 i = 0; i < JISList.Size(); i++ )
		{
			if( JISList[i]->IsBerkleySocket() )
			{
				JISBerkley* sock = (JISBerkley*) JISList[i];
				if( sock->GetBindingParams()->isBlocKing == USE_BLOBKING_SOCKET )
				{
					/// try to send 0 data to let recv thread keep running
					/// to detect the isRecvPollingThreadActive === false so that stop the thread
					char zero[ ] = "This is used to Stop Recv Thread";
					JISSendParams sendParams = { zero, sizeof(zero), 0, sock->GetBoundAddress(), 0 };
					sock->Send(&sendParams, TRACE_FILE_AND_LINE_);
					TimeMS timeout = GetTimeMS() + 1000;
					while( isRecvPollingThreadActive.GetValue() > 0 && GetTimeMS() < timeout )
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
	////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	void ServerApplication::ProcessOneRecvParam(JISRecvParams* recvParams)
	{
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

		CHECK_NE(recvParams->senderINetAddress.GetPortHostOrder(), 0);

		bool isOfflinerecvParams = true;
		if( ProcessOneOfflineRecvParam(recvParams, &isOfflinerecvParams) ) return;

		/// See if this datagram came from a connected system
		RemoteEndPoint* remoteEndPoint =
			GetRemoteEndPoint(recvParams->senderINetAddress, true, true);
		if( remoteEndPoint != 0 ) // if this datagram comes from connected system
		{
			if( !isOfflinerecvParams )
			{
				remoteEndPoint->reliabilityLayer.ProcessJISRecvParamsFromConnectedEndPoint(this, remoteEndPoint->MTUSize);
			}
		} else
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
		unsigned int index;

		const char* strAddr = recvParams->senderINetAddress.ToString();

		return false;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////


	void ServerApplication::ProcessConnectionRequestCancelQ(void)
	{
		JACKIE_INET_Address connReqCancelAddr;
		ConnectionRequest* connReq = 0;
		for( unsigned int index = 0; index < connReqCancelQ.Size(); index++ )
		{
			CHECK_EQ(connReqCancelQ.PopHead(connReqCancelAddr), true);

			connReqQMutex.Lock();
			/// Cancel pending connection attempt, if there is one
			for( unsigned int i = 0; i < connReqQ.Size(); i++ )
			{
				if( connReqQ[i]->receiverAddr == connReqCancelAddr )
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
			connReqQMutex.Unlock();
		}
	}
	void ServerApplication::ProcessAllocCommandQ(TimeUS& timeUS, TimeMS& timeMS)
	{
		Command* bufferedCommand = 0;
		RemoteEndPoint* remoteEndPoint = 0;

		/// process command queue
		for( unsigned int index = 0; index < allocCommandQ.Size(); index++ )
		{
			JINFO << "ProcessAllocCommandQ in loop";

			/// no need to check if bufferedCommand == 0, because we never push 0 pointer
			CHECK_EQ(allocCommandQ.PopHead(bufferedCommand), true);
			CHECK_NOTNULL(bufferedCommand);
			CHECK_NOTNULL(bufferedCommand->data);

			switch( bufferedCommand->command )
			{
				case Command::BCS_SEND:
					JINFO << "BCS_SEND";
					/// GetTime is a very slow call so do it once and as late as possible
					if( timeUS == 0 )
					{
						timeUS = GetTimeUS();
						timeMS = (TimeMS) ( timeUS / (TimeUS) 1000 );
					}
					/// send data stored in this bc right now
					if( SendRightNow(timeUS, true, bufferedCommand) == false )
						jackieFree_Ex(bufferedCommand->data, TRACE_FILE_AND_LINE_);
					/// Set the new connection state AFTER we call sendImmediate in case we are 
					/// setting it to a disconnection state, which does not allow further sends
					if( bufferedCommand->repStatus != RemoteEndPoint::NO_ACTION )
					{
						remoteEndPoint = GetRemoteEndPoint(
							bufferedCommand->systemIdentifier, true, true);
						if( remoteEndPoint != 0 )
							remoteEndPoint->status = bufferedCommand->repStatus;
					}
					break;
				case Command::BCS_CLOSE_CONNECTION:
					JINFO << "BCS_CLOSE_CONNECTION";
					CloseConnectionInternally(false, true, bufferedCommand);
					break;
				case Command::BCS_CHANGE_SYSTEM_ADDRESS: //re-rout
					remoteEndPoint = GetRemoteEndPoint(
						bufferedCommand->systemIdentifier, true, true);
					if( remoteEndPoint != 0 )
					{
						Int32 existingSystemIndex =
							GetRemoteEndPointIndex(remoteEndPoint->systemAddress);
						RefRemoteEndPoint(
							bufferedCommand->systemIdentifier.systemAddress, existingSystemIndex);
					}
					break;
				case Command::BCS_GET_SOCKET:
					JINFO << "BCS_GET_SOCKET";
					break;
				default:
					JERROR << "Not Found Matched BufferedCommand";
					break;
			}

			ReclaimOneCommand(bufferedCommand);
		}

	}
	void ServerApplication::ProcessAllocJISRecvParamsQ(void)
	{
		JISRecvParams* recvParams = 0;
		for( unsigned int outter = 0; outter < JISList.Size(); outter++ )
		{
			for( unsigned int inner = 0; inner < allocRecvParamQ[outter].Size(); inner++ )
			{
				/// no need to check if recvParams == 0, because we never push 0 pointer
				CHECK_EQ(allocRecvParamQ[outter].PopHead(recvParams), true);
				ProcessOneRecvParam(recvParams);
				ReclaimOneJISRecvParams(recvParams, inner);
			}
		}
	}

	/// @TO-DO
	void ServerApplication::ProcessConnectionRequestQ(TimeUS& timeUS, TimeMS& timeMS)
	{
		if( !connReqQ.IsEmpty() )
		{
			Time timeMS;
			if( timeUS == 0 )
			{
				timeUS = GetTimeUS();
				timeMS = (TimeMS) ( timeUS / (TimeUS) 1000 );
			}

			ConnectionRequest *connReq;
			bool isMaxConnAttemptTimes;
			bool isNllAdress;

#if USE_SINGLE_THREAD == 0
			connReqQMutex.Lock();
#endif
			for( unsigned int index = 0; index < connReqQ.Size(); index++ )
			{
				connReq = connReqQ[index];
				if( connReq->nextRequestTime < timeMS )
				{
					connReq->requestsMade == connReq->sendConnectionAttemptCount + 1 ?
						isMaxConnAttemptTimes = true : isMaxConnAttemptTimes = false;

					connReq->receiverAddr == JACKIE_INET_Address_Null ?
						isNllAdress = true : isNllAdress = false;

					/// If too many requests made or a hole then remove this if possible,
					/// otherwise invalidate it
					if( isMaxConnAttemptTimes || isNllAdress )
					{

						/// free data inside conn req
						if( connReq->data != 0 )
						{
							jackieFree_Ex(connReq->data, TRACE_FILE_AND_LINE_);
							connReq->data = 0;
						}

						/// Tell USER connection attempt failed
						if( isMaxConnAttemptTimes && !isNllAdress &&
							connReq->actionToTake == ConnectionRequest::CONNECT )
						{
							char msgid;
							Packet* packet = AllocPacket(sizeof(msgid), &msgid);
							packet->data[0] = ID_CONNECTION_ATTEMPT_FAILED;
							packet->systemAddress = connReq->receiverAddr;
							CHECK_EQ(allocPacketQ.PushTail(packet), true);
						}

#if LIBCAT_SECURITY==1
						CAT_AUDIT_PRINTF("AUDIT: Connection attempt FAILED so deleting rcs->client_handshake object %x\n", rcs->client_handshake);
						JACKIE_INET::OP_DELETE(connReq->client_handshake,
							TRACE_FILE_AND_LINE_);
#endif
						JACKIE_INET::OP_DELETE(connReq, TRACE_FILE_AND_LINE_);

						/// remove this conn request fron  queue
						connReqQ.RemoveAtIndex(index);

					} else ///  isMaxConnAttemptTimes and isNllAdress are both false
					{
						/// more times try to request connection, less mtu used
						int MTUSizeIndex = connReq->requestsMade /
							( connReq->sendConnectionAttemptCount / mtuSizesCount );
						if( MTUSizeIndex >= mtuSizesCount )
							MTUSizeIndex = mtuSizesCount - 1;

						connReq->requestsMade++;
						connReq->nextRequestTime = timeUS + connReq->timeoutReqConn;

						/// @TO-DO
						JackieStream bitStream;
						//bitStream.Write((MessageID) ID_OPEN_CONNECTION_REQUEST_1);
						//bitStream.WriteAlignedBytes((const unsigned char*) OFFLINE_MESSAGE_DATA_ID, sizeof(OFFLINE_MESSAGE_DATA_ID));
						//bitStream.Write((MessageID) RAKNET_PROTOCOL_VERSION);
						//bitStream.PadWithZeroToByteLength(mtuSizes[MTUSizeIndex] - UDP_HEADER_SIZE);

#ifdef _DEBUG
						char str[256];
						connReq->receiverAddr.ToString(true, str);
						JINFO << "The " << connReq->requestsMade
							<< " times to try to connect to remote sever [" << str << "]";
#endif

						/// @TO-DO i am now in here

					}
				}
			}
#if USE_SINGLE_THREAD == 0
			connReqQMutex.Unlock();
#endif
		}
	}

	/// @TO-DO
	void ServerApplication::AdjustTimestamp(Packet*& incomePacket) const
	{
		JINFO << "@TO-DO AdjustTimestamp()";

		if( (unsigned char) incomePacket->data[0] == ID_TIMESTAMP )
		{
			if( incomePacket->length >= sizeof(char) + sizeof(Time) )
			{
				char* data = &incomePacket->data[sizeof(char)];
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


	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const JACKIE_INET_Address&
		sa, bool neededBySendThread, bool onlyWantActiveEndPoint) const
	{
		if( sa == JACKIE_INET_Address_Null ) return 0;

		if( neededBySendThread )
		{
			Int32 index = GetRemoteEndPointIndex(sa);
			if( index != -1 )
			{
				if( !onlyWantActiveEndPoint || remoteSystemList[index].isActive )
				{
					CHECK_EQ(remoteSystemList[index].systemAddress, sa);
					return &remoteSystemList[index];
				}
			}
		} else
		{
			/// Active EndPoints take priority.  But if matched end point is inactice, 
			/// return the first EndPoint match found
			Int32 inActiveEndPointIndex = -1;
			for( unsigned int index = 0; index < maxConnections; index++ )
			{
				if( remoteSystemList[index].systemAddress == sa )
				{
					if( remoteSystemList[index].isActive )
						return &remoteSystemList[index];
					else if( inActiveEndPointIndex == -1 )
						inActiveEndPointIndex = index;
				}
			}

			/// matched end pint was found but it is inactive
			if( inActiveEndPointIndex != -1 && !onlyWantActiveEndPoint )
				return &remoteSystemList[inActiveEndPointIndex];
		}

		// no matched end point found
		return 0;
	}
	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const
		JACKIE_INET_Address_GUID_Wrapper& senderWrapper, bool neededBySendThread,
		bool onlyWantActiveEndPoint) const
	{
		if( senderWrapper.guid != JACKIE_INet_GUID_Null )
			return GetRemoteEndPoint(senderWrapper.guid, onlyWantActiveEndPoint);
		else
			return GetRemoteEndPoint(senderWrapper.systemAddress, neededBySendThread,
			onlyWantActiveEndPoint);
	}
	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const JACKIE_INet_GUID&
		senderGUID, bool onlyWantActiveEndPoint) const
	{
		if( senderGUID == JACKIE_INet_GUID_Null ) return 0;
		for( unsigned int i = 0; i < maxConnections; i++ )
		{
			if( remoteSystemList[i].guid == senderGUID &&
				( onlyWantActiveEndPoint == false || remoteSystemList[i].isActive ) )
			{
				return remoteSystemList + i;
			}
		}
		return 0;
	}
	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const JACKIE_INET_Address& sa) const
	{
		Int32 index = GetRemoteEndPointIndex(sa);
		if( index == -1 ) return 0;
		return remoteSystemList + index;
	}
	Int32 ServerApplication::GetRemoteEndPointIndex(const JACKIE_INET_Address &sa) const
	{
		unsigned int hashindex = JACKIE_INET_Address::ToHashCode(sa);
		hashindex = hashindex % ( maxConnections * RemoteEndPointLookupHashMutiple );
		RemoteEndPointIndex* curr = remoteSystemLookup[hashindex];
		while( curr != 0 )
		{
			if( remoteSystemList[curr->index].systemAddress == sa )
				return curr->index;
			curr = curr->next;
		}
		return  -1;
	}


	void ServerApplication::RefRemoteEndPoint(const JACKIE_INET_Address &sa, unsigned int index)
	{
		RemoteEndPoint* remote = remoteSystemList + index;
		JACKIE_INET_Address old = remote->systemAddress;
		if( old != JACKIE_INET_Address_Null )
		{
			// The system might be active if rerouting
			CHECK_EQ(remoteSystemList[index].isActive, false);
			// Remove the reference if the reference is pointing to this inactive system
			if( GetRemoteEndPoint(old) == remote )
			{
				DeRefRemoteEndPoint(old);
			}
		}

		DeRefRemoteEndPoint(sa);
		remoteSystemList[index].systemAddress = sa;

		unsigned int hashindex = JACKIE_INET_Address::ToHashCode(sa);
		hashindex = hashindex % ( maxConnections * RemoteEndPointLookupHashMutiple );

		RemoteEndPointIndex *rsi = 0;
		do { rsi = remoteSystemIndexPool.Allocate(); } while( rsi == 0 );

		if( remoteSystemLookup[hashindex] == 0 )
		{
			rsi->next = 0;
			rsi->index = index;
			remoteSystemLookup[hashindex] = rsi;
		} else
		{
			RemoteEndPointIndex *cur = remoteSystemLookup[hashindex];
			while( cur->next != 0 ) { cur = cur->next; } /// move to last one
			cur->next = rsi;
			rsi->next = 0;
			rsi->index = index;
		}

	}
	void ServerApplication::DeRefRemoteEndPoint(const JACKIE_INET_Address &sa)
	{
		unsigned int hashindex = JACKIE_INET_Address::ToHashCode(sa);
		hashindex = hashindex % ( maxConnections * RemoteEndPointLookupHashMutiple );

		RemoteEndPointIndex *cur = remoteSystemLookup[hashindex];
		RemoteEndPointIndex *last = 0;

		while( cur != 0 )
		{
			if( remoteSystemList[cur->index].systemAddress == sa )
			{
				if( last == 0 )
				{
					remoteSystemLookup[hashindex] = cur->next;
				} else
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
		JINFO << "@TO-DO::SendRightNow()";
		return true;
	}
	//@TO-DO
	void ServerApplication::CloseConnectionInternally(bool sendDisconnectionNotification, bool performImmediate, Command* bufferedCommand)
	{
		JINFO << "@TO-DO::CloseConnectionInternally()";
	}



	void ServerApplication::PacketGoThroughPluginCBs(Packet*& incomePacket)
	{
		unsigned int i;
		for( i = 0; i < pluginListTS.Size(); i++ )
		{
			switch( incomePacket->data[0] )
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

		for( i = 0; i < pluginListNTS.Size(); i++ )
		{
			switch( incomePacket->data[0] )
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
		unsigned int i;
		PluginActionType pluginResult;

		for( i = 0; i < pluginListTS.Size(); i++ )
		{
			pluginResult = pluginListTS[i]->OnRecvPacket(incomePacket);
			if( pluginResult == PROCESSED_BY_ME_THEN_DEALLOC )
			{
				ReclaimOnePacket(incomePacket);
				// Will do the loop again and get another incomePacket
				incomePacket = 0;
				break; // break out of the enclosing forloop
			} else if( pluginResult == HOLD_ON_BY_ME_NOT_DEALLOC )
			{
				incomePacket = 0;
				break;
			}
		}

		for( i = 0; i < pluginListNTS.Size(); i++ )
		{
			pluginResult = pluginListNTS[i]->OnRecvPacket(incomePacket);
			if( pluginResult == PROCESSED_BY_ME_THEN_DEALLOC )
			{
				ReclaimOnePacket(incomePacket);
				// Will do the loop again and get another incomePacket
				incomePacket = 0;
				break; // break out of the enclosing forloop
			} else if( pluginResult == HOLD_ON_BY_ME_NOT_DEALLOC )
			{
				incomePacket = 0;
				break;
			}
		}
	}
	void ServerApplication::UpdatePlugins(void)
	{
		unsigned int i;
		for( i = 0; i < pluginListTS.Size(); i++ )
		{
			pluginListTS[i]->Update();
		}
		for( i = 0; i < pluginListNTS.Size(); i++ )
		{
			pluginListNTS[i]->Update();
		}
	}


	Packet* ServerApplication::GetPacketOnce(void)
	{
		TIMED_FUNC();
#if USE_SINGLE_THREAD != 0
		RunRecvCycleOnce(0);
		RunNetworkUpdateCycleOnce();
#endif
		return RunGetPacketCycleOnce();
	}
	Packet* ServerApplication::RunGetPacketCycleOnce(void)
	{
		Packet *incomePacket = 0;

		/// UPDATE all plugins
		UpdatePlugins();

		/// Pop out one Packet from queue
		if( allocPacketQ.Size() > 0 )
		{
			//////////////////////////////////////////////////////////////////////////
			/// Get one income packet from bufferedPacketsQueue
			CHECK_EQ(allocPacketQ.PopHead(incomePacket), true);
			CHECK_NOTNULL(incomePacket->data);
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
		TIMED_FUNC();
		//// @NOTICE I moved this code to JISbEKELY::RecvFrom()
		//// unsigned int index;
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
		ReclaimAllCommands();

		//do { recvParams = JISRecvParamsPool[index].Allocate(); } while( recvParams == 0 );
		recvParams = AllocJISRecvParams(index);
		recvParams->socket = JISList[index];

		if( ( (JISBerkley*) JISList[index] )->RecvFrom(recvParams) > 0 )
		{
			CHECK_EQ(allocRecvParamQ[index].PushTail(recvParams), true);

			if( incomeDatagramEventHandler != 0 )
			{
				if( !incomeDatagramEventHandler(recvParams) ) JWARNING << "incomeDatagramEventHandler(recvStruct) Failed.";
			}

#if USE_SINGLE_THREAD == 0
			if( allocRecvParamQ[index].Size() > 0 ) quitAndDataEvents.TriggerEvent();
#endif

		} else
		{
			JISRecvParamsPool[index].Reclaim(recvParams);
		}

	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::RunRecvCycleLoop)
	{
		ServerApplication *serv = *(ServerApplication**) arguments;
		UInt32 index = *( (UInt32*) ( (char*) arguments + sizeof(ServerApplication*) ) );

		serv->isRecvPollingThreadActive.Increment();

		JINFO << "Recv thread " << "is running in backend....";
		while( !serv->endThreads )
		{
			serv->RunRecvCycleOnce(index);
			JackieSleep(1000);
		}
		JINFO << "Recv polling thread Stops....";

		serv->isRecvPollingThreadActive.Decrement();
		jackieFree_Ex(arguments, TRACE_FILE_AND_LINE_);
		return 0;
	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::RunNetworkUpdateCycleLoop)
	{
		ServerApplication *serv = (ServerApplication*) arguments;
		serv->isNetworkUpdateThreadActive = true;

		JINFO << "Send polling thread is running in backend....";
		while( !serv->endThreads )
		{
			/// Normally, buffered sending packets go out every other 10 ms.
			/// or TriggerEvent() is called by recv thread
			serv->RunNetworkUpdateCycleOnce();
			serv->quitAndDataEvents.WaitEvent(5);
			JackieSleep(1000);
		}
		JINFO << "Send polling thread Stops....";

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
	UInt64 ServerApplication::Get64BitUniqueRandomNumber(void)
	{
		UInt64 g = GetTimeUS();
		TimeUS lastTime, thisTime, diff;
		unsigned char diffByte = 0;
		// Sleep a small random time, then use the last 4 bits as a source of randomness
		for( int j = 0; j < 4; j++ )
		{
			diffByte = 0;
			for( int index = 0; index < 4; index++ )
			{
				lastTime = GetTimeUS();
				JackieSleep(1);
				thisTime = GetTimeUS();
				diff = thisTime - lastTime;
				diffByte ^= (unsigned char) ( ( diff & 15 ) << ( index * 2 ) ); ///0xF = 1111 = 15
				if( index == 3 ) diffByte ^= (unsigned char) ( ( diff & 15 ) >> 2 );
			}
			( (unsigned char*) &g )[4 + j] ^= diffByte;
		}
		return g;
	}

}