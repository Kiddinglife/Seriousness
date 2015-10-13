#include "JACKIE_INet_Socket.h"
#include "WSAStartupSingleton.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

namespace JACKIE_INET
{
	////////////// Globals : GetMyIP_Wins_Linux //////////////////////
#if !defined(WINDOWS_STORE_RT) && !defined(__native_client__)
#if NET_SUPPORT_IPV6 ==1
	/// reference to http://www.cnblogs.com/chinacloud/archive/2011/08/11/2135141.html

	void GetMyIP_Wins_Linux_IPV4And6(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR])
	{
		WSAStartupSingleton::AddRef();

		int idx = 0;

		char buf[80];
		JACKIE_ASSERT(gethostname(buf, 80) != -1);

		struct addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo)); // make sure the struct is empty
		hints.ai_socktype = SOCK_DGRAM; // UDP sockets
		hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

		struct addrinfo *servinfo = 0,
		struct addrinfo *aip = 0;  // will point to the results
		getaddrinfo(buf, "", &hints, &servinfo);

		for( idx = 0, aip = servinfo; aip != NULL && idx < MAX_COUNT_LOCAL_IP_ADDR;
			aip = aip->ai_next, idx++ )
		{
			if( aip->ai_family == AF_INET )
			{
				struct sockaddr_in *ipv4 = ( struct sockaddr_in * )aip->ai_addr;
				memcpy(&addresses[idx].address.addr4, ipv4, sizeof(sockaddr_in));
			} else
			{
				struct sockaddr_in6 *ipv6 = ( struct sockaddr_in6 * )aip->ai_addr;
				memcpy(&addresses[idx].address.addr4, ipv6, sizeof(sockaddr_in6));
			}

		}

		freeaddrinfo(servinfo); // free the linked-list

		while( idx < MAX_COUNT_LOCAL_IP_ADDR )
		{
			addresses[idx] = JACKIE_INET_Address_Null;
			idx++;
		}

		WSAStartupSingleton::Deref();
	}
#else
	void GetMyIP_Wins_Linux_IPV4(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR])
	{
		WSAStartupSingleton::AddRef();

		char buf[80];
		JACKIE_ASSERT(gethostname(buf, 80) != -1);

		struct hostent *phe = gethostbyname(buf);
		if( phe == 0 )
		{
			JACKIE_ASSERT(phe != 0);
			return;
		}

		int idx = 0;
		for( idx = 0; idx < MAX_COUNT_LOCAL_IP_ADDR; idx++ )
		{
			if( phe->h_addr_list[idx] == 0 ) break;
			memcpy(&addresses[idx].address.addr4.sin_addr, phe->h_addr_list[idx],
				sizeof(in_addr));
		}

		while( idx < MAX_COUNT_LOCAL_IP_ADDR )
		{
			addresses[idx] = JACKIE_INET_Address_Null;
			idx++;
		}
		WSAStartupSingleton::Deref();
	}
#endif
#endif
	////////////// Globals : GetMyIP_Wins_Linux //////////////////////


	inline void JACKIE_INet_Socket::GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR])
	{
#if defined(WINDOWS_STORE_RT)
		JISWINSTROE8::GetMyIP(addresses);
#elif defined(__native_client__)
		RNS2_NativeClient::GetMyIP(addresses);
#elif defined(_WIN32)
		JISWins::GetMyIP(addresses);
#else
		JISLinux::GetMyIP(addresses);
#endif
	}

#if defined (WINDOWS_STORE_RT) 	/// We are using WINDOWS_STORE_RT plateform
	//@TODO
#elif defined (__native_client__)  	/// We are using NaCI plateform
	//@TODO
# else 	/// We are using Wins or LINUX or Unix plateform

#if defined(__APPLE__)
	static void SocketReadCallback(CFSocketRef s, CFSocketCallBackType type, 
		CFDataRef address, const void *data, void *info)
		// This C routine is called by CFSocket when there's data waiting on our 
		// UDP socket.  It just redirects the call to Objective-C code.
	{
	}
