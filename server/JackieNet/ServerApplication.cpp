#include "ServerApplication.h"
#include "WSAStartupSingleton.h"
#include "RandomSeedCreator.h"
#include "EasyLog.h"

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
	static Ultils::RandomSeedCreator rnr;
	static const unsigned int RemoteEndPointLookupHashMutiple = 8;
	static const int mtuSizesCount = 3;
	static const int mtuSizes[mtuSizesCount] = { MAXIMUM_MTU_SIZE, 1200, 576 };
	/// I set this because I limit ID_CONNECTION_REQUEST to 512 bytes, 
	/// and the password is appended to that packet.
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


	//////////////////////////////////////////////////////////////////////////
	ServerApplication::ServerApplication()
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

#if USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
		quitAndDataEvents.Init();
#endif

		GenerateGUID();
		ResetSendReceipt();
	}
	ServerApplication::~ServerApplication() { }
	//////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////
	JACKIE_INET::StartupResult ServerApplication::Start(UInt32 maxConn,
		JACKIE_LOCAL_SOCKET *bindLocalSockets,
		UInt32 bindLocalSocketsCount,
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

		/////////////////////////////// Start to bind given sockets ///////////////////////////////////
		UInt32 index;
		JACKIE_INet_Socket* sock;
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

#ifdef USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
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
						JINFO << "Bind [" << sock->GetBoundAddress().ToString() << "] Successfully";
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
		/////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////// setup connections list ///////////////////////////////
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
				remoteSystemList[index].connectMode = RemoteEndPoint::NO_ACTION;
				remoteSystemList[index].MTUSize = defaultMTUSize;
				remoteSystemList[index].remoteSystemIndex = (SystemIndex) index;

#ifdef _DEBUG
				remoteSystemList[index].reliabilityLayer.ApplyNetworkSimulator(_packetloss, _minExtraPing, _extraPingVariance);
#endif
				activeSystemList[index] = &remoteSystemList[index];
			}
		}
		//////////////////////////////////////////////////////////////////////////


		////////////////////////////// Setup Plugins ///////////////////////////////
		for( index = 0; index < pluginListTS.Size(); index++ )
		{
			pluginListTS[index]->OnRakPeerStartup();
		}
		for( index = 0; index < pluginListNTS.Size(); index++ )
		{
			pluginListNTS[index]->OnRakPeerStartup();
		}
		//////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////// setup thread things //////////////////////////////////////
		endSendRecvThreads = true;
		isRecvPollingThreadActive = isSendPollingThreadActive = false;
		if( endSendRecvThreads )
		{
			ClearBufferedCommands();
			ClearBufferedRecvParams();
			ClearSocketQueryOutputs();

			firstExternalID = JACKIE_INET_Address_Null;
			updateCycleIsRunning = false;
			endSendRecvThreads = false;

#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
#ifdef USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
			// this will create bindLocalSocketsCount of recv threads which is wrong
			// we only need one recv thread to recv data from all binded sockets
			//for( index = 0; index < bindLocalSocketsCount; index++ )
			//{
			//	if( JISList[index]->IsBerkleySocket() )
			// if( CreateRecvPollingThread(threadPriority) != 0 )
			//{
			//	End(0);
			//	return FAILED_TO_CREATE_NETWORK_THREAD;
			//}
			//}

			if( CreateRecvPollingThread(threadPriority) != 0 )
			{
				End(0);
				JERROR << "ServerApplication::Start() Failed (FAILED_TO_CREATE_SEND_THREAD) ! ";
				return FAILED_TO_CREATE_RECV_THREAD;
			}
			/// Wait for the threads to activate. When they are active they will set these variables to true
			while( !isRecvPollingThreadActive ) JACKIE_Sleep(10);
#endif
#endif

			/// use another thread to charge of sending
			if( !isSendPollingThreadActive )
			{
#ifdef USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
				if( CreateSendPollingThread(threadPriority) != 0 )
				{
					End(0);
					JERROR << "ServerApplication::Start() Failed (FAILED_TO_CREATE_SEND_THREAD) ! ";
					return FAILED_TO_CREATE_SEND_THREAD;
				}
				/// Wait for the threads to activate. When they are active they will set these variables to true
				while( !isSendPollingThreadActive ) JACKIE_Sleep(10);
#endif
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////

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
	////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////// JISRecvParams ////////////////////////////////
	inline void ServerApplication::OnJISRecv(JISRecvParams *recvStruct)
	{
		JINFO << "Recv " << recvStruct->bytesRead << " bytes of data [" << recvStruct->data << "]";

		if( incomeDatagramEventHandler != 0 && !incomeDatagramEventHandler(recvStruct) ) { return; }

		bufferedRecvParamQueue.PushTail(recvStruct);
		CHECK_EQ(bufferedRecvParamQueue.PopHead(), recvStruct);

#ifdef USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
		quitAndDataEvents.TriggerEvent();
#endif

	}
	inline void ServerApplication::ReclaimJISRecvParams(JISRecvParams *s)
	{
		assert(s != 0);
		JISRecvParamsPool.Reclaim(s);
	}
	inline JISRecvParams * ServerApplication::AllocJISRecvParams()
	{
		JISRecvParams *s = 0;
		do { s = JISRecvParamsPool.Allocate(); } while( s == 0 );
		assert(s != 0);
		return s;
	}
	//////////////////////////////////////////////////////////////////////////


	/////////////////////////////// Clear and Allocate ////////////////////////////
	void ServerApplication::InitIPAddress(void)
	{
		assert(IPAddress[0] == JACKIE_INET_Address_Null);
		JACKIE_INet_Socket::GetMyIP(IPAddress);

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
		if( JISList.Size() == 0 || JISList.AllocationSize() == 0 ) return;
		for( unsigned int index = 0; index < JISList.Size(); index++ )
		{
			if( JISList[index] != 0 ) JISAllocator::DeallocJIS(JISList[index]);
		}
		JISList.Clear();
	}
	void ServerApplication::ClearBufferedCommands(void)
	{
		BufferedCommand *bcs;
		while( ( bcs = bufferedCommands.PopHead() ) != 0 )
		{
			if( bcs->data != 0 ) rakFree_Ex(bcs->data, TRACE_FILE_AND_LINE_);
			bufferedCommands.Deallocate(bcs, TRACE_FILE_AND_LINE_);
		}
		bufferedCommands.Clear(TRACE_FILE_AND_LINE_);
	}
	void ServerApplication::ClearSocketQueryOutputs(void)
	{
		socketQueryOutput.Clear(TRACE_FILE_AND_LINE_);
	}
	void ServerApplication::ClearBufferedRecvParams(void)
	{
		bufferedRecvParamQueue.Clear();
	}
	////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////// AllocPacket ///////////////////////////////////
	Packet* ServerApplication::AllocPacket(unsigned int dataSize,
		unsigned int threadType)
	{
		Packet *p = 0;
		p = ( threadType == 0 ) ?
			recvPacketAllocationPool.Allocate() :
			sendPacketAllocationPool.Allocate();

		assert(p != 0);
		p = new ( (void*) p ) Packet;
		p->data = (char*) rakMalloc_Ex(dataSize, TRACE_FILE_AND_LINE_);
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->deleteData = true;
		p->guid = JACKIE_INet_GUID_Null;
		p->wasGeneratedLocally = false;
		return p;
	}
	Packet* ServerApplication::AllocPacket(unsigned dataSize,
		char *data, unsigned int threadType)
	{
		Packet *p = 0;
		p = ( threadType == 0 ) ?
			recvPacketAllocationPool.Allocate() :
			sendPacketAllocationPool.Allocate();

		assert(p != 0);
		p = new ( (void*) p ) Packet;
		p->data = data;
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->deleteData = true;
		p->guid = JACKIE_INet_GUID_Null;
		p->wasGeneratedLocally = false;
		return p;
	}
	//////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////// thread //////////////////////////////////////
	int ServerApplication::CreateRecvPollingThread(int threadPriority)
	{
		JINFO << "Start to Create Recv Polling Thread ......";
		return JACKIE_Thread::Create(JACKIE_INET::RunRecvCycleLoop, this, threadPriority);
	}
	int ServerApplication::CreateSendPollingThread(int threadPriority)
	{
		JINFO << "Start to Create Send Polling Thread ......";
		return JACKIE_Thread::Create(JACKIE_INET::RunSendCycleLoop, this, threadPriority);
	}
	void ServerApplication::BlockOnStopRecvPollingThread(JISBerkley* sock)
	{
		endSendRecvThreads = true;

		/// Change recvfrom to unbloking
		unsigned int zero = 0;
		JISSendParams sendParams = { (char*) &zero, sizeof(zero), 0,
			sock->GetBoundAddress(), 0 };
		sock->Send(&sendParams, TRACE_FILE_AND_LINE_);

		TimeMS timeout = ::GetTimeMS() + 1000;
		while( isRecvPollingThreadActive && GetTimeMS() < timeout )
		{
			// Get recvfrom to unblock
			sock->Send(&sendParams, TRACE_FILE_AND_LINE_);
			JACKIE_Sleep(30);
		}
	}
	void ServerApplication::StopRecvPollingThread(void)
	{ isRecvPollingThreadActive = false; }
	void ServerApplication::StopSendPollingThread(void)
	{ isSendPollingThreadActive = false; }

	void ServerApplication::ProcessBufferedRecvParamQueue(JACKIE_INET_Address& systemAddress, const char *data, const int length, ServerApplication *serverApplication, JACKIE_INet_Socket* socket, BitStream &updateBitStream, TimeUS timeRead)
	{

	}

	//////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////// DECLARATIONS //////////////////////////////
	bool ServerApplication::RunSendCycleOnce(BitStream &updateBitStream)
	{
#if !defined(WINDOWS_STORE_RT) && !defined(__native_client__)
		if( JISList[0]->IsBerkleySocket() )
		{
			JISBerkley* berkelySock = ( (JISBerkley*) JISList[0] );

			if( berkelySock->GetSocketTransceiver() != 0 )
			{
				int len;
				JACKIE_INET_Address sender;
				char dataOut[MAXIMUM_MTU_SIZE];

				do
				{
					len = berkelySock->GetSocketTransceiver()->
						JackieINetRecvFrom(dataOut, &sender, true);

					if( len > 0 )
					{
						ProcessBufferedRecvParamQueue(sender, dataOut, len,
							this, JISList[0], updateBitStream, GetTimeUS());
					}
				} while( len > 0 );

			}
		}
#endif
		return 0;
	}
	void ServerApplication::RunRecvCycleOnce(void)
	{
		JISRecvParams* recvParams = 0;
		for( unsigned int index = 0; index < JISList.Size(); index++ )
		{
			recvParams = AllocJISRecvParams();
			if( recvParams != 0 )
			{
				recvParams->socket = JISList[index];
				( (JISBerkley*) JISList[index] )->RecvFrom(recvParams) >= 0 ? 				// we can recv 0 length data
					OnJISRecv(recvParams) : ReclaimJISRecvParams(recvParams);
			} else
			{
				notifyOutOfMemory(TRACE_FILE_AND_LINE_);
			}
		}
	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::RunRecvCycleLoop)
	{
		ServerApplication *serv = (ServerApplication*) arguments;
		if( !serv->isRecvPollingThreadActive ) serv->isRecvPollingThreadActive = true;

		unsigned int index;
		unsigned int count = serv->JISList.Size();
		JISRecvParams* recvParams = 0;

		JINFO << "Recv polling thread " << "is running in backend....";
		while( !serv->endSendRecvThreads && serv->isRecvPollingThreadActive ) { serv->RunRecvCycleOnce(); }
		JINFO << "Recv polling thread Stops....";

		if( serv->isRecvPollingThreadActive ) serv->isRecvPollingThreadActive = false;
		return 0;
	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::RunSendCycleLoop)
	{
		ServerApplication *serv = (ServerApplication*) arguments;
		if( !serv->isSendPollingThreadActive ) serv->isSendPollingThreadActive = true;

		BitStream sendBitStream(MAXIMUM_MTU_SIZE
#if LIBCAT_SECURITY==1
			+ cat::AuthenticatedEncryption::OVERHEAD_BYTES
#endif
			);

		JINFO << "Send polling thread is running in backend....";
		/// Normally, buffered sending packets go out every other 10 ms.
		/// or TriggerEvent() is called by recv thread
		while( !serv->endSendRecvThreads && serv->isSendPollingThreadActive )
		{ serv->RunSendCycleOnce(sendBitStream); serv->quitAndDataEvents.WaitEvent(10); }
		JINFO << "Send polling thread Stops....";

		if( serv->isSendPollingThreadActive ) serv->isSendPollingThreadActive = false;
		return 0;
	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::UDTConnect) { return 0; }
	//STATIC_FACTORY_DEFINITIONS(IServerApplication, ServerApplication);
	STATIC_FACTORY_DEFINITIONS(ServerApplication, ServerApplication);
	////////////////////////////////////////////////////////////////////////////////////

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
				JACKIE_Sleep(1);
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