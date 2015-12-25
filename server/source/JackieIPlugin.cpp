#include "JackieIPlugin.h"

namespace JACKIE_INET
{
	JackieIPlugin::JackieIPlugin()
	{
		serverApplication = 0;
#if JackieNet_SUPPORT_PacketizedTCP==1 && JackieNet_SUPPORT_TCPInterface==1
		tcpInterface = 0;
#endif
	}
}