#endif

	//////////////////// Send_Windows_Linux_360NoVDP ////////////////////////////
#if defined(_WIN32) || defined(__GNUC__)  || defined(__GCCXML__) || defined(__S3E__)
	JISSendResult Send_Windows_Linux_360NoVDP(
		JISSocket rns2Socket, JISSendParams *sendParameters,
		const char *file, unsigned int line)
	{
		int len = 0;
		int oldTTL = -1;
		int newTTL = -1;
		socklen_t opLen = sizeof(oldTTL);

		do
		{
			oldTTL = -1;
			if( sendParameters->ttl > 0 )
			{
				// Get the current TTL
				if( getsockopt__(rns2Socket,
					sendParameters->systemAddress.GetIPProtocol(),
					IP_TTL, (char *) & oldTTL, &opLen) != -1 )
				{
					newTTL = sendParameters->ttl;
					setsockopt__(rns2Socket,
						sendParameters->systemAddress.GetIPProtocol(),
						IP_TTL, (char *) & newTTL, sizeof(newTTL));
				}
			}


			if( sendParameters->systemAddress.address.addr4.sin_family == AF_INET )
			{
				/// only when sendParameters->length == 0, sendto() will return 0;
				/// otherwise it will return > 0, or error code of -1
				/// here we need avoid user wrong input by assert
				JACKIE_ASSERT(sendParameters->length > 0);
				len = sendto__(rns2Socket, sendParameters->data, sendParameters->length, 0, (const sockaddr*) & sendParameters->systemAddress.address.addr4, sizeof(sockaddr_in));
			} else
			{
#if NET_SUPPORT_IPV6 ==1
				len = sendto__(rns2Socket, sendParameters->data, sendParameters->length, 0, (const sockaddr*) & sendParameters->systemAddress.address.addr6, sizeof(sockaddr_in6));
#endif
			}

			if( len < 0 )
			{
				JACKIE_NET_DEBUG_PRINTF("sendto failed with errno %i for char %i and length %i.\n", len, sendParameters->data[0], sendParameters->length);
			}

			if( oldTTL != -1 )
			{
				setsockopt__(rns2Socket, sendParameters->systemAddress.GetIPProtocol(),
					IP_TTL, (char *) & oldTTL, sizeof(oldTTL));
			}
		} while( len == 0 );

		return len;
	}
