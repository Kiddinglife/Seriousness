#include "JackieReliabler.h"
#include "JackieApplication.h"

namespace JACKIE_INET
{
	JackieReliabler::JackieReliabler() { }
	JackieReliabler::~JackieReliabler() { }

	void JackieReliabler::ApplyNetworkSimulator(double _packetloss, unsigned short _minExtraPing, unsigned short _extraPingVariance)
	{
		JDEBUG << " JackieReliabler::ApplyNetworkSimulator is not implemented.";
	}

	bool JackieReliabler::ProcessOneConnectedRecvParams(JackieApplication* serverApp, JISRecvParams* recvParams, unsigned mtuSize)
	{
		JDEBUG << " JackieReliabler::ProcessOneConnectedRecvParams is not implemented.";
		return true;
	}

	void JackieReliabler::Reset(bool param1, int MTUSize, bool client_has_security)
	{
		JDEBUG << " JackieReliabler::Reset is not implemented.";
	}

	void JackieReliabler::SetSplitMessageProgressInterval(int splitMessageProgressInterval)
	{
		JDEBUG << " JackieReliabler::SetSplitMessageProgressInterval is not implemented.";
	}

	void JackieReliabler::SetUnreliableTimeout(TimeMS unreliableTimeout)
	{
		JDEBUG << " JackieReliabler::SetUnreliableTimeout is not implemented.";
	}

	void JackieReliabler::SetTimeoutTime(TimeMS defaultTimeoutTime)
	{
		JDEBUG << " JackieReliabler::SetTimeoutTime is not implemented.";
	}

	bool JackieReliabler::Send(ReliableSendParams& sendParams)
	{
		//remoteSystemList[sendList[sendListIndex]].reliabilityLayer.Send(data, numberOfBitsToSend, priority, reliability, orderingChannel, useData == false, remoteSystemList[sendList[sendListIndex]].MTUSize, currentTime, receipt);
		JDEBUG <<
			"Not implemented JackieReliabler::Send(ReliableSendParams& sendParams)";
		return true;
	}

}
