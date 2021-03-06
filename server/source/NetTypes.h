/*!
 * \file NetTypes.h
 * \author mengdi
 * \date Oct 2015
 * RakNetTypes.h, INetApplication = ServerApplication
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
#ifndef NET_TYPES_H_
#define NET_TYPES_H_

#include "DLLExport.h"
#include"DefaultNetDefines.h"
#include "BasicTypes.h"
#include "WindowsIncludes.h"
#include "SockOSIncludes.h"
#include "SocketDefines.h"
#include "NetTime.h"
#include "JackieReliabler.h"
#include "CompileFeatures.h"
#include "SecurityHandShake.h"

namespace JACKIE_INET
{
	/// Forward declarations
	class   INetApplication;
	class   JackieBits;
	struct  JackiePacket;
	struct  JackieAddress;
	struct  JackieGUID;
	class JackieINetSocket;

	//////////////////////////////////////////////////////////////////////////
	/// @Internal Defines the default maximum transfer unit.
	/// \li \em 17914 16 Mbit/Sec Token Ring
	/// \li \em 4464 4 Mbits/Sec Token Ring
	/// \li \em 4352 FDDI
	/// \li \em 1500. The largest Ethernet packet size \b recommended. 
	/// This is the typical setting for non-PPPoE, non-VPN connections. 
	/// The default value for NETGEAR routers, adapters and switches.
	/// \li \em 1492. The size PPPoE prefers.
	/// \li \em 1472. Maximum size to use for pinging. (Bigger packets are fragmented.)
	/// \li \em 1468. The size DHCP prefers.
	/// \li \em 1460. Usable by AOL if you don't have large email attachments, etc.
	/// \li \em 1430. The size VPN and PPTP prefer.
	/// \li \em 1400. Maximum size for AOL DSL.
	/// \li \em 576. Typical value to connect to dial-up ISPs.
	/// The largest value for an UDP datagram
	//////////////////////////////////////////////////////////////////////////
#define MAXIMUM_MTU_SIZE 1492
#define Token_Ring_MAX_MTU_SIZE 17914
#define MINIMUM_MTU_SIZE 400

	/// try inginitely when pushtail operation return false if memory out
#define QUEUE_PUSH_TAIL(Queue, ELEMENT)\
bool result = false;\
do{result = Queue.PushTail(ELEMENT);if( !result ) JACKIE_Sleep(10);} while( !result )

#define CLIENT_QUEUE_PTR_SIZE 25
#define SERVER_QUEUE_PTR_SIZE 2500

#define USE_NON_BLOBKING_SOCKET true
#define USE_BLOBKING_SOCKET false

	/// Given a number of bits, 
	/// return how many bytes are needed to hold all bits.
	/// For example, 17 bits will need 3 bytes to hold. 
#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

#if defined(_MSC_VER) && _MSC_VER > 0
#define PRINTF_64BITS_MODIFIER "I64"
#else
#define PRINTF_64BITS_MODIFIER "ll"
#endif

	//#define SEND_10040_ERR(sock, sendParams)\
					//if (sock->Send(&sendParams, TRACE_FILE_AND_LINE_) <= 0 &&\
					//sendParams.bytesWritten == 10040)\
					//{JERROR <<"line 660::void ServerApplication::StopRecvThread()::return 10040 error !!!";}


	/// \sa NetworkIDObject.h
	typedef UInt8   UniqueIDType;
	typedef UInt8   RPCIndex;
	typedef UInt16 SystemIndex;
	typedef UInt64 NetworkID;
	typedef UInt32 BitSize;
	typedef UInt32 ByteSize;
	typedef UInt8 MessageID; ///< First byte of a network message

	/// Index of an invalid JACKIE_INET_Address and JACKIE_INet_GUID
#ifndef SWIG
	JACKIE_EXPORT extern const  JackieAddress JACKIE_NULL_ADDRESS;
	JACKIE_EXPORT extern const  JackieGUID JACKIE_NULL_GUID;
#endif

	const SystemIndex UNASSIGNED_PLAYER_INDEX = 65535; ///  Index of an unassigned player
	const NetworkID UNASSIGNED_NETWORK_ID = (UInt64)-1; /// Unassigned object ID
	const int MAX_RPC_MAP_SIZE = ((RPCIndex)-1) - 1; // 254
	const int UNDEFINED_RPC_INDEX = ((RPCIndex)-1); // 255
	const int PING_TIMES_ARRAY_SIZE = 5;
	//const extern char *IPV6_LOOPBACK = "::1";
	//const extern char *IPV4_LOOPBACK = "127.0.0.1";
	const extern char *IPV6_LOOPBACK;
	const extern char *IPV4_LOOPBACK;

	enum ThreadType { RecvThreadType, SendThreadType };

	enum StartupResult
	{
		START_SUCCEED,
		ALREADY_STARTED,
		INVALID_JACKIE_LOCAL_SOCKET,
		INVALID_MAX_CONNECTIONS,
		SOCKET_FAMILY_NOT_SUPPORTED,
		SOCKET_PORT_ALREADY_IN_USE,
		SOCKET_FAILED_TO_BIND,
		SOCKET_FAILED_TEST_SEND_RECV,
		PORT_CANNOT_BE_ZERO,
		FAILED_TO_CREATE_NETWORK_UPDATE_THREAD,
		FAILED_TO_CREATE_RECV_THREAD,
		COULD_NOT_GENERATE_GUID,
		STARTUP_OTHER_FAILURE
	};

	enum ConnectionAttemptResult
	{
		CONNECTION_ATTEMPT_POSTED,
		INVALID_PARAM,
		CANNOT_RESOLVE_DOMAIN_NAME,
		ALREADY_CONNECTED_TO_ENDPOINT,
		CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS,
		SECURITY_INITIALIZATION_FAILED
	};

	/// Returned from INetApplication::GetConnectionState()
	enum ConnectionState
	{
		/// Connect() was called, but the process hasn't started yet
		IS_PENDING,
		/// Processing the connection attempt
		IS_CONNECTING,
		/// Is connected and able to communicate
		IS_CONNECTED,
		/// Was connected, but will disconnect as soon as the remaining messages are delivered
		IS_DISCONNECTING,
		/// A connection attempt failed and will be aborted
		IS_SILENTLY_DISCONNECTING,
		/// No longer connected
		IS_DISCONNECTED,
		/// Was never connected, or else was disconnected long enough ago that the entry ha
		/// s been discarded
		IS_NOT_CONNECTED
	};

	/// Used with the PublicKey structure
	enum SecureConnectionMode
	{
		/// The connection is insecure. 
		/// You can also just pass 0 for the pointer to PublicKey in ServerApplication::Connect()
		INSECURE_CONNECTION,

		/// Accept whatever public key the server gives us. This is vulnerable to man-in-the 
		///-middle-cheat-attaks, but does not require distribution of the public key in advance of connecting.
		ACCEPT_ANY_PUBLIC_KEY,

		/// Use a known remote server public key. 
		/// PublicKey::remoteServerPublicKey must be non-zero.
		/// This is the recommended mode for secure connections.
		USE_KNOWN_PUBLIC_KEY,

		/// Use a known remote server public key AND provide a public key for the connecting 
		/// client. PublicKey::remoteServerPublicKey, myPublicKey and myPrivateKey must be all
		/// be non-zero. The server must cooperate for this mode to work.  I recommend not 
		/// using this mode except for server-to-server communication as it  significantly 
		/// increases the CPU requirements during connections for both sides. Furthermore, 
		/// when it is used, a connection password should be used as well to avoid DoS attacks.
		USE_TWO_WAY_AUTHENTICATION
	};

	/// Passed to ServerApplication::Connect()
	struct  JACKIE_EXPORT JackieSHSKey
	{
		/// How to interpret the public key, see above
		SecureConnectionMode publicKeyMode;

		/// Pointer to a public key of length cat::EasyHandshake::PUBLIC_KEY_BYTES. 
		/// See the Encryption sample.
		char *remoteServerPublicKey;

		/// (Optional) Pointer to a public key of length cat::EasyHandshake::PUBLIC_KEY_BYTES
		char *myPublicKey;

		/// (Optional) Pointer to a private key of length cat::EasyHandshake::PRIVATE_KEY_BYTES
		char *myPrivateKey;
	};

	/// BindSocket is used to bind socket
	struct JACKIE_EXPORT JackieBindingSocket
	{
		JackieBindingSocket();
		JackieBindingSocket(const char *_hostAddress, UInt16 _port);

		/// The local port to bind to.  Pass 0 to have an OS auto-assigned port.
		UInt16 port;

		/// The local network card address to bind to, such as "127.0.0.1".  
		/// Pass an empty string to use INADDR_ANY.
		char hostAddress[65];

		/// IP version: For IPV4, use AF_INET (default). For IPV6, use AF_INET6. 
		/// To autoselect,  use AF_UNSPEC. IPV6 is the newer internet protocol.
		/// Instead of addresses such as  natpunch.jenkinssoftware.com, 
		/// you may have an address such as fe80::7c:31f7:fec4:27de%14. 
		/// Encoding takes 16 bytes instead of 4, so IPV6 is less efficient for bandwidth.  
		///
		/// On the positive side, NAT Punchthrough is not needed and should not 
		/// be used with IPV6 because there are enough addresses that routers do not need to 
		/// create address mappings.
		/// 
		/// ServerApplication::Startup() will fail if this IP version is not supported.
		/// @Notice NET_SUPPORT_IPV6 must be set to 1 in DefaultDefines.h for AF_INET6
		Int16 socketFamily;

		/// Support PS3
		UInt16 remotePortWasStartedOn_PS3_PSP2;

		/// Required for Google chrome
		_PP_Instance_ chromeInstance;

		/// default is USE_BLOBKING_SOCKET
		bool blockingSocket;

		/// XBOX only: set IPPROTO_VDP if you want to use VDP.
		/// If enabled, this socket does not support broadcast to 255.255.255.255
		unsigned int extraSocketOptions;
	};

	//////////////////////////////////////////////////////////////////////////
	/// Network address for a system Corresponds to a network address
	/// This is not necessarily a unique identifier. For example, if a system has both LAN and
	/// WAN connections, the system may be identified by either one, depending on who is
	/// communicating Therefore, you should not transmit the JACKIE_INET_Address over the 
	/// network and expect it to  identify a system, or use it to connect to that system, except 
	/// in the case where that system is not behind a NAT (such as with a dedciated server)
	/// Use JACKIE_INet_GUID for a unique per-instance of ServerApplication to identify 
	/// systems
	//////////////////////////////////////////////////////////////////////////
	struct JACKIE_EXPORT JackieAddress /// JACKIE_INET_Address
	{
		/// In6 Or In4 
		/// JACKIE_INET_Address, with RAKNET_SUPPORT_IPV6 defined, 
		/// holds both an sockaddr_in6 and a sockaddr_in
		union
		{
#if NET_SUPPORT_IPV6 ==1
			struct sockaddr_storage sa_stor;
			sockaddr_in6 addr6;
#else
			sockaddr_in addr4;
#endif
		} address;

		/// @internal Used internally for fast lookup. 
		/// @optional (use -1 to do regular lookup). Don't transmit this.
		SystemIndex systemIndex;

		/// This is not used internally, but holds a copy of the port held in the address union,
		/// so for debugging it's easier to check what port is being held
		UInt16 debugPort;

		/// Constructors
		JackieAddress();
		JackieAddress(const char *str);
		JackieAddress(const char *str, UInt16 port);

		JackieAddress& operator = (const JackieAddress& input);
		bool operator==(const JackieAddress& right) const;
		bool operator!=(const JackieAddress& right) const;
		bool operator > (const JackieAddress& right) const;
		bool operator < (const JackieAddress& right) const;
		bool EqualsExcludingPort(const JackieAddress& right) const;
		/// @internal Return the size to write to a bitStream
		static Int32 size(void);

		/// Hash the JACKIE_INET_Address
		static unsigned int ToHashCode(const JackieAddress &sa);

		/// Return the IP version, either IPV4 or IPV6
		unsigned char JackieAddress::GetIPVersion(void) const;

		/// @internal Returns either IPPROTO_IP or IPPROTO_IPV6
		unsigned char GetIPProtocol(void) const;

		/// Returns the port in host order (this is what you normally use)
		UInt16 GetPortHostOrder(void) const;

		/// @internal Returns the port in network order
		UInt16 GetPortNetworkOrder(void) const;

		/// Sets the port. The port value should be in host order (this is what you normally use)
		/// Renamed from SetPort because of winspool.h http://edn.embarcadero.com/
		/// article/21494
		void SetPortHostOrder(UInt16 s);

		/// @internal Sets the port. The port value should already be in network order.
		void SetPortNetworkOrder(UInt16 s);

		/// set the port from another JACKIE_INET_Address structure
		void SetPortNetworkOrder(const JackieAddress& right);

		void FixForIPVersion(const JackieAddress &boundAddressToSocket)
		{
			char str[128];
			ToString(false, str);
			// TODO - what about 255.255.255.255?
			if (strcmp(str, IPV6_LOOPBACK) == 0)
			{
				if (boundAddressToSocket.GetIPVersion() == 4)
				{
					FromString(IPV4_LOOPBACK, (Int8)0, (UInt8)4);
				}
			}
			else if (strcmp(str, IPV4_LOOPBACK) == 0)
			{
#if NET_SUPPORT_IPV6==1
				if (boundAddressToSocket.GetIPVersion() == 6)
				{
					FromString(IPV6_LOOPBACK, (Int8)0,  (UInt8)6);
				}
#endif
			}
		}

		/// Call SetToLoopback(), with whatever IP version is currently held. Defaults to IPV4
		void SetToLoopBack(void);

		/// Call SetToLoopback() with a specific IP version
		/// @param[in ipVersion] Either 4 for IPV4 or 6 for IPV6
		void SetToLoopBack(unsigned char ipVersion);

		/// \return If was set to 127.0.0.1 or ::1
		bool IsLoopback(void) const;
		bool IsLANAddress(void);

		/// Old version, for crap platforms that don't support newer socket functions
		bool SetIP4Address(const char *str, char portDelineator = ':');

		/// Set the system address from a printable IP string, for example "192.0.2.1" or 
		/// "2001:db8:63b3:1::3490"  You can write the port as well, using the portDelineator, for 
		/// example "192.0.2.1|1234"
		//
		/// @param[in, str] A printable IP string, for example "192.0.2.1" or "2001:db8:63b3:1::3490". 
		/// Pass 0 for \a str to set to JACKIE_INET_Address_Null
		//
		/// @param[in, portDelineator] if \a str contains a port, delineate the port with this 
		/// character. portDelineator should not be '.', ':', '%', '-', '/', a number, or a-f
		// 
		/// @param[in, ipVersion] Only used if str is a pre-defined address in the wrong format, 
		/// such  as 127.0.0.1 but you want ip version 6, so you can pass 6 here to do the conversion
		//
		/// @note The current port is unchanged if a port is not specified in \a str
		/// @return True on success, false on ipVersion does not match type of passed string
		bool FromString(const char *str, char portDelineator = '|', unsigned char ipVersion = 0);

		/// Same as FromString(), but you explicitly set a port at the same time
		bool FromString(const char *str, UInt16 port, unsigned char ipVersion = 0);

		// Return the systemAddress as a string in the format <IP>|<Port>
		// Returns a static string
		// NOT THREADSAFE
		// portDelineator should not be [.] [:] [%] [-] [/] [number] [a-z]
		const char *ToString(bool writePort = true, char portDelineator = '|') const;

		// Return the systemAddress as a string in the format <IP>|<Port>
		// dest must be large enough to hold the output
		// portDelineator should not be [.] [:] [%] [-] [/] [number] [a-z]
		// THREADSAFE
		void ToString(bool writePort, char *dest, char portDelineator = '|') const;

		/// @internal 
		void ToString_IPV4(bool writePort, char *dest, char portDelineator = ':') const;
		void ToString_IPV6(bool writePort, char *dest, char portDelineator) const;
	};

	//////////////////////////////////////////////////////////////////////////
	/// Uniquely identifies an instance of ServerApplication. 
	/// Use ServerApplication::GetGuidFromSystemAddress() 
	/// and ServerApplication::GetSystemAddressFromGuid() to go between 
	/// JACKIE_INET_Address and  JACKIE_INet_GUID. 
	/// Use ServerApplication::GetGuidFromSystemAddress
	/// (JACKIE_INET_Address_Null) to get your own GUID
	//////////////////////////////////////////////////////////////////////////
	struct JACKIE_EXPORT JackieGUID
	{
		/// Used internally for fast lookup. Optional (use -1 to do regular lookup).
		/// Don't transmit this.
		SystemIndex systemIndex;
		UInt64 g;

		JackieGUID();
		explicit JackieGUID(UInt64 _g);
		JackieGUID& operator = (const JackieGUID& input);


		/// Return the GUID as a static string. 
		/// NOT THREADSAFE
		const char *ToString(void) const;

		/// Return the GUID as a string
		/// dest must be large enough to hold the output
		/// THREADSAFE
		void ToString(char *dest) const;

		bool FromString(const char *source);

		static unsigned long ToUInt32(const JackieGUID &g);
		static int size();

		bool operator==(const JackieGUID& right) const;
		bool operator!=(const JackieGUID& right) const;
		bool operator > (const JackieGUID& right) const;
		bool operator < (const JackieGUID& right) const;
	};

	struct JACKIE_EXPORT JackieAddressGuidWrapper
	{
		JackieGUID guid;
		JackieAddress systemAddress;

		SystemIndex GetSystemIndex(void) const
		{
			if (guid != JACKIE_NULL_GUID)
				return guid.systemIndex;
			else
				return systemAddress.systemIndex;
		}

		bool IsUndefined(void) const
		{
			return guid == JACKIE_NULL_GUID &&
				systemAddress == JACKIE_NULL_ADDRESS;
		}

		void SetUndefined(void)
		{
			guid = JACKIE_NULL_GUID;
			systemAddress = JACKIE_NULL_ADDRESS;
		}

		/// Firstly try to return the hashcode of @guid if guid != null
		/// otherwise it will return hashcode of @systemAddress
		static unsigned long ToHashCode(const JackieAddressGuidWrapper &aog);

		/// Firstly try to return the hashcode of @guid if guid != null
		/// otherwise it will return the string of @systemAddress
		const char *ToString(bool writePort = true) const;
		void ToString(bool writePort, char *dest) const;

		JackieAddressGuidWrapper() { }
		JackieAddressGuidWrapper(const JackieAddressGuidWrapper& input)
		{
			guid = input.guid;
			systemAddress = input.systemAddress;
		}
		JackieAddressGuidWrapper(const JackieAddress& input)
		{
			guid = JACKIE_NULL_GUID;
			systemAddress = input;
		}
		JackieAddressGuidWrapper(const JackiePacket& packet);
		JackieAddressGuidWrapper(const JackieGUID& input)
		{
			guid = input;
			systemAddress = JACKIE_NULL_ADDRESS;
		}
		JackieAddressGuidWrapper& operator = (const JackieAddressGuidWrapper& input)
		{
			guid = input.guid;
			systemAddress = input.systemAddress;
			return *this;
		}
		JackieAddressGuidWrapper& operator = (const JackieAddress& input)
		{
			guid = JACKIE_NULL_GUID;
			systemAddress = input;
			return *this;
		}
		JackieAddressGuidWrapper& operator = (const JackieGUID& input)
		{
			guid = input;
			systemAddress = JACKIE_NULL_ADDRESS;
			return *this;
		}
		bool operator==(const JackieAddressGuidWrapper& right) const
		{
			return (guid != JACKIE_NULL_GUID && guid == right.guid) ||
				(systemAddress != JACKIE_NULL_ADDRESS &&
				systemAddress == right.systemAddress);
		}
	};

	struct JACKIE_EXPORT UInt24
	{
		unsigned int val;

		UInt24() { }
		operator unsigned int() { return val; }
		operator unsigned int() const { return val; }

		UInt24(const UInt24& a) { val = a.val; }
		UInt24 operator++() { ++val; val &= 0x00FFFFFF; return *this; }
		UInt24 operator--() { --val; val &= 0x00FFFFFF; return *this; }
		UInt24 operator++(int) { UInt24 temp(val); ++val; val &= 0x00FFFFFF; return temp; }
		UInt24 operator--(int) { UInt24 temp(val); --val; val &= 0x00FFFFFF; return temp; }
		UInt24 operator&(const UInt24& a) { return UInt24(val&a.val); }
		UInt24& operator=(const UInt24& a) { val = a.val; return *this; }
		UInt24& operator+=(const UInt24& a) { val += a.val; val &= 0x00FFFFFF; return *this; }
		UInt24& operator-=(const UInt24& a) { val -= a.val; val &= 0x00FFFFFF; return *this; }
		bool operator==(const UInt24& right) const { return val == right.val; }
		bool operator!=(const UInt24& right) const { return val != right.val; }
		bool operator > (const UInt24& right) const { return val > right.val; }
		bool operator < (const UInt24& right) const { return val < right.val; }
		const UInt24 operator+(const UInt24 &other) const { return UInt24(val + other.val); }
		const UInt24 operator-(const UInt24 &other) const { return UInt24(val - other.val); }
		const UInt24 operator/(const UInt24 &other) const { return UInt24(val / other.val); }
		const UInt24 operator*(const UInt24 &other) const { return UInt24(val*other.val); }

		UInt24(const unsigned int& a) { val = a; val &= 0x00FFFFFF; }
		UInt24 operator&(const unsigned int& a) { return UInt24(val&a); }
		UInt24& operator=(const unsigned int& a) { val = a; val &= 0x00FFFFFF; return *this; }
		UInt24& operator+=(const unsigned int& a) { val += a; val &= 0x00FFFFFF; return *this; }
		UInt24& operator-=(const unsigned int& a) { val -= a; val &= 0x00FFFFFF; return *this; }
		bool operator==(const unsigned int& right) const { return val == (right & 0x00FFFFFF); }
		bool operator!=(const unsigned int& right) const { return val != (right & 0x00FFFFFF); }
		bool operator > (const unsigned int& right) const { return val > (right & 0x00FFFFFF); }
		bool operator < (const unsigned int& right) const { return val < (right & 0x00FFFFFF); }
		const UInt24 operator+(const unsigned int &other) const { return UInt24(val + other); }
		const UInt24 operator-(const unsigned int &other) const { return UInt24(val - other); }
		const UInt24 operator/(const unsigned int &other) const { return UInt24(val / other); }
		const UInt24 operator*(const unsigned int &other) const { return UInt24(val*other); }
	};


	typedef UInt16 SplitPacketId;
	typedef unsigned int SplitPacketIndex;

	//////////////////////////////////////////////////////////////////////////
	/// This is the counter used for holding packet numbers,
	/// so we can detect duplicate packets.  
	/// Internally assumed to be 4 bytes, but written as 3 bytes in 
	/// ReliabilityLayer::WriteToBitStreamFromInternalPacket
	/// typedef uint24_t MessageNumberType;
	//////////////////////////////////////////////////////////////////////////
	typedef UInt24 PacketIndex;

	//////////////////////////////////////////////////////////////////////////
	/// This is the counter used for holding ordered packet numbers, 
	/// so we can detect out-of-order  packets. 
	/// It should be large enough  that if the variables were to wrap,
	/// the newly wrapped values would no longer be in use.  
	/// Warning: Too large of a value wastes bandwidth!
	/// typedef MessageNumberType OrderingIndexType;
	//////////////////////////////////////////////////////////////////////////
	typedef UInt24 OrderPacketIndex;

	typedef TimeUS RemoteEndPointTimeType;

	/// These enumerations are used to describe when packets are delivered.
	enum PacketSendPriority : unsigned char
	{
		/// The highest possible priority. These message trigger sends immediately, 
		/// and are generally not buffered or aggregated into a single datagram.
		UNBUFFERED_IMMEDIATELY_SEND,

		/// For every 2 IMMEDIATE_PRIORITY messages, 1 HIGH_PRIORITY will be sent.
		/// Messages at this priority and lower are buffered to be sent in groups at 10 
		/// millisecond intervals to reduce UDP overhead and better measure congestion 
		/// control. 
		BUFFERED_FIRSTLY_SEND,

		/// For every 2 HIGH_PRIORITY messages, 1 MEDIUM_PRIORITY will be sent.
		/// Messages at this priority and lower are buffered to be sent in groups at 10 
		/// millisecond intervals to reduce UDP overhead and better measure congestion 
		/// control. 
		BUFFERED_SECONDLY_SEND,

		/// For every 2 MEDIUM_PRIORITY messages, 1 LOW_PRIORITY will be sent.
		/// Messages at this priority and lower are buffered to be sent in groups at 10
		/// millisecond intervals to reduce UDP overhead and better measure congestion 
		/// control. 
		BUFFERED_THIRDLY_SEND,

		/// \internal
		PRIORITIES_COUNT
	};

	/////////////////////////////////////////////////////////////////////////////////////
	/// These enumerations are used to describe how packets are delivered.
	/// \note  Note to self: I write this with 3 bits in the stream.  If I add more 
	/// remember to change that
	/// \note In ReliabilityLayer::WriteToBitStreamFromInternalPacket I assume 
	/// there are 5 major types
	/// \note Do not reorder, I check on >= UNRELIABLE_WITH_ACK_RECEIPT which equals 5
	//////////////////////////////////////////////////////////////////////////////////////
	enum PacketReliability :unsigned char
	{
		//////////////////////////////////////////////////////////////////////////
		/// Same as regular UDP, except that it will also discard duplicate datagrams.  
		/// JackieINet adds (6 to 17) + 21 bits of overhead, 16 of which is used to detect 
		/// duplicate msg and 6 to 17 of which is used for message length.
		//////////////////////////////////////////////////////////////////////////
		UNRELIABLE_NOT_ACK_RECEIPT_OF_PACKET,

		//////////////////////////////////////////////////////////////////////////
		/// Regular UDP with a sequence counter. Out of order messages will be discarded.
		/// Sequenced and ordered messages sent on the same channel will arrive in the 
		/// order sent.
		//////////////////////////////////////////////////////////////////////////
		UNRELIABLE_SEQUENCED_NOT_ACK_RECEIPT_OF_PACKET,

		//////////////////////////////////////////////////////////////////////////
		/// The message is sent reliably, but not necessarily in any order.
		/// reliable here means no dropping msgs, lost msgs will be resent
		//////////////////////////////////////////////////////////////////////////
		RELIABLE_NOT_ACK_RECEIPT_OF_PACKET,

		//////////////////////////////////////////////////////////////////////////
		/// This message is reliable and will arrive in the order you sent it. 
		/// Messages will be delayed while waiting for out of order messages. 
		/// Same overhead as UNRELIABLE_SEQUENCED. Sequenced and ordered 
		/// messages sent on the same channel will arrive in the order sent.
		//////////////////////////////////////////////////////////////////////////
		RELIABLE_ORDERED_NOT_ACK_RECEIPT_OF_PACKET,

		//////////////////////////////////////////////////////////////////////////
		/// This message is reliable and will arrive in the sequence you sent it. 
		/// Out of order messages will be dropped.  Same overhead as 
		/// UNRELIABLE_SEQUENCED. Sequenced and ordered messages 
		/// sent on the same channel will arrive in the order sent.
		//////////////////////////////////////////////////////////////////////////
		RELIABLE_SEQUENCED_NOT_ACK_RECEIPT_OF_PACKET,

		//////////////////////////////////////////////////////////////////////////
		/// however the user will get either ID_SND_RECEIPT_ACKED or ID_SND_RECEIPT_LOSS
		/// based on the result of sending this message when calling ServerApplication::Receive
		/// (). Bytes 1-4 will contain the number returned from the Send() function. On 
		/// disconnect or shutdown, all messages not previously acked should be considered 
		/// lost.
		//////////////////////////////////////////////////////////////////////////
		UNRELIABLE_ACK_RECEIPT_OF_PACKET,

		//////////////////////////////////////////////////////////////////////////
		///  [10/18/2015 mengdi] You can't have sequenced and ack receipts, because you 
		/// don't know if the other system discarded the message, meaning you don't know
		/// if the message was processed
		UNRELIABLE_SEQUENCED_WITH_ACK_RECEIPT,
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		/// The user will also get ID_SND_RECEIPT_ACKED after the message is delivered when 
		/// calling ServerApplication::Receive(). ID_SND_RECEIPT_ACKED is returned when the 
		/// message arrives, not necessarily the order when it was sent. Bytes 1-4 will contain 
		/// the number returned from the Send() function. On disconnect or shutdown, all 
		/// messages not previously acked should be considered lost. This does not return 
		/// ID_SND_RECEIPT_LOSS.
		//////////////////////////////////////////////////////////////////////////
		RELIABLE_ACK_RECEIPT_OF_PACKET,

		//////////////////////////////////////////////////////////////////////////
		/// The user will also get ID_SND_RECEIPT_ACKED after the message is delivered when 
		/// calling ServerApplication::Receive(). ID_SND_RECEIPT_ACKED is returned when the 
		/// message arrives, not necessarily the order when it was sent. Bytes 1-4 will contain 
		/// the number returned from the Send() function. On disconnect or shutdown, all 
		/// messages not previously acked should be considered lost. This does not return 
		/// ID_SND_RECEIPT_LOSS.
		//////////////////////////////////////////////////////////////////////////
		RELIABLE_ORDERED_ACK_RECEIPT_OF_PACKET,

		//////////////////////////////////////////////////////////////////////////
		/// 05/04/10 You can't have sequenced and ack receipts, because you don't know if the 
		/// other system discarded the message, meaning you don't know if the message was 
		/// processed
		RELIABLE_SEQUENCED_WITH_ACK_RECEIPT,
		//////////////////////////////////////////////////////////////////////////

		/// internal
		NUMBER_OF_RELIABILITIES
	};

	/// Not endian safe
	/// InternalPacketFixedSizeTransmissionHeader
	struct PacketHeader
	{
		/// A unique numerical identifier given to this user message.
		/// Used to identify reliable messages on the network
		PacketIndex packetIndex;
		/// The ID used as identification for ordering messages. 
		/// Also included in sequenced messages
		OrderPacketIndex orderingIndex;
		/// Used only with sequenced messages
		OrderPacketIndex sequencingIndex;
		/// What ordering channel this packet is on, 
		/// if the reliability type uses ordering channels
		unsigned char orderingChannel;
		/// The ID of the split packet, if we have split packets.  
		/// This is the maximum number of split messages 
		/// we can send simultaneously per connection.
		SplitPacketId splitPacketId;
		/// If this is a split packet, the index into the array of subsplit packets
		SplitPacketIndex splitPacketIndex;
		/// The size of the array of subsplit packets
		SplitPacketIndex splitPacketCount;
		/// How many bits long the data is
		unsigned int dataBitLength;
		/// What type of reliability algorithm to use with this packet
		PacketReliability reliability;
	};


	/// Holds a user message, and related information
	/// Don't use a constructor or destructor, due to the memory pool I am using
	struct InternalPacket : public PacketHeader
	{
		/// Identifies the order in which this number was sent. Used locally
		PacketIndex messageInternalOrder;
		/// Has this message number been assigned yet?  We don't assign until the message is actually sent.
		/// This fixes a bug where pre-determining message numbers and then sending a message on a different channel creates a huge gap.
		/// This causes performance problems and causes those messages to timeout.
		bool messageNumberAssigned;
		/// Was this packet number used this update to track windowing drops or increases?  Each packet number is only used once per update.
		//	bool allowWindowUpdate;
		///When this packet was created
		TimeUS creationTime;
		///The resendNext time to take action on this packet
		TimeUS nextActionTime;
		// For debugging
		TimeUS retransmissionTime;
		// Size of the header when encoded into a bitstream
		unsigned int headerLength;
		/// Buffer is a pointer to the actual data, assuming this packet has data at all
		unsigned char *data;
		/// How to alloc and delete the data member
		enum
		{
			/// Data is allocated using rakMalloc. Just free it
			NORMAL,
			/// data points to a larger block of data, 
			/// where the larger block is reference counted. RefCountedData is used in this case
			REFCOUNTDATA,
			/// If allocation scheme is STACK, 
			/// data points to stackData and should not be deallocated
			/// This is only used when sending. Received packets are deallocated in JACKIE_INET_
			STACK
		} allocationScheme;
		/// Used in InternalPacket when pointing to refCountedData, 
		/// rather than allocating itself
		struct RefCountData
		{
			char *refCountedData;
			unsigned int refCount;
		} *refCountedData;
		/// How many attempts we made at sending this message
		unsigned char timesTrytoSend;
		/// The priority level of this packe

		PacketSendPriority priority;
		/// If the reliability type requires a receipt, then return this number with it
		unsigned int sendReceiptSerial;
		// Used for the resend queue
		// Linked list implementation so I can remove from the list via a pointer, 
		/// without finding it in the list
		InternalPacket *resendPrev, *resendNext, *unreliablePrev, *unreliableNext;
		unsigned char stackData[128];
	};

	/// This is the smallest unit that represents a meaningful user-created logic data
	struct JACKIE_EXPORT JackiePacket
	{
		/// The system that send this packet.
		JackieAddress systemAddress;

		/// A unique identifier for the system that sent this packet, 
		/// regardless of IP address (internal / external / remote system)
		/// Only valid once a connection has been established 
		/// (ID_CONNECTION_REQUEST_ACCEPTED, or ID_NEW_INCOMING_CONNECTION)
		/// Until that time, will be JACKIE_INet_GUID_Null
		JackieGUID guid;

		/// The length of the data in bytes
		unsigned int length;

		/// The length of the data in bits
		unsigned int bitSize;

		/// The data from the sender
		unsigned char *data;

		/// @internal
		/// Indicates whether to delete the data, or to simply delete the packet.
		/// if true, this packet is allocated by pool if false, by rakAllloc_Ex function
		bool freeInternalData;

		/// @internal  If true, this message is meant for the user, not for the plugins,
		/// so do not process it through plugins
		bool wasGeneratedLocally;

		///which pool it belongs to only 
		//ThreadType threadType;
	};

	/// for internally use
	/// All the information representing a connected remote end point
	struct JACKIE_EXPORT JackieRemoteSystem
	{
		// Is this structure in use?
		bool isActive;
		/// Their external IP on the internet
		JackieAddress systemAddress;
		/// Your external IP on the internet, from their perspective
		JackieAddress myExternalSystemAddress;
		/// Their internal IP, behind the LAN
		JackieAddress theirInternalSystemAddress[MAX_COUNT_LOCAL_IP_ADDR];
		/// The reliability layer associated with this player
		JackieReliabler reliabilityLayer;
		/// True if we started this connection via Connect.  
		/// False if someone else connected to us.
		bool weInitiateConnection;
		/// last x ping times and calculated clock differentials with it
		struct PingAndClockDifferential
		{
			unsigned short pingTime;
			Time clockDifferential;
		} pingAndClockDifferential[PING_TIMES_ARRAY_SIZE];
		/// The index we are writing into the pingAndClockDifferential circular buffer
		Time pingAndClockDifferentialWriteIndex;
		unsigned short lowestPing; ///The lowest ping value encountered
		Time nextPingTime;  /// When to next ping this player
		/// When did the last reliable send occur.  
		/// Reliable sends must occur at least once every 
		/// timeoutTime/2 units to notice disconnects
		Time lastReliableSend;
		/// connection time, if active.
		Time connectionTime;
		JackieGUID guid;
		int MTUSize;
		// Reference counted socket to send back on
		JackieINetSocket* socket2use;
		SystemIndex remoteSystemIndex;

#if ENABLE_SECURE_HAND_SHAKE==1
		// Cached answer used internally by JackieNet
		/// to prevent DoS attacks based on the connetion handshake
		char answer[cat::EasyHandshake::ANSWER_BYTES];
		// If the server has bRequireClientKey = true,
		/// then this is set to the validated public key of the connected client
		/// Valid after connectMode reaches HANDLING_CONNECTION_REQUEST
		char client_public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
#endif
		enum ConnectMode : unsigned char
		{
			NO_ACTION,
			DISCONNECT_ASAP,
			DISCONNECT_ASAP_SILENTLY,
			DISCONNECT_ON_NO_ACK,
			REQUESTED_CONNECTION,
			HANDLING_CONNECTION_REQUEST,
			UNVERIFIED_SENDER,
			CONNECTED
		} connectMode;
	};

	struct JACKIE_EXPORT JackieRemoteIndex
	{
		unsigned int index;
		JackieRemoteIndex *next;
	};

	struct JACKIE_EXPORT Command
	{
		enum : unsigned char
		{
			BCS_SEND,
			BCS_CLOSE_CONNECTION,
			BCS_GET_SOCKET,
			BCS_CHANGE_SYSTEM_ADDRESS,
			/* BCS_USE_USER_SOCKET, BCS_REBIND_SOCKET_ADDRESS, BCS_RPC, BCS_RPC_SHIFT,*/
			BCS_ADD_2_BANNED_LIST,
			BCS_CONEECT,
			BCS_DO_NOTHING,
		} commandID;

		JackieAddressGuidWrapper systemIdentifier;
		char *data;
		union
		{
			struct
			{
				NetworkID networkID;
				PacketReliability priority;
				PacketReliability reliability;
				JackieRemoteSystem::ConnectMode repStatus;
				bool blockingCommand; // Only used for RPC
				bool haveRakNetCloseSocket;
				bool broadcast;
				char orderingChannel;
				unsigned int connectionSocketIndex;
				unsigned int extraSocketOptions;
				unsigned int receipt;
				unsigned int numberOfBitsToSend;
				JackieINetSocket* socket;
				unsigned short port;
				unsigned short remotePortRakNetWasStartedOn_PS3;
			};
			//sizeof(unsigned int) * 4 + sizeof(short) * 2 + sizeof(JackieINetSocket*) + sizeof(char) * 7 + sizeof(NetworkID) = 35 bytes
			char arrayparams[sizeof(unsigned int) * 4 + sizeof(short) * 2 + sizeof(JackieINetSocket*) + sizeof(char) * 7 + sizeof(NetworkID)];
		};
	};

	struct JACKIE_EXPORT ConnectionRequest
	{
		JackieAddress receiverAddr;
		Time nextRequestTime;
		unsigned char requestsMade;
		char *data;
		unsigned short dataLength;
		char pwd[256];
		unsigned char pwdLen;
		unsigned int socketIndex;
		unsigned int extraData;
		unsigned int connAttemptTimes;
		unsigned int connAttemptIntervalMS;
		TimeMS timeout;
		SecureConnectionMode publicKeyMode;
		JackieINetSocket* socket;
		enum ActionToTake : unsigned char
		{
			CONNECT = 1,
			PING = 2,
			PING_OPEN_CONNECTIONS = 4,
			ADVERTISE_SYSTEM = 2
		} actionToTake;
#if ENABLE_SECURE_HAND_SHAKE ==1
		char handshakeChallenge[cat::EasyHandshake::CHALLENGE_BYTES];
		cat::ClientEasyHandshake *client_handshake;
		char remote_public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
#endif
	};

	/// Every platform except windows store 8 and native client supports Berkley sockets
#if !defined(WINDOWS_STORE_RT)
	JACKIE_EXPORT extern void DomainNameToIP_Berkley_IPV4And6(const char *domainName, char ip[65]);
	JACKIE_EXPORT extern void DomainNameToIP_Berkley_IPV4(const char *domainName, char ip[65]);
#else
	JACKIE_EXPORT extern void DomainNameToIP_Non_Berkley(const char *domainName, char ip[65]);
#endif ///  !defined(WINDOWS_STORE_RT)
	inline JACKIE_EXPORT extern void  DomainNameToIP(const char *domainName, char ip[65])
	{
#if defined(WINDOWS_STORE_RT)
		return DomainNameToIP_Non_Berkley(domainName, ip);
#elif defined(__native_client__)
		return DomainNameToIP_Berkley_IPV4And6(domainName, ip);
#elif defined(_WIN32)
		return DomainNameToIP_Berkley_IPV4And6(domainName, ip);
#else
		return DomainNameToIP_Berkley_IPV4And6(domainName, ip);
#endif
}

	/// Return false if Numeric IP address. Return true if domain NonNumericHostString
	JACKIE_EXPORT extern bool  isDomainIPAddr(const char *host);
}
#endif
