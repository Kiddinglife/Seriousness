#include "ReliabilityLayer.h"
#include "ServerApplication.h"
#include "JackieStream.h"

namespace JACKIE_INET
{
	ReliabilityLayer::ReliabilityLayer() { }
	ReliabilityLayer::~ReliabilityLayer() { }

	void ReliabilityLayer::ApplyNetworkSimulator(double _packetloss, unsigned short _minExtraPing, unsigned short _extraPingVariance)
	{

	}

	bool ReliabilityLayer::ProcessJISRecvParamsFromConnectedEndPoint(ServerApplication* serverApp, int MTUSize)
	{
		return true;
	}

}
