#include "JackieReliabler.h"
#include "JackieApplication.h"
#include "JackieBits.h"

namespace JACKIE_INET
{
	JackieReliabler::JackieReliabler() { }
	JackieReliabler::~JackieReliabler() { }

	void JackieReliabler::ApplyNetworkSimulator(double _packetloss, unsigned short _minExtraPing, unsigned short _extraPingVariance)
	{

	}

	bool JackieReliabler::ProcessOneConnectedRecvParams(JackieApplication* serverApp, JISRecvParams* recvParams, unsigned mtuSize)
	{
		return true;
	}

	void JackieReliabler::Reset(bool param1, int MTUSize, bool client_has_security)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void JackieReliabler::SetSplitMessageProgressInterval(int splitMessageProgressInterval)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void JackieReliabler::SetUnreliableTimeout(TimeMS unreliableTimeout)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void JackieReliabler::SetTimeoutTime(TimeMS defaultTimeoutTime)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

}
