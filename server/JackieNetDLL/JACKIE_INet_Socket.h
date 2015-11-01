#ifndef JACKIE_INET_SOCKET_H_
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
	class  JackieINetSocket;
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
	JACKIE_EXPORT extern const char* JISBindResultToString(JISBindResult reason);

	struct JACKIE_EXPORT JISSendParams
	{
		char *data;
		int length;
		JISSendResult bytesWritten; // use 0 to init
		JACKIE_INET_Address receiverINetAddress;
		int ttl;
	};

	struct JACKIE_EXPORT JISRecvParams
	{
		char data[MAXIMUM_MTU_SIZE];
		JISRecvResult bytesRead;
		JACKIE_INET_Address senderINetAddress;
		TimeUS timeRead;
		JackieINetSocket *socket;
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
		static JackieINetSocket* AllocJIS(void);
		static void DeallocJIS(JackieINetSocket *s);
	};

	class JACKIE_EXPORT JackieINetSocket
	{
		protected:
		JISEventHandler *eventHandler;
		JACKIE_INET_Address boundAddress;
		UInt32 userConnectionSocketIndex;
		JISType socketType;

		public:
		JackieINetSocket() : eventHandler(0) { }
		virtual ~JackieINetSocket() { }


		// In order for the handler to trigger, some platforms must call PollRecvFrom, 
		// some platforms this create an internal thread.
		inline void SetRecvEventHandler(JISEventHandler *_eventHandler) { eventHandler = _eventHandler; }
		inline JISEventHandler * GetEventHandler(void) const { return eventHandler; }

		inline JISType GetSocketType(void) const { return socketType; }
		inline void SetSocketType(JISType t) { socketType = t; }

		inline UInt32 GetUserConnectionSocketIndex(void) const { return userConnectionSocketIndex; }
		inline void SetUserConnectionSocketIndex(UInt32 i) { userConnectionSocketIndex = i; }

		inline JACKIE_INET_Address GetBoundAddress(void) const { return boundAddress; }

		inline bool IsBerkleySocket(void) const
		{
			return socketType != JISType_CHROME && socketType != JISType_WINDOWS_STORE_8;
		}

		static void GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		inline static void DomainNameToIP(const char *domainName, char ip[65])
		{
			DomainNameToIP(domainName, ip);
		}

		/// Text-print the intenal memebers in this class
		virtual void Print(void);
	};

#if defined (WINDOWS_STORE_RT) 	/// We are using WINDOWS_STORE_RT plateform
	//@TODO
#elif defined (__native_client__)  	/// We are using NaCI plateform
	//@TODO
