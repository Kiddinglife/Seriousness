#ifndef JACKIE_INET_SOCKET_H_
#define JACKIE_INET_SOCKET_H_

#include "DLLExport.h"
#include "NetTypes.h"
#include "JACKIE_Atomic.h"
#include "NetTime.h"
#include "OverrideMemory.h"
#include "JACKIE_Thread.h"

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

	enum JACKIE_EXPORT JISBindResult
	{
		JISBindResult_SUCCESS,
		JISBindResult_REQUIRES_NET_SUPPORT_IPV6_DEFINED,
		JISBindResult_FAILED_BIND_SOCKET,
		JISBindResult_FAILED_SEND_TEST,
	};

	enum JACKIE_EXPORT JISType
	{
		JISType_WINDOWS_STORE_8,
		JISType_PS3,
		JISType_PS4,
		JISType_CHROME,
		JISType_VITA,
		JISType_XBOX_360,
		JISType_XBOX_720,
		JISType_WINDOWS,
		JISType_LINUX
	};

	struct JACKIE_EXPORT JISSendParams
	{
		JISSendParams() { ttl = 0; }
		char *data;
		int length;
		JACKIE_INET_Address systemAddress;
		int ttl;
	};

	struct JACKIE_EXPORT JISRecvParams
	{
		char data[MAXIMUM_MTU_SIZE];
		int bytesRead;
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

		public:
		JACKIE_INet_Socket() : eventHandler(0) { }
		virtual ~JACKIE_INet_Socket() { }


		// In order for the handler to trigger, some platforms must call PollRecvFrom, 
		// some platforms this create an internal thread.
		void SetRecvEventHandler(JISEventHandler *_eventHandler) { eventHandler = _eventHandler; }
		JISEventHandler * GetEventHandler(void) const { return eventHandler; }
		virtual JISSendResult Send(JISSendParams *sendParameters,
			const char *file, UInt32 line) = 0;

		JISType GetSocketType(void) const { return socketType; }
		void SetSocketType(JISType t) { socketType = t; }

		UInt32 GetUserConnectionSocketIndex(void) const { return userConnectionSocketIndex; }
		void SetUserConnectionSocketIndex(UInt32 i) { userConnectionSocketIndex = i; }

		JACKIE_INET_Address GetBoundAddress(void) const { return boundAddress; }

		bool IsBerkleySocket(void) const
		{
			return socketType != JISType_CHROME && socketType != JISType_WINDOWS_STORE_8;
		}

		// ----------- STATICS ------------
		static void GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		static void DomainNameToIP(const char *domainName, char ip[65])
		{
			DomainNameToIP(domainName, ip);
		}
	};

#if defined(WINDOWS_STORE_RT)
#include "DS_List.h"
	ref class ListenerContext;
	class JISWINSTROE8 : public JACKIE_INet_Socket
	{
		public:
		JISWINSTROE8();
		~JISWINSTROE8();

		virtual JISSendResult Send( RNS2_SendParameters *sendParameters, const char *file, UInt32 line );
		JISBindResult Bind( Platform::String ^localServiceName );
		// ----------- STATICS ------------
		static void GetMyIP( SystemAddress addresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS] );
		static void DomainNameToIP( const char *domainName, char ip[65] );

		static int WinRTInet_Addr(const char * cp);

		static int WinRTSetSockOpt(Windows::Networking::Sockets::DatagramSocket ^s,
			int level,
			int optname,
			const char * optval,
			socklen_t optlen);

		static int WinRTIOCTLSocket(Windows::Networking::Sockets::DatagramSocket ^s,
			long cmd,
			unsigned long *argp);

		static int WinRTGetSockName(Windows::Networking::Sockets::DatagramSocket ^s,
		struct sockaddr *name,
			socklen_t* namelen);

		static JISWINSTROE8 *GetRNS2FromDatagramSocket(Windows::Networking::Sockets::DatagramSocket^ s);
		protected:
		static DataStructures::List<JISWINSTROE8*> rns2List;
		static SimpleMutex rns2ListMutex;

		Windows::Networking::Sockets::DatagramSocket^ listener;
		ListenerContext^ listenerContext;
	};
