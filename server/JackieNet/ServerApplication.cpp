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

#if USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
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
			ClearSocketQueryOutputs();
			ClearBufferedAllocatedRecvParamQueue();
			ClearBufferedDeAllocatedRecvParamQueue();

			firstExternalID = JACKIE_INET_Address_Null;
			updateCycleIsRunning = false;
			endSendRecvThreads = false;

#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
#if USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
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
#if USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
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
	void ServerApplication::OnJISRecv(JISRecvParams *recvStruct)
	{
		JINFO << "Recv " << recvStruct->bytesRead << " bytes of data [" << recvStruct->data << "]";

		if( incomeDatagramEventHandler != 0 )
		{
			if( !incomeDatagramEventHandler(recvStruct) )
			{
				JERROR << "Because incomeDatagramEventHandler(recvStruct) Failed, "
					<< "this received message is ignored.";
				return;
			}
		}

		QUEUE_PUSH_TAIL(bufferedAllocatedRecvParamQueue, recvStruct);

#if USE_SINGLE_THREAD_TO_SEND_AND_RECV == 0
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
		while( bufferedCommands.PopHead(bcs) )
		{
			if( bcs->data != 0 ) rakFree_Ex(bcs->data, TRACE_FILE_AND_LINE_);
			bufferedCommands.Deallocate(bcs);
		}
		bufferedCommands.Clear();
	}
	void ServerApplication::ClearSocketQueryOutputs(void)
	{
		socketQueryOutput.Clear();
	}
	void ServerApplication::ClearBufferedAllocatedRecvParamQueue(void)
	{
		bufferedAllocatedRecvParamQueue.Clear();
	}
	void ServerApplication::ClearBufferedDeAllocatedRecvParamQueue(void)
	{
		bufferedDeallocatedRecvParamQueue.Clear();
	}

	////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////// AllocPacket ///////////////////////////////////
	Packet* ServerApplication::AllocPacket(unsigned int dataSize, ThreadType threadType)
	{
		Packet *p = 0;
		p = ( threadType == RecvThreadType ) ?
			recvThreadPacketAllocationPool.Allocate() :
			sendThreadPacketAllocationPool.Allocate();

		assert(p != 0);
		//p = new ( (void*) p ) Packet; we do not need call default ctor

		p->data = (unsigned char*) rakMalloc_Ex(dataSize, TRACE_FILE_AND_LINE_);
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->deleteData = true;
		p->guid = JACKIE_INet_GUID_Null;
		p->wasGeneratedLocally = false;
		p->threadType = threadType;

		return p;
	}
	Packet* ServerApplication::AllocPacket(unsigned dataSize,
		unsigned char *data, ThreadType threadType)
	{
		Packet *p = 0;
		p = ( threadType == RecvThreadType ) ?
			recvThreadPacketAllocationPool.Allocate() :
			sendThreadPacketAllocationPool.Allocate();

		assert(p != 0);
		//p = new ( (void*) p ) Packet; no custom ctor so no need to call default ctor

		p->data = data;
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->deleteData = true;
		p->guid = JACKIE_INet_GUID_Null;
		p->wasGeneratedLocally = false;
		p->threadType = threadType;

		return p;
	}
	void ServerApplication::DeallocatePacket(Packet *packet)
	{
		CHECK_EQ(packet, 0);
		if( packet == 0 ) return;
		if( packet->deleteData )
		{
			rakFree_Ex(packet->data, TRACE_FILE_AND_LINE_);
			//packet->~Packet(); no custom dtor so no need to call default dtor
			( packet->threadType == RecvThreadType ) ?
				recvThreadPacketAllocationPool.Reclaim(packet) :
				sendThreadPacketAllocationPool.Reclaim(packet);
		} else
		{
			rakFree_Ex(packet, TRACE_FILE_AND_LINE_);
		}

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

	void ServerApplication::ProcessOneRecvParam(JISRecvParams* recvParams,
		BitStream &updateBitStream)
	{
		/// no need to check if recvParams == 0, because we never push 0 pointer
		bufferedAllocatedRecvParamQueue.PopHead(recvParams);
		CHECK_NOTNULL(recvParams);

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

		RemoteEndPoint* remoteEndPoint =
			GetRemoteEndPoint(recvParams->senderINetAddress, true, true);
		if( remoteEndPoint != 0 ) // if this datagram comes from connected system
		{
			if( !isOfflinerecvParams )
			{
				remoteEndPoint->reliabilityLayer.ProcessJISRecvParamsFromConnectedEndPoint(*this, remoteEndPoint->MTUSize, updateBitStream);
			}
		} else
		{
			JWARNING << "Packet from unconnected sender"
				<< recvParams->senderINetAddress.ToString();
		}
	}

	bool ServerApplication::ProcessOneOfflineRecvParam(JISRecvParams* recvParams,
		bool* isOfflinerecvParams)
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	/// @TO-DO
	void ServerApplication::AdjustTimestamp(Packet*& incomePacket) const
	{
		if( (unsigned char) incomePacket->data[0] == ID_TIMESTAMP )
		{
			if( incomePacket->length >= sizeof(unsigned char) + sizeof(Time) )
			{
				unsigned char* data = &incomePacket->data[sizeof(unsigned char)];
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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	RemoteEndPoint* ServerApplication::GetRemoteEndPoint(const JACKIE_INET_Address& senderAddress, bool neededBySendThread, bool onlyWantActiveEndPoint) const
	{
		if( senderAddress == JACKIE_INET_Address_Null ) return 0;

		if( neededBySendThread )
		{
			Int32 index = GetRemoteEndPointIndex(senderAddress);
			if( index != -1 )
			{
				if( !onlyWantActiveEndPoint || remoteSystemList[index].isActive )
				{
					CHECK_EQ(remoteSystemList[index].systemAddress, senderAddress);
					return &remoteSystemList[index];
				}
			}
		} else
		{
			/// Active EndPoints take priority.  But if matched end point is inactice, 
			/// return the first EndPoint match found
			Int32 inActiveEndPointIndex = -1;
			for( UInt32 index = 0; index < maxConnections; index++ )
			{
				if( remoteSystemList[index].systemAddress == senderAddress )
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
	Int32 ServerApplication::GetRemoteEndPointIndex(const JACKIE_INET_Address &sa) const
	{
		UInt32 hashindex = JACKIE_INET_Address::ToHashCode(sa) %
			( maxConnections * RemoteEndPointLookupHashMutiple );
		RemoteEndPointIndex* curr = remoteSystemLookup[hashindex];
		while( curr != 0 )
		{
			if( remoteSystemList[curr->index].systemAddress == sa )
				return curr->index;
			curr = curr->next;
		}
		return  -1;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		UInt32 i;
		PluginActionType pluginResult;

		for( i = 0; i < pluginListTS.Size(); i++ )
		{
			pluginResult = pluginListTS[i]->OnRecvPacket(incomePacket);
			if( pluginResult == PROCESSED_BY_ME_THEN_DEALLOC )
			{
				DeallocatePacket(incomePacket);
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
				DeallocatePacket(incomePacket);
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
		UInt32 i;
		for( i = 0; i < pluginListTS.Size(); i++ )
		{
			pluginListTS[i]->Update();
		}
		for( i = 0; i < pluginListNTS.Size(); i++ )
		{
			pluginListNTS[i]->Update();
		}
	}

	//////////////////////////////////////////////////////////////////////////


	///////////////////////////////// DECLARATIONS //////////////////////////////
	Packet* ServerApplication::FetchOnePacket(void)
	{
		if( !IsActive() ) return 0;

		/// UPDATE all plugins
		UpdatePlugins();

		Packet *incomePacket = 0;
		do
		{
			//////////////////////////////////////////////////////////////////////////
			if( bufferedPacketsQueue.isEmpty() ) return 0;
			/// Get one *incomePacket from queue
			bufferedPacketsQueue.PopHead(incomePacket);
			AdjustTimestamp(incomePacket);
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			/// Some locally generated packets need to be processed by plugins,
			/// for example ID_FCM2_NEW_HOST. The plugin itself should intercept
			/// these messages generated remotely
			PacketGoThroughPluginCBs(incomePacket);
			PacketGoThroughPlugins(incomePacket);
			//////////////////////////////////////////////////////////////////////////

		} while( incomePacket == 0 );

		CHECK_NE(incomePacket->data, 0);
		return incomePacket;
	}
	bool ServerApplication::RunSendCycleOnce(BitStream &updateBitStream)
	{
		unsigned int index;
		static JISRecvParams recv;

#if !defined(WINDOWS_STORE_RT) && !defined(__native_client__)

		for( index = 0; index < JISList.Size(); index++ )
		{
			if( JISList[index]->IsBerkleySocket() )
			{
				JISBerkley* berkelySock = ( (JISBerkley*) JISList[index] );
				if( berkelySock->GetSocketTransceiver() != 0 )
				{
					int len;
					char dataOut[MAXIMUM_MTU_SIZE];
					do
					{
						len = berkelySock->GetSocketTransceiver()->
							JackieINetRecvFrom(dataOut, &recv.senderINetAddress, true);

						if( len > 0 )
						{
							ProcessOneRecvParam(&recv, updateBitStream);
						}
					} while( len > 0 );

				}
			}
		}
#endif

		JISRecvParams* recvParams = 0;
		for( unsigned int index = 0; index < bufferedAllocatedRecvParamQueue.Size();
			index++ )
		{
			ProcessOneRecvParam(recvParams, updateBitStream);
			bufferedDeallocatedRecvParamQueue.PushTail(recvParams);
		}
		return 0;
	}
	void ServerApplication::RunRecvCycleOnce(void)
	{
		JISRecvParams* recvParams = 0;
		unsigned int index;
		for( index = 0; index < JISList.Size(); index++ )
		{
			recvParams = AllocJISRecvParams();
			if( recvParams != 0 )
			{
				recvParams->socket = JISList[index];
				( (JISBerkley*) JISList[index] )->RecvFrom(recvParams) > 0 ?
					OnJISRecv(recvParams) : ReclaimJISRecvParams(recvParams);
			} else
			{
				notifyOutOfMemory(TRACE_FILE_AND_LINE_);
			}
		}

		for( index = 0; index < bufferedDeallocatedRecvParamQueue.Size();
			index++ )
		{
			if( bufferedDeallocatedRecvParamQueue.PopHead(recvParams) )
				ReclaimJISRecvParams(recvParams);
		}

	}
	JACKIE_THREAD_DECLARATION(JACKIE_INET::RunRecvCycleLoop)
	{
		ServerApplication *serv = (ServerApplication*) arguments;
		if( !serv->isRecvPollingThreadActive ) serv->isRecvPollingThreadActive = true;

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