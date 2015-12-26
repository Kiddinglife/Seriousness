﻿/*
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#ifndef SOCKET_DEFINES_H_
#define SOCKET_DEFINES_H_

/// Internal
#if   defined(WINDOWS_STORE_RT)

#include "WinRTSocketAdapter.h"
#define accept__ WinRTAccept
#define connect__ WinRTConnect
#define closesocket__ WinRTClose
#define socket__ WinRTCreateDatagramSocket
#define bind__ WinRTBind
#define getsockname__ RNS2_WindowsStore8::WinRTGetSockName
#define getsockopt__ WinRTGetSockOpt
#define inet_addr__ RNS2_WindowsStore8::WinRTInet_Addr
#define ioctlsocket__ RNS2_WindowsStore8::WinRTIOCTLSocket
#define listen__ WinRTListen
#define recv__ WinRTRecv
#define recvfrom__ WinRTRecvFrom
#define select__ WinRTSelect
#define send__ WinRTSend
#define sendto__ WinRTSendTo
#define setsockopt__ RNS2_WindowsStore8::WinRTSetSockOpt
#define shutdown__ WinRTShutdown
#define WSASendTo__ WinRTSendTo

#else

#if defined(_WIN32)
#define closesocket__ closesocket
#define select__ select
#elif defined(__native_client__)
#define select__ select
#else
#define closesocket__ close
#define select__ select
#endif

#define accept__ accept
#define connect__ connect
#define socket__ socket
#define bind__ bind
#define getsockname__ getsockname
#define getsockopt__ getsockopt

///将字符串形式的IP地址转换为按网络字节顺序的整型值
/// convert the string ip addr to intege in network order
#define inet_addr__ inet_addr

#define ioctlsocket__ ioctlsocket
#define listen__ listen
#define recv__ recv
#define recvfrom__ recvfrom
#define sendto__ sendto
#define send__ send
#define setsockopt__ setsockopt
#define shutdown__ shutdown
#define WSASendTo__ WSASendTo

#endif

#endif