#else

	struct JACKIE_EXPORT JISBerkleyBindParams
	{
		// Input parameters
		unsigned short port;
		char *hostAddress;
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

	// Every platform except Windows Store 8 can use the Berkley sockets interface
	class JACKIE_EXPORT IJISBerkley : public JACKIE_INet_Socket
	{
		public:
		/// For addressFamily, use AF_INET  For type, use SOCK_DGRAM
		static bool IsPortInUse(unsigned short port, const char *hostAddress,
			unsigned short addressFamily, int type);

		virtual JISBindResult Bind(JISBerkleyBindParams *bindParameters,
			const char *file, UInt32 line) = 0;
	};

	// Every platform that uses Berkley sockets, except native client, can compile some common functions
	class JACKIE_EXPORT JISBerkley : public IJISBerkley
	{
		public:
		JISBerkley() { }
		virtual ~JISBerkley() { }
		int CreateRecvPollingThread(int threadPriority);
		void SignalStopRecvPollingThread(void);
		void BlockOnStopRecvPollingThread(void);
		const JISBerkleyBindParams *GetBindings(void) const;
		JISSocket GetSocket(void) const;
		void SetDoNotFragment(int opt);

		protected:
		// Used by other classes
		JISBindResult BindShared(JISBerkleyBindParams *bindParameters,
			const char *file, UInt32 line);
		JISBindResult BindSharedIPV4(JISBerkleyBindParams *bindParameters,
			const char *file, UInt32 line);
		JISBindResult BindSharedIPV4And6(JISBerkleyBindParams *bindParameters,
			const char *file, UInt32 line);

		static void GetSystemAddressIPV4(JISSocket rns2Socket, JACKIE_INET_Address *systemAddressOut);
		static void GetSystemAddressIPV4And6(JISSocket rns2Socket, JACKIE_INET_Address *systemAddressOut);

		// Internal
		void SetNonBlockingSocket(unsigned long nonblocking);
		void SetSocketOptions(void);
		void SetBroadcastSocket(int broadcast);
		void SetIPHdrIncl(int ipHdrIncl);
		void RecvFromBlocking(JISRecvParams *recvFromStruct);
		void RecvFromBlockingIPV4(JISRecvParams *recvFromStruct);
		void RecvFromBlockingIPV4And6(JISRecvParams *recvFromStruct);

		JISSocket rns2Socket;
		JISBerkleyBindParams binding;

		unsigned RecvFromLoopInt(void);
		JACKIE_ATOMIC_LONG isRecvFromLoopThreadActive;
		volatile bool endThreads;
		// Constructor not called!

#if defined(__APPLE__)
		// http://sourceforge.net/p/open-dis/discussion/683284/thread/0929d6a0
		CFSocketRef             _cfSocket;
#endif

		static JACKIE_THREAD_DECLARATION(RecvFromLoop);
	};

#if defined(_WIN32) 
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
		// Return -1 to use JackieNet's normal recvfrom, 0 to abort JackieNet's normal recvfrom, 
		/// and positive to return data
		virtual int JackieINetRecvFrom(char dataOut[MAXIMUM_MTU_SIZE],
			JACKIE_INET_Address *senderOut, bool calledFromMainThread) = 0;
	};
	class JACKIE_EXPORT JISWins : public JISBerkley //, public JISWinLinux360
	{
		public:
		JISWins();
		virtual ~JISWins();
		JISBindResult  Bind(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);
		JISSendResult Send(JISSendParams *sendParameters, const char *file, UInt32 line);
		void SetSocketLayerOverride(JACKIE_ISocketTransceiver *_slo);
		JACKIE_ISocketTransceiver* GetSocketLayerOverride(void);
		// ----------- STATICS ------------
		static void GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		static JISSendResult SendWinsLinux360WithoutVDP(JISSocket rns2Socket,
			JISSendParams *sendParameters, const char *file, UInt32 line);

		protected:
		static void GetMyIPIPV4(JACKIE_INET_Address addrs[MAX_COUNT_LOCAL_IP_ADDR]);
		static void GetMyIPV4AndV6(JACKIE_INET_Address addrs[MAX_COUNT_LOCAL_IP_ADDR]);
		JACKIE_ISocketTransceiver *slo;
	};
#else
	class JISLinux : public JISBerkley //, public JISWinLinux360
	{
		public:
		JISBindResult Bind(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line);
		JISSendResult Send(JISSendParams *sendParameters, const char *file, UInt32 line);

		// ----------- STATICS ------------
		static void GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR]);
		static JISSendResult SendWinsLinux360WithoutVDP(JISSocket rns2Socket,
			JISSendParams *sendParameters, const char *file, UInt32 line);

		protected:
		static void GetMyIPIPV4(JACKIE_INET_Address addrs[MAX_COUNT_LOCAL_IP_ADDR]);
		static void GetMyIPV4AndV6(JACKIE_INET_Address addrs[MAX_COUNT_LOCAL_IP_ADDR]);
	};
#endif

#endif

}
#endif

