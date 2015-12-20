#include "ReliabilityLayer.h"
#include "ServerApplication.h"
#include "JackieBits.h"

namespace JACKIE_INET
{
	ReliabilityLayer::ReliabilityLayer() { }
	ReliabilityLayer::~ReliabilityLayer() { }

	void ReliabilityLayer::ApplyNetworkSimulator(double _packetloss, unsigned short _minExtraPing, unsigned short _extraPingVariance)
	{

	}

	bool ReliabilityLayer::ProcessOneConnectedRecvParams(ServerApplication* serverApp, JISRecvParams* recvParams, unsigned mtuSize)
	{
		return true;
	}

}
