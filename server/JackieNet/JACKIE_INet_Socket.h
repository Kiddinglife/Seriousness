﻿#ifndef JACKIE_INET_SOCKET_H_
#define JACKIE_INET_SOCKET_H_

#include "DLLExport.h"
#include "JACKIE_Atomic.h"
#include "JACKIE_Thread.h"
#include "NetTime.h"
#include "NetTypes.h"
#include "GlobalFunctions.h" // only for JackieSleep() global function
#include "OverrideMemory.h"

// #define TEST_NATIVE_CLIENT_ON_WINDOWS
#ifdef TEST_NATIVE_CLIENT_ON_WINDOWS
#define __native_client__
typedef int PP_Resource;
#endif

namespace JACKIE_INET
{
	/// JIS is short name for JACKIE_INet_Socket 
	class  JACKIE_INet_Socket;
	struct JIS_BerkleyBindParams;
	struct JIS_SendParams;

	typedef int JISSocket;
	typedef int JISSendResult;
	typedef int JISRecvResult;

	enum  JISBindResult
	{
		JISBindResult_SUCCESS = 0,
		JISBindResult_REQUIRES_NET_SUPPORT_IPV6_DEFINED,
		JISBindResult_FAILED_BIND_SOCKET,
		JISBindResult_FAILED_SEND_RECV_TEST,
	};
	JACKIE_EXPORT extern const char* JISBindResultToString(JISBindResult reason);

	enum  JISType
	{
		JISType_WINDOWS_STORE_8 = 0,
		JISType_PS3,
		JISType_PS4,
		JISType_CHROME,
		JISType_VITA,
		JISType_XBOX_360,
		JISType_XBOX_720,
		JISType_WINDOWS,
		JISType_LINUX
	};
	JACKIE_EXPORT extern const char* JISTypeToString(JISType reason);

	struct JACKIE_EXPORT JISSendParams
	{
		char *data;
		int length;
		JISSendResult bytesWritten; // use 0 to init
		JACKIE_INET_Address systemAddress;
		int ttl;
	};

	struct JACKIE_EXPORT JISRecvParams
	{
		char data[MAXIMUM_MTU_SIZE];
		JISRecvResult bytesRead;
		JACKIE_INET_Address systemAddress;
		TimeUS timeRead;
		JACKIE_INet_Socket *socket;
	};

	class JACKIE_EXPORT JISEventHandler
	{
		public:
		JISEventHandler() { }
		virtual ~JISEventHandler() { }

		virtual void OnJISRecv(JISRecvParams *recvStruct) = 0;
		virtual void DeallocJISRecvParams(JISRecvParams *s, const char *file, UInt32 line) = 0;
		virtual JISRecvParams *AllocJISRecvParams(const char *file, UInt32 line) = 0;
	};

	class JACKIE_EXPORT JISAllocator
	{
		public:
		static JACKIE_INet_Socket* AllocJIS(void);
		static void DeallocJIS(JACKIE_INet_Socket *s);
	};

	class JACKIE_EXPORT JACKIE_INet_Socket
	{
		protected:
		JISEventHandler *eventHandler;
		JACKIE_INET_Address boundAddress;
		UInt32 userConnectionSocketIndex;
		JISType socketType;

		/// Text-print the intenal memebers in this class
		virtual void Print(void);

		public:
		JACKIE_INet_Socket() : eventHandler(0) { }
		virtual ~JACKIE_INet_Socket() { }


		// In order for the handler to trigger, some platforms must call PollRecvFrom, 
		// some platforms this create an internal thread.
		void SetRecvEventHandler(JISEventHandler *_eventHandler) { eventHandler = _eventHandler; }
		JISEventHandler * GetEventHandler(void) const { return eventHandler; }

		JISType GetSocketType(void) const { return socketType; }
		void SetSocketType(JISType t) { socketType = t; }

		UInt32 GetUserConnectionSocketIndex(void) const
		{ return userConnectionSocketIndex; }
		void SetUserConnectionSocketIndex(UInt32 i) { userConnectionSocketIndex = i; }

		JACKIE_INET_Address GetBoundAddress(void) const { return boundAddress; }

		bool IsBerkleySocket(void) const
		{
			return socketType != JISType_CHROME && socketType != JISType_WINDOWS_STORE_8;
		}

		// ----------- STATICS ------------
		void GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		static void DomainNameToIP(const char *domainName, char ip[65])
		{
			DomainNameToIP(domainName, ip);
		}
	};