#else 	/// We are using Wins or LINUX or Unix plateform
	struct JACKIE_EXPORT JISBerkleyBindParams
	{
		// Input parameters
		unsigned short port;
		char *hostAddress; // must be number ip adress, using 'localhost' is wrong
		unsigned short addressFamily; // AF_INET or AF_INET6
		int type; // SOCK_DGRAM
		int protocol; // 0
		bool isBlocKing;
		int isBroadcast;
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
		virtual int JackieINetSendTo(const char *data, int length, const JACKIE_INET_Address &systemAddress) = 0;

		/// Called when RecvFrom would otherwise occur. 
		/// Return number of bytes read and Write data into dataOut
		/// Return -1 to use JackieNet's normal recvfrom, 0 to abort JackieNet's normal 
		/// recvfrom,and positive to return data
		virtual int JackieINetRecvFrom(char dataOut[MAXIMUM_MTU_SIZE], JACKIE_INET_Address *senderOut, bool calledFromMainThread) = 0;

		/// RakNet needs to know whether an address is a dummy override address, 
		/// so it won't be added as an external addresses
		virtual bool IsFork(const JACKIE_INET_Address &systemAddress) const = 0;
	};

	class JACKIE_EXPORT JISBerkley : public JackieINetSocket
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
		JISBerkley();
		virtual ~JISBerkley();


		/// Use these three functions when you want a duplicate thread . to poll recvfrom(...)
		int CreateRecvPollingThread(int threadPriority);
		void BlockOnStopRecvPollingThread(void);
		inline void SignalStopRecvPollingThread(void) { endThreads = true; }

		const JISBerkleyBindParams *GetBindingParams(void) const { return &binding; }
		inline JISSocket GetSocket(void) const { return rns2Socket; }

		inline void SetSocketTransceiver(JACKIE_ISocketTransceiver *jst_) { this->jst = jst_; }
		inline JACKIE_ISocketTransceiver* GetSocketTransceiver(void) const { return this->jst; }

		//////////////////////////////////////////////////////////////////////////
		/// 1. Expecially, this function will internally test if the binding really works 
		/// 2. by sending a char to itself and receive it, only if send and receive both succeed, 
		/// 3. binded is finally considered as successful
		//////////////////////////////////////////////////////////////////////////
		JISBindResult Bind(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);
		JISBindResult BindShared(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);
		JISBindResult BindSharedIPV4(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);
		JISBindResult BindSharedIPV4And6(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);

		//////////////////////////////////////////////////////////////////////////
		/// 1. Used internally in @mtd JISBindResult Bind(...)
		/// 2. set nonblocking to 0 = blocking-socket; 
		/// 3. set nonblocking  to 1 = nonblocking-socket; 
		/// 4. setsockopt() will always return 0 if succeed otherwise return < 0
		/// 5. do not support reuse addr which means you cannot to a in-use port
		//////////////////////////////////////////////////////////////////////////
		inline void SetSocketOptions(void)
		{
			int returnVal = SOCKET_ERROR;

			/// do not support reuse addr which means you cannot to a in - use port
			//returnVal = setsockopt(rns2Socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &returnVal, sizeof(returnVal));
			//JACKIE_ASSERT(returnVal == 0);

			// This doubles the max throughput rate
			returnVal = JACKIE_SO_REVBUF_SIZE;
			returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_RCVBUF, (char *) & returnVal, sizeof(returnVal));
			JACKIE_ASSERT(returnVal == 0);

			// This doesn't make much difference: 10% maybe Not supported on console 2
			returnVal = JACKIE_SO_SNDBUF_SIZE;
			returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_SNDBUF, (char *) & returnVal, sizeof(returnVal));
			JACKIE_ASSERT(returnVal == 0);

			//////////////////////////////////////////////////////////////////////////
			/// Immediate fore-close with ignoring the buffered sending data. 
			/// voice, xbox and windows's SOCK_DGRAM does not support 
			/// SO_DONTLINGER, SO_KEEPALIVE, SO_LINGER and SO_OOBINLINE
			//////////////////////////////////////////////////////////////////////////
			returnVal = 0;
			returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_LINGER,
				(char *) & returnVal, sizeof(returnVal));
			//JACKIE_ASSERT(returnVal == 0);

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

		//////////////////////////////////////////////////////////////////////////
		/// 1. return value == 0 - sender sends us an msg with length 0 for some reason
		/// 2. return value < 0 - other error occurs (EWOUDBLOCK exlusive) in recvform()
		/// 3. returnvalue  > 0 - size of read bytes
		/// 4. RecvFromLoopInt() will autimatically use the block mode setup in @mem binding
		//////////////////////////////////////////////////////////////////////////
		JISRecvResult RecvFrom(JISRecvParams *recvFromStruct);
		JISRecvResult RecvFromIPV4(JISRecvParams *recvFromStruct);
		JISRecvResult RecvFromIPV4And6(JISRecvParams *recvFromStruct);

		//////////////////////////////////////////////////////////////////////////
		/// 1. send by jst if not null, otherwise by @mtd SendWithoutVDP(...)
		/// 2. Returns value is either > 0(send succeeds) or <0 (send error)
		/// 3. ! WILL NEVER return 0 because we never send empty msg to the receiver
		//////////////////////////////////////////////////////////////////////////
		JISSendResult Send(JISSendParams *sendParameters, const char *file, UInt32 line);
		JISSendResult SendWithoutVDP(JISSocket rns2Socket, JISSendParams *sendParameters, const char *file, unsigned int line);

		/// Constructor not called at this monment !
		static JACKIE_THREAD_DECLARATION(RecvFromLoop);
		static bool IsPortInUse(unsigned short port, const char *hostAddress, unsigned short addressFamily, int type);

		//////////////////////////////////////////////////////////////////////////
		/// 1. use gethostname(...) to get host name like mengdizhang,
		/// 2. use this host name in gethostbyname(...) to het a list of avaiable ip addrs
		/// 3. if addrs num < MAX_COUNT_LOCAL_IP_ADDR, the rest of them will be filled with empty addr
		//////////////////////////////////////////////////////////////////////////
		static void GetMyIPBerkley(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		static void GetMyIPBerkleyV4V6(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		static void GetMyIPBerkleyV4(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);

		virtual void Print(void);

		protected:
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// 1. Notice that you can only call this function after calling Bind(...) to associate @mem rns2Socket with @mem systemAddressOut
		/// 2. Individually call this function will get unkonwn error, so they are marked as protected functions only for internal use
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		static void GetSystemAddressViaJISSocket(JISSocket rns2Socket, JACKIE_INET_Address *systemAddressOut);
		static void GetSystemAddressViaJISSocketIPV4(JISSocket rns2Socket, JACKIE_INET_Address *systemAddressOut);
		static void GetSystemAddressViaJISSocketIPV4And6(JISSocket rns2Socket, JACKIE_INET_Address *systemAddressOut);
	};
#endif
}
#endif