#endif
	//////////////////// Send_Windows_Linux_360NoVDP ////////////////////////////


	////////////////////////////// JISBerkley implementations ////////////////////////////
	bool JISBerkley::IsPortInUse(unsigned short port, const char *hostAddress,
		unsigned short addressFamily, int type)
	{
		unsigned short remotePortJackieNetWasStartedOn_PS3_PS4_PSP2;
		JISBerkleyBindParams bbp =
		{
			port, //unsigned short port;
			(char*) hostAddress, //char *hostAddress;
			addressFamily, //unsigned short addressFamily; // AF_INET or AF_INET6
			type, //int type; // SOCK_DGRAM
			0, //int protocol; // 0
			false, //bool nonBlockingSocket;
			false, //int setBroadcast;
			false, //int setIPHdrIncl;
			false, //int doNotFragment;
			0, //int pollingThreadPriority;
			0,//JISEventHandler *eventHandler;
			0 //unsigned short remotePortJackieNetWasStartedOn_PS3_PS4_PSP2;
		};

		JACKIE_INET_Address boundAddress;
		JISBerkley *rns2 = (JISBerkley*) JISAllocator::AllocJIS();
		JISBindResult bindResult = rns2->Bind(&bbp, TRACE_FILE_AND_LINE_);
		JISAllocator::DeallocJIS(rns2);
		return bindResult == JISBindResult_FAILED_BIND_SOCKET;
	}

	JISBindResult JISBerkley::Bind(JISBerkleyBindParams *bindParameters,
		const char *file, UInt32 line)
	{
		WSAStartupSingleton::AddRef();
		JISBindResult bindResult = BindShared(bindParameters, file, line);
		WSAStartupSingleton::Deref();
		/// we do not test bindResult == JISBindResult_FAILED_SEND_TEST here 
		while( bindResult == JISBindResult_FAILED_BIND_SOCKET )
		{
			// Sometimes windows will fail if the socket is recreated too quickly
			JACKIE_Sleep(100);
			bindResult = BindShared(bindParameters, file, line);
		}
		return bindResult;
	}
	JISBindResult JISBerkley::BindShared(JISBerkleyBindParams *bindParameters,
		const char *file, unsigned int line)
	{
		JISBindResult br;

#if NET_SUPPORT_IPV6==1
		br = BindSharedIPV4And6(bindParameters, file, line);
#else
		br = BindSharedIPV4(bindParameters, file, line);
#endif

		if( br != JISBindResult_SUCCESS ) return br;
		unsigned long zero = 0;
		JISSendParams sendParams =
		{
			(char*) &zero,              // char *data;
			4,                                  // int length;
			this->boundAddress, // JACKIE_INET_Address systemAddress;
			0                                  // int ttl; 
		};

		JISSendResult sr = this->Send(&sendParams, TRACE_FILE_AND_LINE_);
		if( sr < 0 ) return JISBindResult_FAILED_SEND_TEST;
		memcpy(&this->binding, bindParameters, sizeof(JISBerkleyBindParams));
		return br;
	}
	JISBindResult JISBerkley::BindSharedIPV4(JISBerkleyBindParams *bindParameters,
		const char *file, UInt32 line)
	{
		memset(&boundAddress.address.addr4, 0, sizeof(sockaddr_in));
		boundAddress.address.addr4.sin_port = htons(bindParameters->port);
		boundAddress.address.addr4.sin_family = AF_INET;

		if( ( rns2Socket = (int) socket__(bindParameters->addressFamily, bindParameters->type, bindParameters->protocol) ) == -1 )
		{
			JACKIE_ASSERT(rns2Socket != -1);
			return JISBindResult_FAILED_BIND_SOCKET;
		}

		SetSocketOptions();
		SetNonBlockingSocket(bindParameters->nonBlockingSocket);
		SetBroadcastSocket(bindParameters->setBroadcast);
		SetIPHdrIncl(bindParameters->setIPHdrIncl);

		/// test hostAddress exists and != empty string
		if( bindParameters->hostAddress != 0 && bindParameters->hostAddress[0] != 0 )
		{
			boundAddress.address.addr4.sin_addr.s_addr =
				inet_addr__(bindParameters->hostAddress);
		} else
		{
			boundAddress.address.addr4.sin_addr.s_addr = INADDR_ANY;
		}

		// bind our address to the socket
		if( bind__(rns2Socket, ( struct sockaddr * ) &boundAddress.address.addr4, sizeof(sockaddr_in)) <= -1 )
		{
#if defined(_WIN32)
			closesocket__(rns2Socket);
#elif (defined(__GNUC__) || defined(__GCCXML__) ) && !defined(_WIN32)
			closesocket__(rns2Socket);
			switch( ret )
			{
				case EBADF:
					JACKIE_NET_DEBUG_PRINTF("bind__(): sockfd is not a valid descriptor.\n"); break;

				case ENOTSOCK:
					JACKIE_NET_DEBUG_PRINTF("bind__(): Argument is a descriptor for a file, not a socket.\n"); break;

				case EINVAL:
					JACKIE_NET_DEBUG_PRINTF("bind__(): The addrlen is wrong, or the socket was not in the AF_UNIX family.\n"); break;
				case EROFS:
					JACKIE_NET_DEBUG_PRINTF("bind__(): The socket inode would reside on a read-only file system.\n"); break;
				case EFAULT:
					JACKIE_NET_DEBUG_PRINTF("bind__(): my_addr points outside the user's accessible address space.\n"); break;
				case ENAMETOOLONG:
					JACKIE_NET_DEBUG_PRINTF("bind__(): my_addr is too long.\n"); break;
				case ENOENT:
					JACKIE_NET_DEBUG_PRINTF("bind__(): The file does not exist.\n"); break;
				case ENOMEM:
					JACKIE_NET_DEBUG_PRINTF("bind__(): Insufficient kernel memory was available.\n"); break;
				case ENOTDIR:
					JACKIE_NET_DEBUG_PRINTF("bind__(): A component of the path prefix is not a directory.\n"); break;
				case EACCES:
					// Port reserved on PS4
					JACKIE_NET_DEBUG_PRINTF("bind__(): Search permission is denied on a component of the path prefix.\n"); break;
				case ELOOP:
					JACKIE_NET_DEBUG_PRINTF("bind__(): Too many symbolic links were encountered in resolving my_addr.\n"); break;
				default:
					JACKIE_NET_DEBUG_PRINTF("Unknown bind__() error %i.\n", ret); break;
			}
#endif
			return JISBindResult_FAILED_BIND_SOCKET;
		}

		/// reinit bounfAddress to double check it is correct and fill out some other params
		/// INADDR_ANY will be changed to localhost ip adress
		GetSystemAddressIPV4(this->rns2Socket, &boundAddress);

		return JISBindResult_SUCCESS;
	}
	JISBindResult JISBerkley::BindSharedIPV4And6(JISBerkleyBindParams *bindParameters,
		const char *file, UInt32 line)
	{
		WSAStartupSingleton::AddRef();

#if NET_SUPPORT_IPV6 ==1

		int ret = 0;
		struct addrinfo *servinfo = 0, *aip;  // will point to the results

		struct addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo)); // make sure the struct is empty
		hints.ai_socktype = SOCK_DGRAM; // UDP sockets
		hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
		hints.ai_family = bindParameters->addressFamily;

		char portStr[32];
		::Itoa(bindParameters->port, portStr, 10);

		if( bindParameters->hostAddress != 0 &&
			( _stricmp(bindParameters->hostAddress, "UNASSIGNED_SYSTEM_ADDRESS") == 0
			|| bindParameters->hostAddress[0] == 0 ) )
		{
			// empty ip adress with port can work on Ubuntu
			getaddrinfo(0, portStr, &hints, &servinfo);
		} else if( bindParameters->hostAddress != 0 && bindParameters->hostAddress[0] != 0
			&& ( _stricmp(bindParameters->hostAddress, "UNASSIGNED_SYSTEM_ADDRESS") != 0 ) )
		{
			getaddrinfo(bindParameters->hostAddress, portStr, &hints, &servinfo);
		}

		// Try all returned addresses until one works
		for( aip = servinfo; aip != 0; aip = aip->ai_next )
		{
			// Open socket. The address type depends on what
			// getaddrinfo() gave us.
			rns2Socket = socket__(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
			if( rns2Socket == -1 ) return JISBindResult_FAILED_BIND_SOCKET;

			ret = bind__(rns2Socket, aip->ai_addr, (int) aip->ai_addrlen);
			if( ret >= 0 )
			{
				// Is this valid?
				memcpy(&boundAddress.address.addr6, aip->ai_addr, sizeof(boundAddress.address.addr6));

				freeaddrinfo(servinfo); // free the linked-list

				SetSocketOptions();
				SetNonBlockingSocket(bindParameters->nonBlockingSocket);
				SetBroadcastSocket(bindParameters->setBroadcast);
				SetIPHdrIncl(bindParameters->setIPHdrIncl);

				GetSystemAddressIPV4And6(rns2Socket, &boundAddress);

				return JISBindResult_SUCCESS;
			} else
			{
				closesocket__(rns2Socket);
			}
		}

		return JISBindResult_FAILED_BIND_SOCKET;
#else
		return JISBindResult_REQUIRES_NET_SUPPORT_IPV6_DEFINED;
#endif
		WSAStartupSingleton::Deref();
	}

	void JISBerkley::RecvFromBlockingIPV4(JISRecvParams *recvFromStruct)
	{
		//sockaddr_in sa;
		//memset(&sa, 0, sizeof(sockaddr_in));
		//sa.sin_family = AF_INET;
		//sa.sin_port = 0;
		//socklen_t sockLen = sizeof(sa);
		//socklen_t* socketlenPtr = (socklen_t*) &sockLen;
		//sockaddr* sockAddrPtr = (sockaddr*) &sa;
		//const int flag = 0;

		static const sockaddr_in sa = { AF_INET, /*PORT=0*/0 };
		static socklen_t sockLen = sizeof(sa);
		static socklen_t* socketlenPtr = (socklen_t*) &sockLen;
		static sockaddr* sockAddrPtr = (sockaddr*) &sa;
		static const int flag = 0;

		recvFromStruct->bytesRead = recvfrom__(GetSocket(),
			recvFromStruct->data,
			MAXIMUM_MTU_SIZE,
			flag,
			sockAddrPtr,
			socketlenPtr);

		/// there are only two resons for UDP recvfrom() return 0 :
		/// 1. Socket has been soft closed by shutdown() or setting up linear attribute
		/// 2. Receives an empty (0 size) message from remote endpoint 
		/// However, we cannot expect temote endpoint  will never sends 0 length mesage
		/// to us. So, this case will be noticed as kind of ERROR same to the return -1
		if( recvFromStruct->bytesRead <= 0 )
		{
#if defined(_WIN32) && !defined(_XBOX) && !defined(_XBOX_720_COMPILE_AS_WINDOWS) && !defined(X360) && defined(_DEBUG) && !defined(_XBOX_720_COMPILE_AS_WINDOWS) && !defined(WINDOWS_PHONE_8)
			DWORD dwIOError = WSAGetLastError();

			if( dwIOError == WSAECONNRESET )
			{
				JACKIE_NET_DEBUG_PRINTF("A previous send operation resulted in an ICMP Port Unreachable message.\n");
			} else if( dwIOError != WSAEWOULDBLOCK && dwIOError != WSAEADDRNOTAVAIL )
			{
				LPVOID messageBuffer;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, dwIOError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Default language
					(LPTSTR) & messageBuffer, 0, NULL);
				// something has gone wrong here...
				JACKIE_NET_DEBUG_PRINTF("recvfrom__ failed:Error code - %d\n%s", dwIOError, messageBuffer);

				//Free the buffer.
				LocalFree(messageBuffer);
			}
#else
			fprintf_s(stderr, "ERROR::recvfrom__ failed:: Got %i bytes from %s\n",
				recvFromStruct->bytesRead, recvFromStruct->systemAddress.ToString());
#endif
			return;
		}

		recvFromStruct->timeRead = GetTimeUS();
		recvFromStruct->systemAddress.SetPortNetworkOrder(sa.sin_port);
		recvFromStruct->systemAddress.address.addr4.sin_addr.s_addr = sa.sin_addr.s_addr;

		printf_s("--- Got %i bytes from %s\n", recvFromStruct->bytesRead, recvFromStruct->systemAddress.ToString());
	}
	void JISBerkley::RecvFromBlockingIPV4And6(JISRecvParams *recvFromStruct)
	{
#if  NET_SUPPORT_IPV6==1

		//sockaddr_storage sa;
		//memset(&sa, 0, sizeof(sa));
		//socklen_t sockLen = sizeof(sa);
		//socklen_t* socketlenPtr = (socklen_t*) &sockLen;
		//sockaddr* sockAddrPtr = (sockaddr*) &sa;
		//const int flag = 0;

		static sockaddr_storage sa = { 0 };
		static socklen_t sockLen = sizeof(sa);
		static socklen_t* socketlenPtr = (socklen_t*) &sockLen;
		static sockaddr* sockAddrPtr = (sockaddr*) &sa;
		static const int flag = 0;

		recvFromStruct->bytesRead = recvfrom__(rns2Socket, recvFromStruct->data, MAXIMUM_MTU_SIZE, flag, sockAddrPtr, socketlenPtr);

		if( recvFromStruct->bytesRead <= 0 )
		{
#if defined(_WIN32) && defined(_DEBUG) && !defined(WINDOWS_PHONE_8)
			DWORD dwIOError = GetLastError();
			if( dwIOError != 10035 )
			{
				LPVOID messageBuffer;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, dwIOError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Default language
					(LPTSTR) & messageBuffer, 0, NULL);
				// I see this hit on XP with IPV6 for some reason
				JACKIE_NET_DEBUG_PRINTF("Warning: recvfrom failed:Error code - %d\n%s", dwIOError, messageBuffer);
				LocalFree(messageBuffer);
			}
#endif
			return;
		}

		recvFromStruct->timeRead = GetTimeUS();

		if( sa.ss_family == AF_INET )
		{
			memcpy(&recvFromStruct->systemAddress.address.addr4, (sockaddr_in *) &sa, sizeof(sockaddr_in));
			recvFromStruct->systemAddress.debugPort = ntohs(recvFromStruct->systemAddress.address.addr4.sin_port);
		} else
		{
			memcpy(&recvFromStruct->systemAddress.address.addr6, (sockaddr_in6 *) &sa, sizeof(sockaddr_in6));
			recvFromStruct->systemAddress.debugPort = ntohs(recvFromStruct->systemAddress.address.addr6.sin6_port);
		}