	/// Macros are based on if supporting berkley implementation or not 
#if defined(WINDOWS_STORE_RT)
	// Every platform except Windows Store 8 can use the Berkley sockets interface
	/// @TO-DO
#elif defined(__native_client__) 	
	/// Every platform that uses Berkley sockets,
	///except native client, can share some common functions like bind, sendto() and recvfrom()
	/// @TO-DO
#else
	/// linux-like and wins systen go here
#if defined(_WIN32) || defined(__GNUC__)  || defined(__GCCXML__) || defined(__S3E__)
	extern JACKIE_EXPORT JISSendResult Send_Windows_Linux_360NoVDP(
		JISSocket rns2Socket,
		JISSendParams *sendParameters,
		const char *file, unsigned int line);
#endif

	struct JACKIE_EXPORT JISBerkleyBindParams
	{
		// Input parameters
		unsigned short port;
		char *hostAddress; // must be number ip adress, using 'localhost' is wrong
		unsigned short addressFamily; // AF_INET or AF_INET6
		int type; // SOCK_DGRAM
		int protocol; // 0
		bool nonBlockingSocket;
		int setBroadcast;
		int setIPHdrIncl;
		int doNotFragment;
		int pollingThreadPriority;
		JISEventHandler *eventHandler;
		unsigned short remotePortJackieNetWasStartedOn_PS3_PS4_PSP2;
	};

	class JACKIE_EXPORT JACKIE_ISocketTransceiver
	{
		public:
		JACKIE_ISocketTransceiver() { }
		virtual ~JACKIE_ISocketTransceiver() { }

		/// Called when SendTo would otherwise occur.
		virtual int JackieINetSendTo(const char *data, int length,
			const JACKIE_INET_Address &systemAddress) = 0;

		/// Called when RecvFrom would otherwise occur. 
		/// Return number of bytes read and Write data into dataOut
		/// Return -1 to use JackieNet's normal recvfrom, 0 to abort JackieNet's normal 
		/// recvfrom,and positive to return data
		virtual int JackieINetRecvFrom(char dataOut[MAXIMUM_MTU_SIZE],
			JACKIE_INET_Address *senderOut, bool calledFromMainThread) = 0;

		/// RakNet needs to know whether an address is a dummy override address, 
		/// so it won't be added as an external addresses
		virtual bool IsFork(const JACKIE_INET_Address &systemAddress) const = 0;
	};

	class JACKIE_EXPORT JISBerkley : public JACKIE_INet_Socket
	{
		protected:
		JISSocket rns2Socket;
		JISBerkleyBindParams binding;
		JACKIE_ISocketTransceiver *jst;
		JACKIE_ATOMIC_LONG isRecvFromLoopThreadActive;
		volatile bool endThreads;

#if defined(__APPLE__)
		// http://sourceforge.net/p/open-dis/discussion/683284/thread/0929d6a0
		CFSocketRef             _cfSocket;
#endif

		public:
		JISBerkley() { rns2Socket = (JISSocket) INVALID_SOCKET; jst = 0; }
		virtual ~JISBerkley() { if( rns2Socket != INVALID_SOCKET ) closesocket__(rns2Socket); }

		inline int CreateRecvPollingThread(int threadPriority)
		{
			endThreads = false;
			int errorCode = JACKIE_Thread::Create(RecvFromLoop, this, threadPriority);
			return errorCode;
		}
		inline void SignalStopRecvPollingThread(void) { endThreads = true; }
		void BlockOnStopRecvPollingThread(void);

		const JISBerkleyBindParams *GetBindingParams(void) const { return &binding; }
		inline JISSocket GetSocket(void) const { return rns2Socket; }

		inline void SetSocketTransceiver(JACKIE_ISocketTransceiver *jst_) { this->jst = jst_; }
		inline JACKIE_ISocketTransceiver* GetSocketTransceiver(void) const { return this->jst; }

		static bool IsPortInUse(unsigned short port, const char *hostAddress,
			unsigned short addressFamily, int type);

		void GetSystemAddressIPV4(JISSocket rns2Socket, JACKIE_INET_Address *systemAddressOut);
		void GetSystemAddressIPV4And6(JISSocket rns2Socket, JACKIE_INET_Address *systemAddressOut);

		JISBindResult Bind(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);
		JISBindResult BindShared(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);
		JISBindResult BindSharedIPV4(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);
		JISBindResult BindSharedIPV4And6(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);

