#ifndef COMPILE_FEATURES_H_
#define COMPILE_FEATURES_H_

// If you want to change these defines, 
/// put them in CompileFeaturesOverride.h 
/// so your changes are not lost when updating JackieNet
// The user should not edit this file
#include "CompileFeaturesOverride.h"

// Uncomment below defines, and paste to CompileFeaturesOverride.h, 
/// to exclude plugins that you do not want to build into the static library, or DLL
// These are not all the plugins, only those that are in the core library
// Other plugins are located in DependentExtensions
// #define _JackieNet_SUPPORT_ConnectionGraph2 0
// #define _JackieNet_SUPPORT_DirectoryDeltaTransfer 0
// #define _JackieNet_SUPPORT_FileListTransfer 0
// #define _JackieNet_SUPPORT_FullyConnectedMesh2 0
// #define _JackieNet_SUPPORT_MessageFilter 0
// #define _JackieNet_SUPPORT_NatPunchthroughClient 0
// #define _JackieNet_SUPPORT_NatPunchthroughServer 0
// #define _JackieNet_SUPPORT_NatTypeDetectionClient 0
// #define _JackieNet_SUPPORT_NatTypeDetectionServer 0
// #define _JackieNet_SUPPORT_PacketLogger 0
// #define _JackieNet_SUPPORT_ReadyEvent 0
// #define _JackieNet_SUPPORT_ReplicaManager3 0
// #define _JackieNet_SUPPORT_Router2 0
// #define _JackieNet_SUPPORT_RPC4Plugin 0
// #define _JackieNet_SUPPORT_TeamBalancer 0
// #define _JackieNet_SUPPORT_TeamManager 0
// #define _JackieNet_SUPPORT_UDPProxyClient 0
// #define _JackieNet_SUPPORT_UDPProxyCoordinator 0
// #define _JackieNet_SUPPORT_UDPProxyServer 0
// #define _JackieNet_SUPPORT_ConsoleServer 0
// #define _JackieNet_SUPPORT_JackieNetTransport 0
// #define _JackieNet_SUPPORT_TelnetTransport 0
// #define _JackieNet_SUPPORT_TCPInterface 0
// #define _JackieNet_SUPPORT_LogCommandParser 0
// #define _JackieNet_SUPPORT_JackieNetCommandParser 0
// #define _JackieNet_SUPPORT_EmailSender 0
// #define _JackieNet_SUPPORT_HTTPConnection 0
// #define _JackieNet_SUPPORT_HTTPConnection2 0
// #define _JackieNet_SUPPORT_PacketizedTCP 0
// #define _JackieNet_SUPPORT_TwoWayAuthentication 0