#else
		RecvFromBlockingIPV4(recvFromStruct);
#endif
	}
	void JISBerkley::RecvFromNonBlockingIPV4(JISRecvParams *recvFromStruct) { }
	void JISBerkley::RecvFromNonBlockingIPV4And6(JISRecvParams *recvFromStruct) { }

	void JISBerkley::BlockOnStopRecvPollingThread(void)
	{
		endThreads = true;

		/// Change recvfrom to unbloking
		unsigned int zero = 0;
		JISSendParams sendParams = { (char*) &zero, sizeof(zero), this->boundAddress, 0 };
		this->Send(&sendParams, TRACE_FILE_AND_LINE_);

		::TimeMS timeout = ::GetTimeMS() + 1000;
		while( isRecvFromLoopThreadActive.GetValue() > 0 &&
			::GetTimeMS() < timeout )
		{
			// Get recvfrom to unblock
			this->Send(&sendParams, TRACE_FILE_AND_LINE_);
			JACKIE_Sleep(30);
		}
	}
	unsigned int JISBerkley::RecvFromLoopInt(void)
	{
		JISRecvParams* recvParams;
		isRecvFromLoopThreadActive.Increment();

		while( !endThreads )
		{
			recvParams = binding.eventHandler->AllocJISRecvParams(TRACE_FILE_AND_LINE_);
			if( recvParams != 0 )
			{
				recvParams->socket = this;
				RecvFromBlocking(recvParams);

				if( recvParams->bytesRead > 0 )
				{
					JACKIE_ASSERT(recvParams->systemAddress.GetPortHostOrder());
					binding.eventHandler->OnJISRecv(recvParams);
				} else
				{
					JACKIE_Sleep(0); /// why sleep 0 ms ?
					binding.eventHandler->DeallocJISRecvParams(
						recvParams, TRACE_FILE_AND_LINE_);
				}
			}
		}

		isRecvFromLoopThreadActive.Decrement();
		return 0;
	}

	JISSendResult JISBerkley::Send(JISSendParams *sendParameters,
		const char *file, UInt32 line)
	{
		return jst != 0 ? jst->JackieINetSendTo(sendParameters->data,
			sendParameters->length,
			sendParameters->systemAddress) :
			Send_Windows_Linux_360NoVDP(rns2Socket, sendParameters, file, line);
	}

	/* static */ void JISBerkley::GetSystemAddressIPV4(JISSocket rns2Socket,
		JACKIE_INET_Address *systemAddressOut)
	{
		sockaddr_in sa;
		memset(&sa, 0, sizeof(sockaddr_in));
		socklen_t len = sizeof(sa);

		getsockname__(rns2Socket, (sockaddr*) &sa, &len);
		systemAddressOut->SetPortNetworkOrder(sa.sin_port);
		systemAddressOut->address.addr4.sin_addr.s_addr = sa.sin_addr.s_addr;

		if( systemAddressOut->address.addr4.sin_addr.s_addr == INADDR_ANY )
		{
			systemAddressOut->address.addr4.sin_addr.s_addr = inet_addr__("127.0.0.1");
		}
	}
	/* static */ void JISBerkley::GetSystemAddressIPV4And6(JISSocket rns2Socket,
		JACKIE_INET_Address *systemAddressOut)
	{
#if NET_SUPPORT_IPV6 ==1

		socklen_t slen;
		sockaddr_storage ss;
		slen = sizeof(ss);

		if( getsockname__(rns2Socket, ( struct sockaddr * )&ss, &slen) != 0 )
		{
#if defined(_WIN32) && defined(_DEBUG)
			DWORD dwIOError = GetLastError();
			LPVOID messageBuffer;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwIOError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Default language
				(LPTSTR) & messageBuffer, 0, NULL);
			// something has gone wrong here...
			JACKIE_NET_DEBUG_PRINTF("getsockname failed:Error code - %d\n%s", dwIOError, messageBuffer);

			//Free the buffer.
			LocalFree(messageBuffer);
#endif
			systemAddressOut->FromString(0);
			return;
		}

		if( ss.ss_family == AF_INET )
		{
			memcpy(&systemAddressOut->address.addr4, (sockaddr_in *) &ss, sizeof(sockaddr_in));
			systemAddressOut->debugPort = ntohs(systemAddressOut->address.addr4.sin_port);

			UInt32 zero = 0;
			if( memcmp(&systemAddressOut->address.addr4.sin_addr.s_addr, &zero,
				sizeof(zero)) == 0 )
				systemAddressOut->SetToLoopBack(4);
		} else
		{
			memcpy(&systemAddressOut->address.addr6, (sockaddr_in6 *) &ss, sizeof(sockaddr_in6));
			systemAddressOut->debugPort = ntohs(systemAddressOut->address.addr6.sin6_port);

			char zero[16] = { 0 };
			if( memcmp(&systemAddressOut->address.addr4.sin_addr.s_addr, &zero,
				sizeof(zero)) == 0 )
				systemAddressOut->SetToLoopBack(6);
		}

