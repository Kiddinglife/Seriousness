#include "JACKIE_INet_Socket.h"

namespace JACKIE_INET
{
	inline void JACKIE_INet_Socket::GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR])
	{
#if defined(WINDOWS_STORE_RT)
		JISWINSTROE8::GetMyIP(addresses);
#elif defined(__native_client__)
		RNS2_NativeClient::GetMyIP(addresses);
#elif defined(_WIN32)
		JISWins::GetMyIP(addresses);
#else
		RNS2_Linux::GetMyIP(addresses);
#endif
	}

	JISWins::JISWins() : JISBerkley()
	{
	}

	JISWins::~JISWins()
	{
	}

	JACKIE_INET::JISBindResult JISWins::Bind(JISBerkleyBindParams *bindParameters, const char *file, UInt32 line)
	{
		return JISBindResult_SUCCESS;
	}

	JACKIE_INET::JISSendResult JISWins::Send(JISSendParams *sendParameters, const char *file, UInt32 line)
	{
		return 0;
	}

	void JISWins::SetSocketLayerOverride(JACKIE_ISocketTransceiver *_slo)
	{

	}

	JACKIE_ISocketTransceiver* JISWins::GetSocketLayerOverride(void)
	{
		return 0;
	}

	void JISWins::GetMyIP(JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR])
	{

	}

	JACKIE_INET::JISSendResult JISWins::SendWinsLinux360WithoutVDP(JISSocket rns2Socket, JISSendParams *sendParameters, const char *file, UInt32 line)
	{
		return 0;
	}

	void JISWins::GetMyIPIPV4(JACKIE_INET_Address addrs[MAX_COUNT_LOCAL_IP_ADDR])
	{

	}

	void JISWins::GetMyIPV4AndV6(JACKIE_INET_Address addrs[MAX_COUNT_LOCAL_IP_ADDR])
	{

	}

	inline JACKIE_INet_Socket*  JISAllocator::AllocJIS(void)
	{
		JACKIE_INet_Socket* s2;
#if defined(WINDOWS_STORE_RT)
		s2 = JACKIE_INET::OP_NEW<JISWINSTROE8>(TRACE_FILE_AND_LINE_);
		s2->SetSocketType(JISType_WINDOWS_STORE_8);
#elif defined(__native_client__)
		s2 = JACKIE_INET::OP_NEW<RNS2_NativeClient>(TRACE_FILE_AND_LINE_);
		s2->SetSocketType(RNS2T_CHROME);
#elif defined(_WIN32)
		s2 = JACKIE_INET::OP_NEW<JISWins>(TRACE_FILE_AND_LINE_);
		s2->SetSocketType(JISType_LINUX);
#else
		s2 = JACKIE_INET::OP_NEW<JISLinux>(TRACE_FILE_AND_LINE_);
		s2->SetSocketType(RNS2T_LINUX);
#endif
		return s2;
	}
	inline void  JISAllocator::DeallocJIS(JACKIE_INet_Socket *s)
	{
		JACKIE_INET::OP_DELETE(s, TRACE_FILE_AND_LINE_);
	}

}
