#ifndef JackieSlidingWindows_h__
#define JackieSlidingWindows_h__
/*
http://www.ssfnet.org/Exchange/tcp/tcpTutorialNotes.html

cwnd=max bytes allowed on wire at once

Start:
cwnd=mtu
ssthresh=unlimited

Slow start:
On ack cwnd*=2

congestion avoidance:
On ack during new period
cwnd+=mtu*mtu/cwnd

on loss or duplicate ack during period:
sshtresh=cwnd/2
cwnd=MTU
This reenters slow start

If cwnd < ssthresh, then use slow start
else use congestion avoidance
*/
#include "BasicTypes.h"
#include "NetTime.h"
#include "NetTypes.h"
#include "JackieArraryQueue.h"

#define UDP_HEADER_SIZE 28 ///< Sizeof an UDP header in byte

namespace JACKIE_INET
{
	class JackieSlidingWindows
	{
	public:
		JackieSlidingWindows();
		~JackieSlidingWindows();
	};
}

#endif // JackieSlidingWindows_h__

