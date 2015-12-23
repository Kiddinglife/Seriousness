#ifndef ReliabilityLayer_H_
#define ReliabilityLayer_H_

#include "DLLExport.h"

namespace JACKIE_INET
{
	class RemoteEndPoint;
	class JISRecvParams;
	class ServerApplication;
	class JackieBits;

	class JACKIE_EXPORT ReliabilityLayer
	{
	private:
		RemoteEndPoint* remoteEndpoint;

		public:
		ReliabilityLayer();
		~ReliabilityLayer();

		void ApplyNetworkSimulator(double _packetloss, unsigned short _minExtraPing, unsigned short _extraPingVariance);

		// Packets are read directly from the socket layer and skip the reliability
		//layer  because unconnected players do not use the reliability layer
		// This function takes packet data after a player has been confirmed as
		//connected.  The game should not use that data directly
		// because some data is used internally, such as packet acknowledgment and
		//split packets
		bool ProcessOneConnectedRecvParams(ServerApplication* serverApp,
			JISRecvParams* recvParams, unsigned mtuSize);
		void Reset(bool param1, int MTUSize, bool client_has_security);
		void SetSplitMessageProgressInterval(int splitMessageProgressInterval);
		void SetUnreliableTimeout(TimeMS unreliableTimeout);
		void SetTimeoutTime(TimeMS defaultTimeoutTime);
	};
}

#endif



