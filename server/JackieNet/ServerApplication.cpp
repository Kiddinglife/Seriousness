#include "ServerApplication.h"
#include "WSAStartupSingleton.h"
#include "EasyLog.h"
#include "MessageID.h"
#include "JackieINetVersion.h"
#include "JackieSlidingWindows.h"
#include "IPlugin.h"

#if !defined ( __APPLE__ ) && !defined ( __APPLE_CC__ )
#include <stdlib.h> // malloc
#endif

#ifdef CAT_AUDIT
#define CAT_AUDIT_PRINTF(...) printf(__VA_ARGS__)
#else
#define CAT_AUDIT_PRINTF(...)
#endif

#define UNCONNETED_RECVPARAMS_HANDLER0 \
	if (recvParams->bytesRead >= sizeof(MessageID) + \
	sizeof(OFFLINE_MESSAGE_DATA_ID) + JackieGUID::size())\
																																																																{*isUnconnected = memcmp(recvParams->data + sizeof(MessageID),\
	OFFLINE_MESSAGE_DATA_ID, sizeof(OFFLINE_MESSAGE_DATA_ID)) == 0;}

#define UNCONNETED_RECVPARAMS_HANDLER1 \
	if (recvParams->bytesRead >=sizeof(MessageID) + sizeof(Time) + sizeof\
	(OFFLINE_MESSAGE_DATA_ID))\
																																																																{*isUnconnected =memcmp(recvParams->data + sizeof(MessageID) + \
	sizeof(Time), OFFLINE_MESSAGE_DATA_ID,sizeof(OFFLINE_MESSAGE_DATA_ID)) == 0;}

#define UNCONNECTED_RECVPARAMS_HANDLER2 \
   if (*isUnconnected) {\
   for (index = 0; index < pluginListNTS.Size(); index++)\
   pluginListNTS[index]->OnDirectSocketReceive(recvParams);}

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
	static void test_sendto(ServerApplication& sp)
	{
		JackieBits writer;
		writer.Write(ID_INCOMPATIBLE_PROTOCOL_VERSION);
		writer.Write((MessageID)JACKIE_INET_PROTOCOL_VERSION);
		writer.Write(OFFLINE_MESSAGE_DATA_ID,
			sizeof(OFFLINE_MESSAGE_DATA_ID));
		writer.WriteMini(sp.GetGuidFromSystemAddress(JACKIE_NULL_ADDRESS));

		JISSendParams data2send;
		data2send.data = writer.DataInt8();
		data2send.length = writer.GetWrittenBytesCount();
		data2send.receiverINetAddress = sp.bindedSockets[0]->GetBoundAddress();

		sp.bindedSockets[0]->Send(&data2send, TRACE_FILE_AND_LINE_);
		JDEBUG << "actually has sent " << data2send.bytesWritten << " bytes ";
		Sleep(1000);
	}

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
			RNS2_NativeClient * nativeClientSocket = (RNS2_NativeClient*)r2;
			ncbp.eventHandler = this;
			ncbp.forceHostAddress = (char*)bindLocalSockets[index].hostAddress;
			ncbp.is_ipv6 = bindLocalSockets[index].socketFamily == AF_INET6;
			ncbp.nativeClientInstance = bindLocalSockets[index].chromeInstance;
			ncbp.port = bindLocalSockets[index].port;
			nativeClientSocket->Bind(&ncbp, _FILE_AND_LINE_);
#elif defined(WINDOWS_STORE_RT)
			RNS2BindResult br;
			((RNS2_WindowsStore8*)r2)->SetRecvEventHandler(this);
			br = ((RNS2_WindowsStore8*)r2)->Bind(ref new Platform::String());
			if (br != BR_SUCCESS)
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
				berkleyBindParams.isNonBlocking = bindLocalSockets[index].blockingSocket;
