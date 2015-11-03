/*!
 * \file IApplication.h
 * \date 2015/10/17 14:47
 * \author mengdi
 * Contact: 2502700710@qq.com
 * \brief  Simply contains all user functions as pure virtuals.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 */
#ifndef IServerApplication_H_
#define IServerApplication_H_

#include "DLLExport.h"
//#include "DS_List.h"
#include "OverrideMemory.h"
#include "JackieINetSocket.h"

namespace JACKIE_INET
{
	// Forward declarations
	class JackieStream;
	class PluginInterface2;
	struct RPCMap;
	struct RakNetStatistics;
	struct RakNetBandwidth;
	class RouterInterface;
	class NetworkIDManager;
	struct JISRecvParams;
	//////////////////////////////////////////////////////////////////////////
	/// \class IServerApplication
	/// \brief
	/// The primary interface for JackieNet, ApplicationInstance contains all major
	/// functions for the library. See the individual functions for what the class can do.
	/// The main interface fonetwork communications
	/// \author mengdi
	/// \date Oct 2015
	//////////////////////////////////////////////////////////////////////////
	class JACKIE_EXPORT IServerApplication  //: public JISEventHandler
	{
		public:
		/// GetInstance() and DestroyInstance(instance*)
		/*STATIC_FACTORY_DECLARATIONS(IServerApplication);*/

		virtual ~IServerApplication() { }

		//////////////////////////////////////////////////////////////////////////
		/// Starts the network threads [optional], opens the listen port.
		/// You must call this before calling Connect().
		/// On the PS3, call Startup() after Client_Login()
		/// On Android, add the necessary permission to your application's 
		/// androidmanifest.xml: <uses-permission 
		/// android:name="android.permission.INTERNET" />
		/// Multiple calls while already active are ignored.  
		/// To call this function again with different settings, you must first call End().
		/// param[in] maxConnections 
		/// The maximum number of connections between this instance of ServerApplication 
		/// and another instance of ServerApplication. Required so, the network can preallocate
		/// and for thread safety. A pure client would set this to 1.  A pure server would set it to
		/// the number of allowed clients. A hybrid would set it to the sum of both types of 
		/// connections
		/// param[in] localPort 
		/// The port to listen for connections on. On linux the system may be set up so thast 
		/// ports under 1024 are restricted for everything but the root user. Use a higher port 
		/// for maximum compatibility. 
		//////////////////////////////////////////////////////////////////////////

		///========================================
		/// @Function Start 
		/// @Brief
		/// @Access virtual  public  
		/// @Param [in] [unsigned int maxConnections] 
		/// Required so the network can preallocate and for thread safety. 
		/// A pure client would set this to 1.  
		/// A pure server would set it to the number of allowed clients. 
		/// A mixture would set it to the sum of both types of connections
		/// @Param [in] [JACKIE_LOCAL_SOCKET * socketDescriptors]  
		/// An array of SocketDescriptor structures to force RakNet to 
		/// listen on a particular IP address or port (or both).  
		/// Each SocketDescriptor will represent one unique socket.  
		/// Do not pass redundant structures.  
		/// To listen on a specific port, you can pass SocketDescriptor(myPort,0) such as for a server.  
		/// For a client, it is usually OK to just pass SocketDescriptor(); 
		/// However, on the XBOX be sure to use IPPROTO_VDP
		/// @Param [in] [unsigned int socketDescriptorCount]  
		/// The size of the @socketDescriptors array.
		/// @Param [in] [Int32 threadPriority]  
		/// @Returns [JACKIE_INET::StartupResult]
		/// @Remarks
		/// @Notice
		/// @Author mengdi[Jackie]
		///========================================
		virtual StartupResult Start(unsigned int maxConnections,
			BindSocket *bindSockets,
			unsigned int socketDescriptorCount,
			Int32 threadPriority = -99999)
		{
			return ALREADY_STARTED;
		}

		virtual const JACKIE_INet_GUID& GetMyGUID(void) const { return JACKIE_INet_GUID_Null; };
	};


}

#endif