// SET DEFAULTS IF UNDEFINED
#ifndef LIBCAT_SECURITY
#define LIBCAT_SECURITY 0
#endif
#ifndef _JackieNet_SUPPORT_ConnectionGraph2
#define _JackieNet_SUPPORT_ConnectionGraph2 1
#endif
#ifndef _JackieNet_SUPPORT_DirectoryDeltaTransfer
#define _JackieNet_SUPPORT_DirectoryDeltaTransfer 1
#endif
#ifndef _JackieNet_SUPPORT_FileListTransfer
#define _JackieNet_SUPPORT_FileListTransfer 1
#endif
#ifndef _JackieNet_SUPPORT_FullyConnectedMesh
#define _JackieNet_SUPPORT_FullyConnectedMesh 1
#endif
#ifndef _JackieNet_SUPPORT_FullyConnectedMesh2
#define _JackieNet_SUPPORT_FullyConnectedMesh2 1
#endif
#ifndef _JackieNet_SUPPORT_MessageFilter
#define _JackieNet_SUPPORT_MessageFilter 1
#endif
#ifndef _JackieNet_SUPPORT_NatPunchthroughClient
#define _JackieNet_SUPPORT_NatPunchthroughClient 1
#endif
#ifndef _JackieNet_SUPPORT_NatPunchthroughServer
#define _JackieNet_SUPPORT_NatPunchthroughServer 1
#endif
#ifndef _JackieNet_SUPPORT_NatTypeDetectionClient
#define _JackieNet_SUPPORT_NatTypeDetectionClient 1
#endif
#ifndef _JackieNet_SUPPORT_NatTypeDetectionServer
#define _JackieNet_SUPPORT_NatTypeDetectionServer 1
#endif
#ifndef _JackieNet_SUPPORT_PacketLogger
#define _JackieNet_SUPPORT_PacketLogger 1
#endif
#ifndef _JackieNet_SUPPORT_ReadyEvent
#define _JackieNet_SUPPORT_ReadyEvent 1
#endif
#ifndef _JackieNet_SUPPORT_ReplicaManager3
#define _JackieNet_SUPPORT_ReplicaManager3 1
#endif
#ifndef _JackieNet_SUPPORT_Router2
#define _JackieNet_SUPPORT_Router2 1
#endif
#ifndef _JackieNet_SUPPORT_RPC4Plugin
#define _JackieNet_SUPPORT_RPC4Plugin 1
#endif
#ifndef _JackieNet_SUPPORT_TeamBalancer
#define _JackieNet_SUPPORT_TeamBalancer 1
#endif
#ifndef _JackieNet_SUPPORT_TeamManager
#define _JackieNet_SUPPORT_TeamManager 1
#endif
#ifndef _JackieNet_SUPPORT_UDPProxyClient
#define _JackieNet_SUPPORT_UDPProxyClient 1
#endif
#ifndef _JackieNet_SUPPORT_UDPProxyCoordinator
#define _JackieNet_SUPPORT_UDPProxyCoordinator 1
#endif
#ifndef _JackieNet_SUPPORT_UDPProxyServer
#define _JackieNet_SUPPORT_UDPProxyServer 1
#endif
#ifndef _JackieNet_SUPPORT_ConsoleServer
#define _JackieNet_SUPPORT_ConsoleServer 1
#endif
#ifndef _JackieNet_SUPPORT_JackieNetTransport
#define _JackieNet_SUPPORT_JackieNetTransport 1
#endif
#ifndef _JackieNet_SUPPORT_TelnetTransport
#define _JackieNet_SUPPORT_TelnetTransport 1
#endif
#ifndef _JackieNet_SUPPORT_TCPInterface
#define _JackieNet_SUPPORT_TCPInterface 1
#endif
#ifndef _JackieNet_SUPPORT_LogCommandParser
#define _JackieNet_SUPPORT_LogCommandParser 1
#endif
#ifndef _JackieNet_SUPPORT_JackieNetCommandParser
#define _JackieNet_SUPPORT_JackieNetCommandParser 1
#endif
#ifndef _JackieNet_SUPPORT_EmailSender
#define _JackieNet_SUPPORT_EmailSender 1
#endif
#ifndef _JackieNet_SUPPORT_HTTPConnection
#define _JackieNet_SUPPORT_HTTPConnection 1
#endif
#ifndef _JackieNet_SUPPORT_HTTPConnection2
#define _JackieNet_SUPPORT_HTTPConnection2 1
#endif
#ifndef _JackieNet_SUPPORT_PacketizedTCP
#define _JackieNet_SUPPORT_PacketizedTCP 1
#endif
#ifndef _JackieNet_SUPPORT_TwoWayAuthentication
#define _JackieNet_SUPPORT_TwoWayAuthentication 1
#endif
#ifndef _JackieNet_SUPPORT_CloudClient
#define _JackieNet_SUPPORT_CloudClient 1
#endif
#ifndef _JackieNet_SUPPORT_CloudServer
#define _JackieNet_SUPPORT_CloudServer 1
#endif
#ifndef _JackieNet_SUPPORT_DynDNS
#define _JackieNet_SUPPORT_DynDNS 1
#endif
#ifndef _JackieNet_SUPPORT_Rackspace
#define _JackieNet_SUPPORT_Rackspace 1
#endif
#ifndef _JackieNet_SUPPORT_FileOperations
#define _JackieNet_SUPPORT_FileOperations 1
#endif
#ifndef _JackieNet_SUPPORT_UDPForwarder
#define _JackieNet_SUPPORT_UDPForwarder 1
#endif
#ifndef _JackieNet_SUPPORT_StatisticsHistory
#define _JackieNet_SUPPORT_StatisticsHistory 1
#endif
#ifndef _JackieNet_SUPPORT_LibVoice
#define _JackieNet_SUPPORT_LibVoice 0
#endif
#ifndef _JackieNet_SUPPORT_RelayPlugin
#define _JackieNet_SUPPORT_RelayPlugin 1
#endif

// Take care of dependencies
#if _JackieNet_SUPPORT_DirectoryDeltaTransfer==1
#undef _JackieNet_SUPPORT_FileListTransfer
#define _JackieNet_SUPPORT_FileListTransfer 1
#endif
#if _JackieNet_SUPPORT_FullyConnectedMesh2==1
#undef _JackieNet_SUPPORT_ConnectionGraph2
#define _JackieNet_SUPPORT_ConnectionGraph2 1
#endif
#if _JackieNet_SUPPORT_TelnetTransport==1
#undef _JackieNet_SUPPORT_PacketizedTCP
#define _JackieNet_SUPPORT_PacketizedTCP 1
#endif
#if _JackieNet_SUPPORT_PacketizedTCP==1 || _JackieNet_SUPPORT_EmailSender==1 || _JackieNet_SUPPORT_HTTPConnection==1
#undef _JackieNet_SUPPORT_TCPInterface
#define _JackieNet_SUPPORT_TCPInterface 1
#endif
#endif