#else
		GetSystemAddressIPV4(rns2Socket, systemAddressOut);
#endif
	}


#if defined(_WIN32) 
	void JISWins::GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR])
	{
		return GetMyIP_Wins_Linux(addresses);
	}
#else
	void JISLinux::GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR])
	{
		return GetMyIP_Wins_Linux(addresses);
	}
#endif
	////////////////////////////// JISBerkley implementations ////////////////////////////

#endif

	/////////////////////////////// JISAllocator starts /////////////////////////////////
	inline JACKIE_INet_Socket*  JISAllocator::AllocJIS(void)
	{
		JACKIE_INet_Socket* s2;
#if defined(WINDOWS_STORE_RT)
		s2 = JACKIE_INET::OP_NEW<JISWINSTROE8>(TRACE_FILE_AND_LINE_);
		s2->SetSocketType(JISType_WINDOWS_STORE_8);
#elif defined(__native_client__)
		s2 = JACKIE_INET::OP_NEW<RNS2_NativeClient>(TRACE_FILE_AND_LINE_);
		s2->SetSocketType(RNS2T_CHROME);
#elif defined(_WIN32)
		s2 = JACKIE_INET::OP_NEW<JISWins>(TRACE_FILE_AND_LINE_);
		s2->SetSocketType(JISType_WINDOWS);
#else
		s2 = JACKIE_INET::OP_NEW<JISLinux>(TRACE_FILE_AND_LINE_);
		s2->SetSocketType(JISType_LINUX);
#endif
		return s2;
	}
	inline void  JISAllocator::DeallocJIS(JACKIE_INet_Socket *s)
	{
		JACKIE_INET::OP_DELETE(s, TRACE_FILE_AND_LINE_);
	}
	//////////////////////////////// JISAllocator ends  ///////////////////////

}