		/// nonblocking = 0 means blocking;  nonblocking != 0 means nonblocking; 
		/// setsockopt() will always return 0 if succeed otherwise return < 0
		inline void SetSocketOptions(void)
		{
			int returnVal = 1;

			returnVal = setsockopt(rns2Socket, SOL_SOCKET, SO_REUSEADDR,
				(const char*) &returnVal, sizeof(returnVal));

			JACKIE_ASSERT(returnVal == 0);

			// This doubles the max throughput rate
			returnVal = JACKIE_SO_REVBUF_SIZE;
			returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_RCVBUF,
				(char *) & returnVal, sizeof(returnVal));

			JACKIE_ASSERT(returnVal == 0);

			/// Immediate fore-close with ignoring the buffered sending data. 
			/// voice, xbox and windows's SOCK_DGRAM does not support 
			/// SO_DONTLINGER, SO_KEEPALIVE, SO_LINGER and SO_OOBINLINE
			returnVal = 0;
			returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_LINGER,
				(char *) & returnVal, sizeof(returnVal));

#ifndef WIN32
			JACKIE_ASSERT(returnVal == 0);
#endif

			// This doesn't make much difference: 10% maybe
			// Not supported on console 2
			returnVal = JACKIE_SO_SNDBUF_SIZE;
			returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_SNDBUF,
				(char *) & returnVal, sizeof(returnVal));

			JACKIE_ASSERT(returnVal == 0);

		}
		inline void SetNonBlockingSocket(unsigned long nonblocking)
		{
			int res;
#ifdef _WIN32
			res = ioctlsocket__(rns2Socket, FIONBIO, &nonblocking);
#else
			if( nonblocking > 0 )
			{
				int flags = fcntl(socket, F_GETFL, 0);
				res = fcntl(rns2Socket, F_SETFL, flags | O_NONBLOCK); // setup to non-blocking
			}
			else
				res = fcntl(rns2Socket, F_SETFL, 0); // setup to blocking
#endif
			JACKIE_ASSERT(res == 0);
		}
		inline void SetBroadcastSocket(int broadcast)
		{
			JACKIE_ASSERT(setsockopt__(rns2Socket, SOL_SOCKET, SO_BROADCAST,
				(char *) & broadcast, sizeof(broadcast)) == 0);
		}
		inline void SetIPHdrIncl(int ipHdrIncl)
		{
			//int val = setsockopt__(rns2Socket, IPPROTO_IP, IP_HDRINCL,
			//(char*) &ipHdrIncl, sizeof(ipHdrIncl));
			int val = setsockopt__(rns2Socket, IPPROTO_IP, SO_DONTLINGER,
				(const char*) &ipHdrIncl, sizeof(ipHdrIncl));
			/// this assert always fail maybe need admin permission
			/// JACKIE_ASSERT(val == 0);
		}
		inline void SetDoNotFragment(int opt)
		{
#if defined( IP_DONTFRAGMENT )
#if defined(_WIN32) && !defined(_DEBUG)
			// If this assert hit you improperly linked against WSock32.h
			JACKIE_ASSERT(IP_DONTFRAGMENT == 14);
#endif
			opt = setsockopt__(rns2Socket, boundAddress.GetIPProtocol(), IP_DONTFRAGMENT,
				(char *) & opt, sizeof(opt));
			JACKIE_ASSERT(opt == 0);
#endif
		}

		unsigned int RecvFromLoopInt(void);
		JISRecvResult RecvFromBlocking(JISRecvParams *recvFromStruct);
		JISRecvResult RecvFromBlockingIPV4(JISRecvParams *recvFromStruct);
		JISRecvResult RecvFromBlockingIPV4And6(JISRecvParams *recvFromStruct);

		JISRecvResult RecvFromNonBlocking(JISRecvParams *recvFromStruct);
		JISRecvResult RecvFromNonBlockingIPV4(JISRecvParams *recvFromStruct);
		JISRecvResult RecvFromNonBlockingIPV4And6(JISRecvParams *recvFromStruct);

		// send by jst if not null, otherwise by SendWithoutVDP
		JISSendResult Send(JISSendParams *sendParameters, const char *file, UInt32 line);
		JISSendResult SendWithoutVDP(JISSocket rns2Socket, JISSendParams *sendParameters,
			const char *file, unsigned int line);

		/// Constructor not called at this monment !
		static JACKIE_THREAD_DECLARATION(RecvFromLoop);



		static void GetMyIPBerkley(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		static void GetMyIPBerkleyV4V6(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		static void GetMyIPBerkleyV4(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
	};

#endif

}
#endif

