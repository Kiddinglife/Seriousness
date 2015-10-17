/*
*  RakNetDefines.h
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*/
#ifndef DEFAULT_DEFINES_H_
#define DEFAULT_DEFINES_H_

#ifdef _MSC_VER
#pragma warning( disable : 4702 ) // warning C4702: unreachable code
#endif

/// If you want to change these defines, put them in OverrideDefines.h 
/// so your changes are not lost when updating JackieNet
/// The user should not handly-change the content of DefaultDefine.h
#include "OverrideNetDefines.h"
#include <assert.h>

/// #define USE_MEM_TRACK_OVERRIDE 1 for custom memory tracking
/// See OverrideMemory.h. 
#ifndef USE_MEM_OVERRIDE
#define USE_MEM_OVERRIDE 0
#endif

/// Threshold at which to do a malloc / free rather than pushing data onto a fixed stack
/// for the bitstream class. 256 is an arbitrary size, just picking something likely to be larger 
/// than  most packets
#ifndef BITSTREAM_STACK_ALLOCATION_SIZE
#define BITSTREAM_STACK_ALLOCATION_SIZE 256
#endif

// Redefine if you want to disable or change the target for debug RAKNET_DEBUG_PRINTF
#ifndef JACKIE_NET_DEBUG_PRINTF
#define JACKIE_NET_DEBUG_PRINTF printf
#endif

/// #define DO_BYTE_SWAP 0 means NOT support byte swapping in the BitStream class.  
/// This is faster and is what you should use.  unless you actually plan to have different 
/// endianness systems connect to each other. 
/// Support  byte-swapping by default.
#ifndef DO_BYTE_SWAP
#define DO_BYTE_SWAP 1
#endif

/// Maximum (stack) size to use with _alloca before using new and delete instead.
#ifndef MAX_ALLOC_STACK_COUNT
#define MAX_ALLOC_STACK_COUNT 1048576
#endif

/// Define USE_TIME_MS_64BITS to have JackieNet::TimeMS use a 64, 
/// rather than 32 bit value.  A 32 bit value will overflow after about 5 weeks.
/// However, this doubles the bandwidth use for sending times, so don't do 
/// it unless you have a reason to. This must be the same on all systems, or they
/// won't connect. Comment out if you are using the iPod Touch TG. 
/// See http://www.jenkinssoftware.com/forum/index.php?topic=2717.0
#ifndef USE_TIME_MS_64BITS
#define USE_TIME_MS_64BITS 1
#endif

#ifndef TRACE_FILE_NAME
#define TRACE_FILE_NAME __FILE__
/// if you want to strip out file and line info for memory tracking from the EXE
/// #define TRACE_FILE_NAME  "" or 0
#endif
#ifndef TRACE_LINE_NUMBER
#define TRACE_LINE_NUMBER __LINE__
/// if you want to strip out file and line info for memory tracking from the EXE
/// #define TRACE_LINE_NUMBER  "" or 0
#endif

#ifndef TRACE_FILE_AND_LINE_
#define TRACE_FILE_AND_LINE_ __FILE__,__LINE__
/// if you want to strip out file and line info for memory tracking from the EXE
/// #define TRACE_LINE_NUMBER  "" or 0
#endif

/// Use WaitForSingleObject instead of sleep. Defining it plays nicer with other systems, and 
/// uses less CPU, but gives worse RakNet performance. Undefining it uses more CPU time,
/// but is more responsive and faster.
#ifndef _WIN32_WCE
#define USE_WAIT_FOR_MULTIPLE_EVENTS
#endif

/// #define OPEN_SSL_CLIENT_SUPPORT  1 means that OpenSSL is enabled for the class 
/// TCPInterface. This is necessary to use the SendEmail class with Google POP servers
/// This also requires that you enable header search paths to DependentExtensions
/// \openssl-1.0.0d
#ifndef OPEN_SSL_CLIENT_SUPPORT
#define OPEN_SSL_CLIENT_SUPPORT 0
#endif

#ifndef MAX_COUNT_LOCAL_IP_ADDR
#define MAX_COUNT_LOCAL_IP_ADDR 10 /// Maximum number of local IP addresses supported
#endif

/// ASSERT
#ifndef JACKIE_ASSERT
#if defined(__native_client__)
#define JACKIE_ASSERT(x)
#else
#if defined(_DEBUG)
#define JACKIE_ASSERT(x) assert(x);
#else
#define JACKIE_ASSERT(x) 
#endif
#endif
#endif

/// This controls the amount of memory used per connection. This many datagrams are 
/// tracked by datagramNumber. If more than this many datagrams are sent, then an ack
/// for an older datagram would be ignored. This results in an unnecessary resend in that 
/// case
#ifndef DATAGRAM_MESSAGE_ID_ARRAY_LENGTH
#define DATAGRAM_MESSAGE_ID_ARRAY_LENGTH 512
#endif

