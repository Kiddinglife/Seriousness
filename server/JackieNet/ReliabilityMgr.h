#ifndef ReliabilityLayer_H_
#define ReliabilityLayer_H_

#include "DLLExport.h"

class JACKIE_EXPORT ReliabilityLayer
{
	public:
	ReliabilityLayer() { }
	~ReliabilityLayer() { }

	void ApplyNetworkSimulator(double _packetloss, unsigned short _minExtraPing, unsigned short _extraPingVariance)
	{
	}


};

#endif



