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

	void ReliabilityLayer::Reset(bool param1, int MTUSize, bool client_has_security)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void ReliabilityLayer::SetSplitMessageProgressInterval(int splitMessageProgressInterval)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void ReliabilityLayer::SetUnreliableTimeout(TimeMS unreliableTimeout)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void ReliabilityLayer::SetTimeoutTime(TimeMS defaultTimeoutTime)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

}