/// This is the maximum number of reliable user messages that can be on the wire at a time
/// If this is too low, then high ping connections with a large throughput will be 
/// underutilized(未被充分使用).  This will be evident because 
/// RakNetStatistics::messagesInSend buffer will  increase over time, yet at the same time the
/// outgoing bandwidth per second is less than your connection supports
#ifndef RESEND_BUFFER_ARRAY_LENGTH
#define RESEND_BUFFER_ARRAY_LENGTH 512
#define RESEND_BUFFER_ARRAY_MASK 511
#endif

/// Uncomment if you want to link in the DLMalloc library to use with RakMemoryOverride
// #define _LINK_DL_MALLOC

#ifndef GET_TIME_SPIKE_LIMIT
/// Workaround for http://support.microsoft.com/kb/274323
/// If two calls between RakNet::GetTime() happen farther apart than this time in 
/// microseconds, this delta will be returned instead. Note: This will cause ID_TIMESTAMP to 
/// be temporarily inaccurate if you set a breakpoint that pauses the UpdateNetworkLoop() 
/// thread in RakPeer. Define in OverrideDefines.h to enable (non-zero) or disable (0)
#define GET_TIME_SPIKE_LIMIT 0
#endif

/// Use sliding window congestion control instead of ping based congestion control
#ifndef USE_SLIDING_WINDOW_CONGESTION_CONTROL
#define USE_SLIDING_WINDOW_CONGESTION_CONTROL 1
#endif

/// When a large message is arriving, preallocate the memory for the entire block
/// This results in large messages not taking up time to reassembly with memcpy, but is 
/// vulnerable to attackers causing the host to run out of memory
#ifndef PREALLOCATE_LARGE_MESSAGES
#define PREALLOCATE_LARGE_MESSAGES 0
#endif

/// Define in OverrideDefines.h to enable (non-zero) or disable (0)
#ifndef NET_SUPPORT_IPV6
#define NET_SUPPORT_IPV6 0
#endif

#ifndef RAKSTRING_TYPE
#if defined(_UNICODE)
#define RAKSTRING_TYPE RakWString
#define RAKSTRING_TYPE_IS_UNICODE 1
#else
#define RAKSTRING_TYPE RakString
#define RAKSTRING_TYPE_IS_UNICODE 0
#endif
#endif

#ifndef RPC_GLOBAL_REGISTRATION_MAX_FUNCTIONS
#define RPC_GLOBAL_REGISTRATION_MAX_FUNCTIONS 48
#endif

#ifndef RPC4_GLOBAL_REGISTRATION_MAX_FUNCTION_NAME_LENGTH
#define RPC4_GLOBAL_REGISTRATION_MAX_FUNCTION_NAME_LENGTH 48
#endif

#ifndef XBOX_BYPASS_SECURITY
#define XBOX_BYPASS_SECURITY 1
#endif

/// Controls how many allocations occur at once for the memory pool of incoming 
/// datagrams waiting to be transferred between the recvfrom thread and the main 
/// update thread. Has large effect on memory usage, per instance of RakPeer. 
/// Approximately MAXIMUM_MTU_SIZE*BUFFERED_PACKETS_PAGE_SIZE bytes, once 
/// after calling RakPeer::Startup()
#ifndef BUFFERED_PACKETS_PAGE_SIZE
#define BUFFERED_PACKETS_PAGE_SIZE 8
#endif

/// Controls how many allocations occur at once for the memory pool of incoming or 
/// outgoing datagrams. Has small effect on memory usage per connection. Uses about 
/// 256 bytes*INTERNAL_PACKET_PAGE_SIZE per connection
#ifndef INTERNAL_PACKET_PAGE_SIZE
#define INTERNAL_PACKET_PAGE_SIZE 8
#endif

/// If defined to 1, the user is responsible for 
/// calling RakPeer::RunUpdateCycle and RakPeer::RunRecvfrom
#ifndef RAKPEER_USER_THREADED
#define RAKPEER_USER_THREADED 0
#endif

/// 内存分配函数,与malloc,calloc,realloc类似.
/// 但是注意一个重要的区别, _alloca是在栈(stack)上申请空间, 用完马上就释放.
///  unsigned若省略后一个关键字，大多数编译器都会认为是unsigned int。
#ifndef USE_STACK_ALLOCA
#define USE_STACK_ALLOCA 1
#endif

/// Define in OverrideDefines.h to enable (non-zero) or disable (0)
#ifndef USE_THREADED_SEND
#define USE_THREADED_SEND 0
#endif

#ifndef JACKIE_SO_SNDBUF_SIZE
#define JACKIE_SO_SNDBUF_SIZE 1024*256; //256KB
#endif

#ifndef JACKIE_SO_REVBUF_SIZE
#define JACKIE_SO_REVBUF_SIZE 1024*256; //256KB
#endif

#endif
