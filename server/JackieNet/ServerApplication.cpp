#include "ServerApplication.h"
#include "NetTypes.h"
#include "WSAStartupSingleton.h"

namespace JACKIE_INET
{
	static const int mtuSizesCount = 3;
	static const int mtuSizes[mtuSizesCount] = { MAXIMUM_MTU_SIZE, 1200, 576 };

	ServerApplication::ServerApplication()
	{
#if LIBCAT_SECURITY == 1
		// Encryption and security
		CAT_AUDIT_PRINTF("AUDIT: Initializing RakPeer security flags: using_security = false, server_handshake = null, cookie_jar = null\n");
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
		maximumIncomingConnections = maximumNumberOfPeers = 0;
		bytesSentPerSecond = bytesReceivedPerSecond = 0;

		endThreads = true;
		isMainLoopThreadActive = false;
		recvHandler = 0;
		userUpdateThreadPtr = 0;
		userUpdateThreadData = 0;

		allowInternalRouting = false;
		incomingPasswordLength = 0;
		splitMessageProgressInterval = 0;

		allowConnectionResponseIPMigration = false;
		unreliableTimeout = 1000;
		maxOutgoingBPS = 0;

		myGuid = JACKIE_INet_GUID_Null;
		firstExternalID = JACKIE_INET_Address_Null;

		for( unsigned int i = 0; i < MAX_COUNT_LOCAL_IP_ADDR; i++ )
		{
			IPAddress[i] = JACKIE_INET_Address_Null;
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
		quitAndDataEvents.Init();

		packetAllocationPoolMutex.Lock();
		packetAllocationPool.SetPoolObjectsInitialSize(256);
		remoteSystemIndexPool.SetPoolObjectsInitialSize(256);
		packetAllocationPoolMutex.Unlock();


		GenerateGUID();
		ResetSendReceipt();
	}

	ServerApplication::~ServerApplication() { }

	JACKIE_INET::StartupResult ServerApplication::Start(UInt32 maxConnections,
		JACKIE_LOCAL_SOCKET *socketDescriptors,
		UInt32 socketDescriptorCount,
		Int32 threadPriority /*= -99999*/)
	{
		if( IsActive() ) return StartupResult::ALREADY_STARTED;

		// If getting the guid failed in the constructor, try again
		if( myGuid.g == 0 )
		{
			GenerateGUID();
			if( myGuid.g == 0 ) return StartupResult::COULD_NOT_GENERATE_GUID;
		}

		if( threadPriority == -99999 )
		{
#if  defined(_WIN32)
			threadPriority = 0;
#else
			threadPriority = 1000;
#endif
		}

		return ALREADY_STARTED;
	}


	void ServerApplication::OnJISRecv(JISRecvParams *recvStruct)
	{
	}

	void ServerApplication::DeallocJISRecvParams(JISRecvParams *s, const char *file, UInt32 line)
	{
	}

	JISRecvParams * ServerApplication::AllocJISRecvParams(const char *file, UInt32 line)
	{
		return NULL;
	}

	void ServerApplication::ResetSendReceipt(void)
	{
		sendReceiptSerialMutex.Lock();
		sendReceiptSerial = 1;
		sendReceiptSerialMutex.Unlock();
	}

}