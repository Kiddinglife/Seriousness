/*
*  RakNetTypes.h, INetApplication = RakPeerInterface
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*/
#ifndef NET_TYPES_H_
#define NET_TYPES_H_

#include "DLLExport.h"
#include"DefaultNetDefines.h"
#include "BasicTypes.h"
#include "WindowsIncludes.h"
#include "SockOSIncludes.h"
#include "SocketDefines.h"

namespace JACKIE_INET
{
	/// Forward declarations
	class   INetApplication;
	class   JackieStream;
	struct  Packet;
	struct  JackieAddress;
	struct  JackieGUID;

	/// @Internal Defines the default maximum transfer unit.
	/// \li \em 17914 16 Mbit/Sec Token Ring
	/// \li \em 4464 4 Mbits/Sec Token Ring
	/// \li \em 4352 FDDI
	/// \li \em 1500. The largest Ethernet packet size \b recommended. This is the typical setting for non-PPPoE, non-VPN connections. The default value for NETGEAR routers, adapters and switches.
	/// \li \em 1492. The size PPPoE prefers.
	/// \li \em 1472. Maximum size to use for pinging. (Bigger packets are fragmented.)
	/// \li \em 1468. The size DHCP prefers.
	/// \li \em 1460. Usable by AOL if you don't have large email attachments, etc.
	/// \li \em 1430. The size VPN and PPTP prefer.
	/// \li \em 1400. Maximum size for AOL DSL.
	/// \li \em 576. Typical value to connect to dial-up ISPs.
	/// The largest value for an UDP datagram
#define MAXIMUM_MTU_SIZE 1492
#define MINIMUM_MTU_SIZE 400


	/// Given a number of bits, return how many bytes are needed to represent that.
#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

#if defined(_MSC_VER) && _MSC_VER > 0
#define PRINTF_64BITS_MODIFIER "I64"
#else
#define PRINTF_64BITS_MODIFIER "ll"
#endif

	/// \sa NetworkIDObject.h
	typedef unsigned char   UniqueIDType;
	typedef unsigned char   RPCIndex;
	typedef UInt16 SystemIndex;
	typedef UInt64 NetworkID;

	/// First byte of a network message
	typedef unsigned char MessageID;

	/// Index of an invalid JACKIE_INET_Address
#ifndef SWIG
	JACKIE_EXPORT extern const  JackieAddress JACKIE_INET_Address_Null;
	JACKIE_EXPORT extern const  JackieGUID JACKIE_INet_GUID_Null;
#endif

	const SystemIndex UNASSIGNED_PLAYER_INDEX = 65535; ///  Index of an unassigned player
	const NetworkID UNASSIGNED_NETWORK_ID = (UInt64) -1; /// Unassigned object ID
	const int MAX_RPC_MAP_SIZE = ( (RPCIndex) -1 ) - 1; // 254
	const int UNDEFINED_RPC_INDEX = ( (RPCIndex) -1 ); // 255
	const int PING_TIMES_ARRAY_SIZE = 5;

	/// enums
	enum StartupResult
	{
		RAKNET_STARTED,
		RAKNET_ALREADY_STARTED,
		INVALID_SOCKET_DESCRIPTORS,
		INVALID_MAX_CONNECTIONS,
		SOCKET_FAMILY_NOT_SUPPORTED,
		SOCKET_PORT_ALREADY_IN_USE,
		SOCKET_FAILED_TO_BIND,
		SOCKET_FAILED_TEST_SEND,
		PORT_CANNOT_BE_ZERO,
		FAILED_TO_CREATE_NETWORK_THREAD,
		COULD_NOT_GENERATE_GUID,
		STARTUP_OTHER_FAILURE
	};

	enum ConnectionAttemptResult
	{
		CONNECTION_ATTEMPT_STARTED,
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
	enum PublicKeyMode
	{
		/// The connection is insecure. 
		/// You can also just pass 0 for the pointer to PublicKey in RakPeerInterface::Connect()
		INSECURE_CONNECTION,

		/// Accept whatever public key the server gives us. This is vulnerable to man in the 
		/// middle, but does not require distribution of the public key in advance of connecting.
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


	/// Passed to RakPeerInterface::Connect()
	struct  JACKIE_EXPORT JACKIE_Public_Key
	{
		/// How to interpret the public key, see above
		PublicKeyMode publicKeyMode;

		/// Pointer to a public key of length cat::EasyHandshake::PUBLIC_KEY_BYTES. 
		/// See the Encryption sample.
		char *remoteServerPublicKey;