#else
				///  single thread app will always use non-blobking socket 
				berkleyBindParams.isNonBlocking = true;
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
			< JISRecvParams* >> (bindedSockets.Size(), TRACE_FILE_AND_LINE_);
		allocRecvParamQ = JACKIE_INET::OP_NEW_ARRAY < ArraryQueue <
			JISRecvParams* >> (bindedSockets.Size(), TRACE_FILE_AND_LINE_);
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
			pluginListTS[index]->OnStartup();

		}

		for (index = 0; index < pluginListNTS.Size(); index++)
		{
			pluginListNTS[index]->OnStartup();
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

	inline void ServerApplication::ReclaimOneJISRecvParams(JISRecvParams *s, UInt32 index)
	{
		//JDEBUG << "Network Thread Reclaims One JISRecvParams";
		DCHECK_EQ(deAllocRecvParamQ[index].PushTail(s), true);
	}
	void ServerApplication::ReclaimAllJISRecvParams(UInt32 Index)
	{
		//JDEBUG << "Recv thread " << Index << " Reclaim All JISRecvParams";

		JISRecvParams* recvParams = 0;
		for (UInt32 index = 0; index < deAllocRecvParamQ[Index].Size(); index++)
		{
			DCHECK_EQ(deAllocRecvParamQ[Index].PopHead(recvParams), true);
			JISRecvParamsPool[Index].Reclaim(recvParams);
		}
	}
	inline JISRecvParams * ServerApplication::AllocJISRecvParams(UInt32 Index)
	{
		//JDEBUG << "Recv Thread" << Index << " Alloc An JISRecvParams";
		JISRecvParams* ptr = 0;
		do { ptr = JISRecvParamsPool[Index].Allocate(); } while (ptr == 0);
		ptr->localBoundSocket = bindedSockets[Index];
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
		//JDEBUG << "User Thread Reclaims All Commands";

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
		//JDEBUG << "User Thread Alloc An Command";
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
		//JDEBUG << "Network Thread Alloc One Packet";
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

	/// default 	p->freeInternalData = false;
	Packet* ServerApplication::AllocPacket(UInt32 dataSize, unsigned char *data)
	{
		//JDEBUG << "Network Thread Alloc One Packet";
		Packet *p = 0;
		do { p = packetPool.Allocate(); } while (p == 0);

		//p = new ( (void*) p ) Packet; no custom ctor so no need to call default ctor
		p->data = (unsigned char*)data;
		p->length = dataSize;
		p->bitSize = BYTES_TO_BITS(dataSize);
		p->freeInternalData = false;
		p->guid = JACKIE_NULL_GUID;
		p->wasGeneratedLocally = false;

		return p;
	}
	void ServerApplication::ReclaimAllPackets()
	{
		//JDEBUG << "Network Thread Reclaims All Packets";

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
				if (sock->GetBindingParams()->isNonBlocking == USE_BLOBKING_SOCKET)
				{
					/// try to send 0 data to let recv thread keep running
					/// to detect the isRecvPollingThreadActive === false so that stop the thread
					char zero[] = "This is used to Stop Recv Thread";
					JISSendParams sendParams =
					{ zero, sizeof(zero), 0, sock->GetBoundAddress(), 0 };
					SEND_10040_ERR(sock, sendParams);
					TimeMS timeout = Get32BitsTimeMS() + 1000;
					while (isRecvPollingThreadActive.GetValue() > 0 && Get32BitsTimeMS() < timeout)
					{
						SEND_10040_ERR(sock, sendParams);
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
		//JDEBUG << "Process One RecvParam";

#if LIBCAT_SECURITY==1
#ifdef CAT_AUDIT
		printf("AUDIT: RECV ");
		for (int ii = 0; ii < length; ++ii)
		{
			printf("%02x", (cat::u8)data[ii]);
		}
		printf("\n");
#endif
#endif // LIBCAT_SECURITY

		// 1.process baned client's recv
		/*char str1[64];
		systemAddress.ToString(false, str1);
		if (rakPeer->IsBanned(str1))
		{
		for (i = 0; i < rakPeer->pluginListNTS.Size(); i++)
		rakPeer->pluginListNTS[i]->OnDirectSocketReceive(data, length * 8, systemAddress);

		RakNet::BitStream bs;
		bs.WriteFrom((MessageID)ID_CONNECTION_BANNED);
		bs.WriteAlignedBytesFrom((const unsigned char*)OFFLINE_MESSAGE_DATA_ID, sizeof(OFFLINE_MESSAGE_DATA_ID));
		bs.WriteFrom(rakPeer->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS));


		RNS2_SendParameters bsp;
		bsp.data = (char*)bs.GetData();
		bsp.length = bs.GetWrittenBytesCount();
		bsp.systemAddress = systemAddress;
		for (i = 0; i < rakPeer->pluginListNTS.Size(); i++)
		rakPeer->pluginListNTS[i]->OnDirectSocketSend((char*)bs.GetData(), bs.GetNumberOfBitsUsed(), systemAddress);
		rakNetSocket->Send(&bsp, _FILE_AND_LINE_);

		return true;
		}*/

		DCHECK(recvParams->senderINetAddress.GetPortHostOrder() != 0);

		// 2. Try to process this recv params as unconnected
		bool isUnconnectedRecvPrrams;
		ProcessOneUnconnectedRecvParams(recvParams, &isUnconnectedRecvPrrams);

		/// no need to use return vlue
		//bool notSend2ReliabilityLayer = ProcessOneUnconnectedRecvParams(recvParams, &isUnconnectedRecvPrrams);
		//if (!notSend2ReliabilityLayer)

		if (!isUnconnectedRecvPrrams) //notSend2ReliabilityLayer
		{
			/// See if this datagram came from a connected system
			RemoteEndPoint* remoteEndPoint =
				GetRemoteEndPoint(recvParams->senderINetAddress, true, true);
			if (remoteEndPoint != 0) // if this datagram comes from connected system
			{
				//if (!isUnconnectedRecvPrrams)
				//{
				remoteEndPoint->reliabilityLayer.ProcessOneConnectedRecvParams(this, recvParams, remoteEndPoint->MTUSize);
				//}
			}
			else
			{
				char str[256];
				recvParams->senderINetAddress.ToString(true, str);
				JWARNING << "Network Thread Says Packet from unconnected sender " << str;
			}
		}
	}

	bool ServerApplication::ProcessOneUnconnectedRecvParams(
		JISRecvParams* recvParams, bool* isUnconnected)
	{
		//JDEBUG << "Network thread Process One Unconnected Recv Params";

		RemoteEndPoint* remoteEndPoint;
		Packet* packet;
		unsigned int index;

		// The reason for all this is that the reliability layer has no way to tell between offline
		// messages that arrived late for a player that is now connected,
		// and a regular encoding. So I insert OFFLINE_MESSAGE_DATA_ID into the stream,
		// the encoding of which is essentially impossible to hit by chance 
		if (recvParams->bytesRead <= 2)
		{
			*isUnconnected = true;
		}
		else
		{
			switch ((MessageID)recvParams->data[0])
			{
			case ID_UNCONNECTED_PING:
				UNCONNETED_RECVPARAMS_HANDLER1;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_UNCONNECTED_PING_OPEN_CONNECTIONS:
				UNCONNETED_RECVPARAMS_HANDLER1;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_UNCONNECTED_PONG:
				if (recvParams->bytesRead >
					sizeof(MessageID) + sizeof(TimeMS) + JackieGUID::size() +
					sizeof(OFFLINE_MESSAGE_DATA_ID))
				{
					*isUnconnected = memcmp(recvParams->data + sizeof(MessageID) +
						sizeof(Time) + JackieGUID::size(), OFFLINE_MESSAGE_DATA_ID, sizeof
						(OFFLINE_MESSAGE_DATA_ID)) == 0;
				}
				else
				{
					*isUnconnected = memcmp(recvParams->data + sizeof(MessageID) +
						sizeof(TimeMS) + JackieGUID::size(), OFFLINE_MESSAGE_DATA_ID, sizeof
						(OFFLINE_MESSAGE_DATA_ID)) == 0;
				}
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_OUT_OF_BAND_INTERNAL:
				if (recvParams->bytesRead >= sizeof(MessageID) + JackieGUID::size() +
					sizeof(OFFLINE_MESSAGE_DATA_ID))
				{
					*isUnconnected = memcmp(recvParams->data + sizeof(MessageID) +
						JackieGUID::size(), OFFLINE_MESSAGE_DATA_ID,
						sizeof(OFFLINE_MESSAGE_DATA_ID)) == 0;
				}
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				/// msg layout: MessageID MessageID OFFLINE_MESSAGE_DATA_ID JackieGUID
				if (recvParams->bytesRead > sizeof(MessageID) * 2 +
					sizeof(OFFLINE_MESSAGE_DATA_ID) &&
					recvParams->bytesRead <= sizeof(MessageID) * 2 +
					sizeof(OFFLINE_MESSAGE_DATA_ID) + JackieGUID::size())
				{
					*isUnconnected = memcmp(recvParams->data + sizeof(MessageID) * 2,
						OFFLINE_MESSAGE_DATA_ID, sizeof(OFFLINE_MESSAGE_DATA_ID)) == 0;
				}
				UNCONNECTED_RECVPARAMS_HANDLER2;
				if (*isUnconnected)
				{
					JackieBits reader((UInt8*)recvParams->data, recvParams->bytesRead);
					MessageID msgid;
					reader.Read(msgid);
					JDEBUG << "client receives msg with " << "msgid " << (int)msgid;
					if ((MessageID)recvParams->data[0] == ID_INCOMPATIBLE_PROTOCOL_VERSION)
					{
						MessageID update_protocol_version;
						reader.Read(update_protocol_version);
						JDEBUG << "update_protocol_version " << (int)update_protocol_version;
					}
					reader.ReadSkipBytes(sizeof(OFFLINE_MESSAGE_DATA_ID));

					JackieGUID guid;
					reader.ReadMini(guid);
					JDEBUG << "guid " << guid.g;

					ConnectionRequest *rcs;
					bool connectionAttemptCancelled = false;
					unsigned int index;

					connReqQLock.Lock();
					for (index = 0; index < connReqQ.Size(); index++)
					{
						rcs = connReqQ[index];
						if (rcs->actionToTake == ConnectionRequest::CONNECT &&
							rcs->receiverAddr == recvParams->senderINetAddress)
						{
							connectionAttemptCancelled = true;
							connReqQ.RemoveAtIndex(index);

#if LIBCAT_SECURITY==1
							CAT_AUDIT_PRINTF("AUDIT: Connection attempt canceled so deleting rcs->client_handshake object %x\n", rcs->client_handshake);
							JACKIE_INET::OP_DELETE(rcs->client_handshake, _FILE_AND_LINE_);
#endif // LIBCAT_SECURITY

							JACKIE_INET::OP_DELETE(rcs, TRACE_FILE_AND_LINE_);
							break;
						}
					}
					connReqQLock.Unlock();

					if (connectionAttemptCancelled)
					{
						/// Tell user connection attempt failed
						Packet* packet = AllocPacket(sizeof(unsigned char),
							(unsigned char*)recvParams->data);
						packet->systemAddress = recvParams->senderINetAddress;
						packet->guid = guid;
						DCHECK_EQ(allocPacketQ.PushTail(packet), true);
					}
					return true;
				}
				break;
			case ID_OPEN_CONNECTION_REPLY_1:
				if (recvParams->bytesRead >= sizeof(MessageID) * 2 +
					sizeof(OFFLINE_MESSAGE_DATA_ID) + JackieGUID::size() + sizeof(UInt16))
				{
					*isUnconnected = memcmp(recvParams->data + sizeof(MessageID),
						OFFLINE_MESSAGE_DATA_ID, sizeof(OFFLINE_MESSAGE_DATA_ID)) == 0;
				}
				UNCONNECTED_RECVPARAMS_HANDLER2;
				if (*isUnconnected)
				{
					JackieBits bsIn((UInt8*)recvParams->data, recvParams->bytesRead);
					bsIn.ReadSkipBytes(sizeof(MessageID));
					bsIn.ReadSkipBytes(sizeof(OFFLINE_MESSAGE_DATA_ID));
					JackieGUID serverGuid;
					bsIn.ReadMini(serverGuid);
					bool serverHasSecurity;
					bsIn.ReadMini(serverHasSecurity);
					UInt32 cookie;
					// Even if the server has security,
					// it may not be required of us if we are in the security exception list
					if (serverHasSecurity)
					{
						bsIn.Read(cookie);
					}

					JackieBits bsOut;
					bsOut.Write((MessageID)ID_OPEN_CONNECTION_REQUEST_2);
					bsOut.Write((const unsigned char*)OFFLINE_MESSAGE_DATA_ID, sizeof(OFFLINE_MESSAGE_DATA_ID));
					if (serverHasSecurity)
						bsOut.Write(cookie);

					ConnectionRequest *rcs;
					connReqQLock.Lock();
					for (unsigned int index = 0; index < connReqQ.Size(); index++)
					{
						rcs = connReqQ[index];
						if (rcs->actionToTake == ConnectionRequest::CONNECT &&
							rcs->receiverAddr == recvParams->senderINetAddress)
						{
							/// we can  unlock now 
							connReqQLock.Unlock();

							if (serverHasSecurity)
							{
#if LIBCAT_SECURITY==1
								unsigned char public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
								bsIn.ReadAlignedBytes(public_key, sizeof(public_key));

								if (rcs->publicKeyMode==PKM_ACCEPT_ANY_PUBLIC_KEY)
								{
									memcpy(rcs->remote_public_key, public_key, cat::EasyHandshake::PUBLIC_KEY_BYTES);
									if (!rcs->client_handshake->Initialize(public_key) ||
										!rcs->client_handshake->GenerateChallenge(rcs->handshakeChallenge))
									{
										CAT_AUDIT_PRINTF("AUDIT: Server passed a bad public key with PKM_ACCEPT_ANY_PUBLIC_KEY");
										return true;
									}
								}

								if (cat::SecureEqual(public_key,
									rcs->remote_public_key,
									cat::EasyHandshake::PUBLIC_KEY_BYTES) == false)
								{
									rakPeer->requestedConnectionQueueMutex.Unlock();
									CAT_AUDIT_PRINTF("AUDIT: Expected public key does not match what was sent by server -- Reporting back ID_PUBLIC_KEY_MISMATCH to user\n");

									packet = rakPeer->AllocPacket(sizeof(char), _FILE_AND_LINE_);
									packet->data[0] = ID_PUBLIC_KEY_MISMATCH; // Attempted a connection and couldn't
									packet->bitSize = (sizeof(char) * 8);
									packet->systemAddress = rcs->systemAddress;
									packet->guid = serverGuid;
									rakPeer->AddPacketToProducer(packet);
									return true;
								}

								if (rcs->client_handshake == 0)
								{
									// Message does not contain a challenge
									// We might still pass if we are in the security exception list
									bsOut.WriteFrom((unsigned char)0);
								}
								else
								{
									// Message contains a challenge
									bsOut.WriteFrom((unsigned char)1);
									// challenge
									CAT_AUDIT_PRINTF("AUDIT: Sending challenge\n");
									bsOut.WriteAlignedBytesFrom((const unsigned char*)rcs->handshakeChallenge, cat::EasyHandshake::CHALLENGE_BYTES);
								}
#else // LIBCAT_SECURITY
								// Message does not contain a challenge
								bsOut.WriteMini(false);
#endif // LIBCAT_SECURITY
							}
							else
							{
								// Server does not need security
#if LIBCAT_SECURITY==1
								if (rcs->client_handshake != 0)
								{
									rakPeer->requestedConnectionQueueMutex.Unlock();
									CAT_AUDIT_PRINTF("AUDIT: Security disabled by server but we expected security (indicated by client_handshake not null) so failing!\n");

									packet = rakPeer->AllocPacket(sizeof(char), _FILE_AND_LINE_);
									packet->data[0] = ID_OUR_SYSTEM_REQUIRES_SECURITY; // Attempted a connection and couldn't
									packet->bitSize = (sizeof(char) * 8);
									packet->systemAddress = rcs->systemAddress;
									packet->guid = serverGuid;
									rakPeer->AddPacketToProducer(packet);
									return true;
								}
#endif // LIBCAT_SECURITY
							}

							UInt16 mtu;
							bsIn.ReadMini(mtu);

							// bound address
							bsOut.Write(rcs->receiverAddr);
							// echo MTU
							bsOut.Write(mtu);
							// echo Our guid
							bsOut.Write(GetGuidFromSystemAddress(JACKIE_NULL_ADDRESS));

							JISSendParams bsp;
							bsp.data = bsOut.DataInt8();
							bsp.length = bsOut.GetWrittenBytesCount();
							bsp.receiverINetAddress = recvParams->senderINetAddress;
							recvParams->localBoundSocket->Send(&bsp, TRACE_FILE_AND_LINE_);

							for (index = 0; index < pluginListNTS.Size(); index++)
								pluginListNTS[index]->OnDirectSocketSend(&bsp);

							return true;
						}
					}
					connReqQLock.Unlock();
				}
				break;
			case ID_OPEN_CONNECTION_REPLY_2:
				UNCONNETED_RECVPARAMS_HANDLER0;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_OPEN_CONNECTION_REQUEST_1:
				UNCONNETED_RECVPARAMS_HANDLER0;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				if (*isUnconnected)
				{
					unsigned char remote_system_protcol = recvParams->data[sizeof(MessageID) + sizeof(OFFLINE_MESSAGE_DATA_ID)];

					JackieBits writer;
					// see if the protocol is up-to-date
					if (remote_system_protcol != (MessageID)JACKIE_INET_PROTOCOL_VERSION)
					{
						//test_sendto(*this);
						writer.Write(ID_INCOMPATIBLE_PROTOCOL_VERSION);
						writer.Write((MessageID)JACKIE_INET_PROTOCOL_VERSION);
						writer.Write(OFFLINE_MESSAGE_DATA_ID,
							sizeof(OFFLINE_MESSAGE_DATA_ID));
						writer.WriteMini(GetGuidFromSystemAddress(JACKIE_NULL_ADDRESS));

						JISSendParams data2send;
						data2send.data = writer.DataInt8();
						data2send.length = writer.GetWrittenBytesCount();
						data2send.receiverINetAddress = recvParams->senderINetAddress;

						recvParams->localBoundSocket->Send(&data2send, TRACE_FILE_AND_LINE_);

						for (index = 0; index < pluginListNTS.Size(); index++)
							pluginListNTS[index]->OnDirectSocketSend(&data2send);
					}
					else
					{
#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
						if (recvParams->localBoundSocket->IsBerkleySocket())
							((JISBerkley*)recvParams->localBoundSocket)->SetDoNotFragment(1);
#endif
						writer.Write(ID_OPEN_CONNECTION_REPLY_1);
						writer.Write((const unsigned char*)OFFLINE_MESSAGE_DATA_ID,
							sizeof(OFFLINE_MESSAGE_DATA_ID));
						writer.WriteMini(GetGuidFromSystemAddress(JACKIE_NULL_ADDRESS));

#if LIBCAT_SECURITY==1
						if (_using_security)
						{
							writer.WriteMini(true);  // HasCookie on
							// Write cookie
							UInt32 cookie = _cookie_jar->Generate(&systemAddress.address, sizeof(systemAddress.address));
							CAT_AUDIT_PRINTF("AUDIT: Writing cookie %i to %i:%i\n", cookie, systemAddress);
							writer.Write(cookie);
							// Write my public key
							writer.Write((const unsigned char *)my_public_key, sizeof(my_public_key));
						}
						else
#endif // LIBCAT_SECURITY
							writer.WriteMini(false);  // HasCookie off

						// MTU. Lower MTU if it exceeds our own limit.
						// because the size clients send us is hardcoded as 
						// bitStream.PadZero2LengthOf(mtuSizes[MTUSizeIndex] -
						// UDP_HEADER_SIZE);  see connect() for details
						UInt16 client_max_mtu = (recvParams->bytesRead + UDP_HEADER_SIZE >= MAXIMUM_MTU_SIZE) ? MAXIMUM_MTU_SIZE : recvParams->bytesRead + UDP_HEADER_SIZE;
						writer.WriteMini(client_max_mtu);
						// Pad response with zeros to MTU size
						// so the connection's MTU will be tested in both directions
						writer.PadZero2LengthOf(client_max_mtu - writer.GetWrittenBytesCount());

						JISSendParams bsp;
						bsp.data = writer.DataInt8();
						bsp.length = writer.GetWrittenBytesCount();
						bsp.receiverINetAddress = recvParams->senderINetAddress;

						recvParams->localBoundSocket->Send(&bsp, TRACE_FILE_AND_LINE_);

						for (index = 0; index < pluginListNTS.Size(); index++)
							pluginListNTS[index]->OnDirectSocketSend(&bsp);

#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
						if (recvParams->localBoundSocket->IsBerkleySocket())
							((JISBerkley*)recvParams->localBoundSocket)->SetDoNotFragment(0);
#endif
					}

					return true;
				}
				break;
			case ID_OPEN_CONNECTION_REQUEST_2:
				UNCONNETED_RECVPARAMS_HANDLER0;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				UNCONNETED_RECVPARAMS_HANDLER0;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				UNCONNETED_RECVPARAMS_HANDLER0;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_CONNECTION_BANNED:
				UNCONNETED_RECVPARAMS_HANDLER0;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_ALREADY_CONNECTED:
				UNCONNETED_RECVPARAMS_HANDLER0;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			case ID_IP_RECENTLY_CONNECTED:
				UNCONNETED_RECVPARAMS_HANDLER0;
				UNCONNECTED_RECVPARAMS_HANDLER2;
				break;
			default:
				*isUnconnected = false;
				break;
			}
		}
		return false;
	}


	void ServerApplication::ProcessConnectionRequestCancelQ(void)
	{
		//JDEBUG << "Network Thread Process ConnectionRequest CancelQ";

		if (connReqCancelQ.IsEmpty())
		{
			//JDEBUG << "ConnectionRequestCancelQ is EMPTY";
			return;
		}

		//JDEBUG << "Connection Request CancelQ is NOT EMPTY";

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

	/// @Done
	void ServerApplication::ProcessConnectionRequestQ(TimeUS& timeUS, TimeMS& timeMS)
	{
		//DEBUG << "Network Thread Process ConnectionRequestQ";

		if (connReqQ.IsEmpty())
		{
			//JDEBUG << "ConnectionRequestQ is EMPTY";
			return;
		}

		//JDEBUG << "Connection Request Q not EMPTY";

		if (timeUS == 0)
		{
			timeUS = Get64BitsTimeUS();
			timeMS = (TimeMS)(timeUS / (TimeUS)1000);
		}

		ConnectionRequest *connReq = 0;

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
					MessageID msgid = ID_CONNECTION_ATTEMPT_FAILED;
					Packet* packet = AllocPacket(sizeof(MessageID), &msgid);
					packet->systemAddress = connReq->receiverAddr;
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
					bitStream.Write(ID_OPEN_CONNECTION_REQUEST_1);
					bitStream.Write(OFFLINE_MESSAGE_DATA_ID,
						sizeof(OFFLINE_MESSAGE_DATA_ID));
					bitStream.Write((MessageID)JACKIE_INET_PROTOCOL_VERSION);
					bitStream.PadZero2LengthOf(mtuSizes[MTUSizeIndex] -
						UDP_HEADER_SIZE);
					//bitStream.PadZero2LengthOf(3000 -
					//	UDP_HEADER_SIZE); //will trigger 10040 recvfrom 

					connReq->receiverAddr.FixForIPVersion(connReq->socket->GetBoundAddress());

					JISSendParams jsp;
					jsp.data = bitStream.DataInt8();
					jsp.length = bitStream.GetWrittenBytesCount();
					jsp.receiverINetAddress = connReq->receiverAddr;

#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
					if (connReq->socket->IsBerkleySocket())
						((JISBerkley*)connReq->socket)->SetDoNotFragment(1);
#endif
					Time sendToStart = GetTimeMS();
					if (connReq->socket->Send(&jsp, TRACE_FILE_AND_LINE_) < 0 &&
						jsp.bytesWritten == 10040)
					{
						// MessageId: WSAEMSGSIZE
						// MessageText:
						// A message sent on a datagram socket was larger than the internal message 
						// buffer or some other network limit, or the buffer used to receive a datagram
						// into was smaller than the datagram itself.
						JWARNING << "Send return 10040 error!!!";
						connReq->requestsMade = (unsigned char)((MTUSizeIndex + 1) * (connReq->connAttemptTimes / mtuSizesCount));
						connReq->nextRequestTime = timeMS;
					}
					else
					{
						Time sendToEnd = GetTimeMS();
						if (sendToEnd - sendToStart > 100)
						{
							JINFO << "> 100";
							/// Drop to lowest MTU
							int lowestMtuIndex = connReq->connAttemptTimes / mtuSizesCount
								* (mtuSizesCount - 1);
							if (lowestMtuIndex > connReq->requestsMade)
							{
								connReq->requestsMade = (unsigned char)lowestMtuIndex;
								connReq->nextRequestTime = timeMS;
							}
							else
							{
								connReq->requestsMade = (unsigned char)(connReq->connAttemptTimes + 1);
							}
						}
						//}

						/// set back  the SetDoNotFragment to allowed fragment
#if !defined(__native_client__) && !defined(WINDOWS_STORE_RT)
						if (connReq->socket->IsBerkleySocket())
							((JISBerkley*)connReq->socket)->SetDoNotFragment(0);
#endif
					}
				}
			}
			connReqQLock.Unlock();
		}
	}

	void ServerApplication::ProcessAllocCommandQ(TimeUS& timeUS, TimeMS& timeMS)
	{
		//JDEBUG << "Network Thread Process Alloc CommandQ";

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
		//static bool oneshot = false;
		//if (!oneshot)
		//{
		//	JackieBits bitStream;
		//	bitStream.Write(ID_OPEN_CONNECTION_REQUEST_1);
		//	bitStream.Write(OFFLINE_MESSAGE_DATA_ID,
		//		sizeof(OFFLINE_MESSAGE_DATA_ID));
		//	//bitStream.Write((MessageID)JACKIE_INET_PROTOCOL_VERSION);
		//	bitStream.Write((MessageID)12);
		//	// definitely will not be fragmented because 
		//	// UDP_HEADER_SIZE = 28 > 1+16+1=18
		//	bitStream.PadZeroAfterAlignedWRPos(mtuSizes[0] -
		//		UDP_HEADER_SIZE);
		//	JISSendParams jsp;
		//	jsp.data = bitStream.DataInt8();
		//	jsp.length = bitStream.GetWrittenBytesCount();
		//	jsp.receiverINetAddress = bindedSockets[0]->GetBoundAddress();
		//	JISSendResult len = bindedSockets[0]->Send(&jsp, TRACE_FILE_AND_LINE_);
		//	JDEBUG << "actually has sent " << len << " bytes ";
		//	Sleep(1000);
		//	oneshot = true;
		//}

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
		//JDEBUG << "@TO-DO AdjustTimestamp()";

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

	const JackieGUID& ServerApplication::GetGuidFromSystemAddress(const JackieAddress
		input) const
	{
		if (input == JACKIE_NULL_ADDRESS) return myGuid;

		if (input.systemIndex != (SystemIndex)-1 &&
			input.systemIndex < maxConnections &&
			remoteSystemList[input.systemIndex].systemAddress == input)
			return remoteSystemList[input.systemIndex].guid;

		for (unsigned int i = 0; i < maxConnections; i++)
		{
			if (remoteSystemList[i].systemAddress == input)
			{
				// Set the systemIndex so future lookups will be fast
				remoteSystemList[i].guid.systemIndex = (SystemIndex)i;
				return remoteSystemList[i].guid;
			}
		}
		return JACKIE_NULL_GUID;
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

	// Attatches a Plugin interface to run code automatically 
	// on message receipt in the Receive call
	void ServerApplication::AttachOnePlugin(IPlugin *plugin)
	{
		bool isNotThreadsafe = plugin->UsesReliabilityLayer();
		if (isNotThreadsafe)
		{
			if (pluginListNTS.GetIndexOf(plugin) == MAX_UNSIGNED_LONG)
			{
				plugin->serverApplication = this;
				plugin->OnAttach();
				pluginListNTS.InsertAtLast(plugin);
			}
		}
		else
		{
			if (pluginListTS.GetIndexOf(plugin) == MAX_UNSIGNED_LONG)
			{
				plugin->serverApplication = this;
				plugin->OnAttach();
				pluginListTS.InsertAtLast(plugin);
			}
		}
	}

	void ServerApplication::PacketGoThroughPluginCBs(Packet*& incomePacket)
	{

		UInt32 i;
		for (i = 0; i < pluginListTS.Size(); i++)
		{
			switch ((MessageID)incomePacket->data[0])
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
		//JDEBUG << "User Thread Update Plugins";
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
		//	TIMED_FUNC();

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

		/// Pop out one Packet from queue
		if (allocPacketQ.Size() > 0)
		{
			Packet *incomePacket = 0;

			/// Get one income packet from bufferedPacketsQueue
			DCHECK_EQ(allocPacketQ.PopHead(incomePacket), true);
			DCHECK_NOTNULL(incomePacket->data);

			AdjustTimestamp(incomePacket);

			/// Some locally generated packets need to be processed by plugins,
			/// for example ID_FCM2_NEW_HOST. The plugin itself should intercept
			/// these messages generated remotely
			PacketGoThroughPluginCBs(incomePacket);
			PacketGoThroughPlugins(incomePacket);

			return incomePacket;
		}

		return 0;
	}

	void ServerApplication::RunNetworkUpdateCycleOnce(void)
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
	}

	void ServerApplication::RunRecvCycleOnce(UInt32 index)
	{
		//TIMED_FUNC();
		ReclaimAllJISRecvParams(index);
		JISRecvParams* recvParams = AllocJISRecvParams(index);
		int result = ((JISBerkley*)bindedSockets[index])->RecvFrom(recvParams);
		if (result > 0)
		{
			DCHECK_EQ(allocRecvParamQ[index].PushTail(recvParams), true);
			if (incomeDatagramEventHandler != 0)
			{
				if (!incomeDatagramEventHandler(recvParams))
					JWARNING << "incomeDatagramEventHandler(recvStruct) Failed.";
			}
#if USE_SINGLE_THREAD == 0
			if (allocRecvParamQ[index].Size() > 0) quitAndDataEvents.TriggerEvent();
#endif
		}
		else
		{
			//@ Remember myself
			// this hsould not happen because we have hard coded the max MTU is 1500
			// so client who uses this library will send not datagram bigger than that, 
			// so this error will not trigger. Hoever, for the clients 
			/// who uses other library sneds us a datagram that is bigger than our max MTU,
			/// 10040 error  will happend n such case.
			/// in this case, we have nothing to do with such case only way is to tell this guy in realistic world "man, please send smaller datagram to me"
			/// i put it here is just reminding myself and 
			if (recvParams->bytesRead == 10040)
			{
				JWARNING <<
					"recvfrom() return 10040 error, the remote send a bigger datagram than our  max mtu";
			}
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
#if USE_SINGLE_THREAD == 1
			JackieSleep(10);
#endif
			//JackieSleep(1000);
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

		JDEBUG << "Network thread is running in backend....";
		while (!serv->endThreads)
		{
			/// Normally, buffered sending packets go out every other 10 ms.
			/// or TriggerEvent() is called by recv thread
			serv->RunNetworkUpdateCycleOnce();
#if USE_SINGLE_THREAD == 0
			serv->quitAndDataEvents.WaitEvent(10);
#endif
			//JackieSleep(1000);
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
		JDEBUG << "User Thread start to Connect() to " << host << ":" << port;

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
		connReq->extraData = extraData;
		connReq->socketIndex = ConnectionSocketIndex;
		connReq->socket = bindedSockets[ConnectionSocketIndex];
		connReq->actionToTake = ConnectionRequest::CONNECT;
		connReq->connAttemptTimes = ConnectionAttemptTimes;
		connReq->connAttemptIntervalMS = ConnectionAttemptIntervalMS;
		connReq->timeout = timeout;
		connReq->pwdLen = pwdLen;
		if (pwdLen > 0 && pwd != 0)
		{
			memcpy(connReq->pwd, pwd, pwdLen);
		}

#if LIBCAT_SECURITY ==1
		CAT_AUDIT_PRINTF("AUDIT: In SendConnectionRequest()\n");
		if (!GenerateConnectionRequestChallenge(connReq, publicKey))
			return SECURITY_INITIALIZATION_FAILED;
#else
		(void)publicKey;
#endif

		connReqQLock.Lock();
		for (UInt32 Index = 0; Index < connReqQ.Size(); Index++)
		{
			if (connReqQ[Index]->receiverAddr == addr)
			{
				connReqQLock.Unlock();
				JACKIE_INET::OP_DELETE(connReq, TRACE_FILE_AND_LINE_);
				return CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS;
			}
		}
		DCHECK_EQ(connReqQ.PushTail(connReq), true);
		connReqQLock.Unlock();

		return CONNECTION_ATTEMPT_POSTED;
	}

}