		/// (Optional) Pointer to a public key of length cat::EasyHandshake::PUBLIC_KEY_BYTES
		char *myPublicKey;

		/// (Optional) Pointer to a private key of length cat::EasyHandshake::PRIVATE_KEY_BYTES
		char *myPrivateKey;
	};


	/// SocketDescriptor
	/// Describes the local socket to use for RakPeer::Startup
	struct JACKIE_EXPORT BindSocket
	{
		BindSocket();
		BindSocket(const char *_hostAddress, UInt16 _port);
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
		/// RakPeer::Startup() will fail if this IP version is not supported.
		/// @Notice NET_SUPPORT_IPV6 must be set to 1 in DefaultDefines.h for AF_INET6
		Int16 socketFamily;

		/// Support PS3
		UInt16 remotePortWasStartedOn_PS3_PSP2;

		/// Required for Google chrome
		_PP_Instance_ chromeInstance;

		/// Set to true to use a blocking socket (default, do not change unless you have a reason to)
		bool blockingSocket;

		/// XBOX only: set IPPROTO_VDP if you want to use VDP.
		/// If enabled, this socket does not support broadcast to 255.255.255.255
		unsigned int extraSocketOptions;
	};


	/// @Brief Network address for a system Corresponds to a network address
	//
	/// This is not necessarily a unique identifier. For example, if a system has both LAN and 
	/// WAN connections, the system may be identified by either one, depending on who is
	/// communicating
	// 
	/// Therefore, you should not transmit the JACKIE_INET_Address over the network and expect it to 
	/// identify a system, or use it to connect to that system, except in the case where that system
	/// is not behind a NAT (such as with a dedciated server)
	// 
	/// Use JACKIE_INet_GUID for a unique per-instance of JackieApplication(RakPeer) to identify 
	/// systems
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

		JackieAddress& operator = ( const JackieAddress& input );
		bool operator==( const JackieAddress& right ) const;
		bool operator!=( const JackieAddress& right ) const;
		bool operator > ( const JackieAddress& right ) const;
		bool operator < ( const JackieAddress& right ) const;

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


	/// Uniquely identifies an instance of RakPeer. Use RakPeer::GetGuidFromSystemAddress() 
	/// and RakPeer::GetSystemAddressFromGuid() to go between JACKIE_INET_Address and 
	/// JACKIE_INet_GUID Use RakPeer::GetGuidFromSystemAddress
	/// (JACKIE_INET_Address_Null) to get your own GUID
	struct JACKIE_EXPORT JackieGUID
	{
		/// Used internally for fast lookup. Optional (use -1 to do regular lookup).
		/// Don't transmit this.
		SystemIndex systemIndex;
		UInt64 g;

		JackieGUID();
		explicit JackieGUID(UInt64 _g);
		JackieGUID& operator = ( const JackieGUID& input );


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

		bool operator==( const JackieGUID& right ) const;
		bool operator!=( const JackieGUID& right ) const;
		bool operator > ( const JackieGUID& right ) const;
		bool operator < ( const JackieGUID& right ) const;
	};


	struct JACKIE_EXPORT JACKIE_INET_Address_GUID_Wrapper
	{
		JackieGUID guid;
		JackieAddress systemAddress;

		SystemIndex GetSystemIndex(void) const
		{
			if( guid != JACKIE_INet_GUID_Null )
				return guid.systemIndex; else return systemAddress.systemIndex;
		}

		bool IsUndefined(void) const
		{
			return guid == JACKIE_INet_GUID_Null &&
				systemAddress == JACKIE_INET_Address_Null;
		}

		void SetUndefined(void)
		{
			guid = JACKIE_INet_GUID_Null;
			systemAddress = JACKIE_INET_Address_Null;
		}

		/// Firstly try to return the hashcode of @guid if guid != null
		/// otherwise it will return hashcode of @systemAddress
		static unsigned long ToHashCode(const JACKIE_INET_Address_GUID_Wrapper &aog);

		/// Firstly try to return the hashcode of @guid if guid != null
		/// otherwise it will return the string of @systemAddress
		const char *ToString(bool writePort = true) const;
		void ToString(bool writePort, char *dest) const;

		JACKIE_INET_Address_GUID_Wrapper() { }
		JACKIE_INET_Address_GUID_Wrapper(const JACKIE_INET_Address_GUID_Wrapper& input)
		{
			guid = input.guid;
			systemAddress = input.systemAddress;
		}
		JACKIE_INET_Address_GUID_Wrapper(const JackieAddress& input)
		{
			guid = JACKIE_INet_GUID_Null;
			systemAddress = input;
		}
		JACKIE_INET_Address_GUID_Wrapper(const Packet& packet);
		JACKIE_INET_Address_GUID_Wrapper(const JackieGUID& input)
		{
			guid = input;
			systemAddress = JACKIE_INET_Address_Null;
		}
		JACKIE_INET_Address_GUID_Wrapper& operator = ( const JACKIE_INET_Address_GUID_Wrapper& input )
		{
			guid = input.guid;
			systemAddress = input.systemAddress;
			return *this;
		}
		JACKIE_INET_Address_GUID_Wrapper& operator = ( const JackieAddress& input )
		{
			guid = JACKIE_INet_GUID_Null;
			systemAddress = input;
			return *this;
		}
		JACKIE_INET_Address_GUID_Wrapper& operator = ( const JackieGUID& input )
		{
			guid = input;
			systemAddress = JACKIE_INET_Address_Null;
			return *this;
		}
		bool operator==( const JACKIE_INET_Address_GUID_Wrapper& right ) const
		{
			return ( guid != JACKIE_INet_GUID_Null && guid == right.guid ) ||
				( systemAddress != JACKIE_INET_Address_Null &&
				systemAddress == right.systemAddress );
		}
	};

	/// This represents a user message from another system.
	struct JACKIE_EXPORT Packet
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
		char *data;

		/// @internal
		/// Indicates whether to delete the data, or to simply delete the packet.
		bool isAllocatedFromPool;

		/// @internal  If true, this message is meant for the user, not for the plugins,
		/// so do not process it through plugins
		bool wasGeneratedLocally;
	};

	struct JACKIE_EXPORT UInt24
	{
		unsigned int val;

		UInt24() { }
		operator unsigned int() { return val; }
		operator unsigned int() const { return val; }

		UInt24(const UInt24& a) { val = a.val; }
		UInt24 operator++( ) { ++val; val &= 0x00FFFFFF; return *this; }
		UInt24 operator--( ) { --val; val &= 0x00FFFFFF; return *this; }
		UInt24 operator++( int ) { UInt24 temp(val); ++val; val &= 0x00FFFFFF; return temp; }
		UInt24 operator--( int ) { UInt24 temp(val); --val; val &= 0x00FFFFFF; return temp; }
		UInt24 operator&( const UInt24& a ) { return UInt24(val&a.val); }
		UInt24& operator=( const UInt24& a ) { val = a.val; return *this; }
		UInt24& operator+=( const UInt24& a ) { val += a.val; val &= 0x00FFFFFF; return *this; }
		UInt24& operator-=( const UInt24& a ) { val -= a.val; val &= 0x00FFFFFF; return *this; }
		bool operator==( const UInt24& right ) const { return val == right.val; }
		bool operator!=( const UInt24& right ) const { return val != right.val; }
		bool operator > ( const UInt24& right ) const { return val > right.val; }
		bool operator < ( const UInt24& right ) const { return val < right.val; }
		const UInt24 operator+( const UInt24 &other ) const { return UInt24(val + other.val); }
		const UInt24 operator-( const UInt24 &other ) const { return UInt24(val - other.val); }
		const UInt24 operator/( const UInt24 &other ) const { return UInt24(val / other.val); }
		const UInt24 operator*( const UInt24 &other ) const { return UInt24(val*other.val); }

		UInt24(const unsigned int& a) { val = a; val &= 0x00FFFFFF; }
		UInt24 operator&( const unsigned int& a ) { return UInt24(val&a); }
		UInt24& operator=( const unsigned int& a ) { val = a; val &= 0x00FFFFFF; return *this; }
		UInt24& operator+=( const unsigned int& a ) { val += a; val &= 0x00FFFFFF; return *this; }
		UInt24& operator-=( const unsigned int& a ) { val -= a; val &= 0x00FFFFFF; return *this; }
		bool operator==( const unsigned int& right ) const { return val == ( right & 0x00FFFFFF ); }
		bool operator!=( const unsigned int& right ) const { return val != ( right & 0x00FFFFFF ); }
		bool operator > ( const unsigned int& right ) const { return val > ( right & 0x00FFFFFF ); }
		bool operator < ( const unsigned int& right ) const { return val < ( right & 0x00FFFFFF ); }
		const UInt24 operator+( const unsigned int &other ) const { return UInt24(val + other); }
		const UInt24 operator-( const unsigned int &other ) const { return UInt24(val - other); }
		const UInt24 operator/( const unsigned int &other ) const { return UInt24(val / other); }
		const UInt24 operator*( const unsigned int &other ) const { return UInt24(val*other); }
	};
}

#endif
