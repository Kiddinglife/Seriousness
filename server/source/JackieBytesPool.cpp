#include "JackieBytesPool.h"
#ifndef __APPLE__
// Use stdlib and not malloc for compatibility
#include <stdlib.h>
#endif
namespace DataStructures
{
	JackieBytesPool* JackieBytesPool::instance = 0;
	unsigned char* JackieBytesPool::Allocate(unsigned int bytesWanted,
		const char *file, unsigned int line)
	{
		assert(bytesWanted != 0);
		bytesWanted = RoundUpPower2(bytesWanted);
		unsigned char *out;
		switch (bytesWanted)
		{
		case 1: //0
			out = (unsigned char*)pool1.Allocate();
			*out = 0;
			return out + 1;
			break;
		case 2://1
			out = (unsigned char*)pool2.Allocate();
			*out = 1;
			return out + 1;
			break;
		case 4://2
			out = (unsigned char*)pool4.Allocate();
			*out = 2;
			return out + 1;
			break;
		case 8://3
			out = (unsigned char*)pool8.Allocate();
			*out = 3;
			return out + 1;
			break;
		case 16://4
			out = (unsigned char*)pool16.Allocate();
			*out = 4;
			return out + 1;
			break;
		case 32://5
			out = (unsigned char*)pool32.Allocate();
			*out = 5;
			return out + 1;
			break;
		case 64://6
			out = (unsigned char*)pool64.Allocate();
			*out = 6;
			return out + 1;
			break;
		case 128://7
			out = (unsigned char*)pool128.Allocate();
			*out = 7;
			return out + 1;
			break;
		case 256://8
			out = (unsigned char*)pool256.Allocate();
			*out = 8;
			return out + 1;
			break;
		case 512://9
			out = (unsigned char*)pool512.Allocate();
			*out = 9;
			return out + 1;
			break;
		case 1024://10
			out = (unsigned char*)pool1024.Allocate();
			*out = 10;
			return out + 1;
			break;
		case 2048://11
			out = (unsigned char*)pool2048.Allocate();
			*out = 11;
			return out + 1;
			break;
		case 4096://12
			out = (unsigned char*)pool4096.Allocate();
			*out = 12;
			return out + 1;
			break;
		case 8192://13
			out = (unsigned char*)pool8192.Allocate();
			*out = 13;
			return out + 1;
			break;
		default:
			assert(0 && "no such option");
		}
	}

	void JackieBytesPool::Release(unsigned char *data, const char *file, unsigned int line)
	{
		assert(data != 0);
		switch ((data - 1)[0])
		{
		case 0: //1
			pool1.Reclaim((unsigned char(*)[1])(data - 1));
			break;
		case 1://2
			pool2.Reclaim((unsigned char(*)[2])(data - 1));
			break;
		case 2://4
			pool4.Reclaim((unsigned char(*)[4])(data - 1));
			break;
		case 3://8
			pool8.Reclaim((unsigned char(*)[8])(data - 1));
			break;
		case 4://16
			pool16.Reclaim((unsigned char(*)[16])(data - 1));
			break;
		case 5://32
			pool32.Reclaim((unsigned char(*)[32])(data - 1));
			break;
		case 6://64
			pool64.Reclaim((unsigned char(*)[64])(data - 1));
			break;
		case 7://128
			pool128.Reclaim((unsigned char(*)[128])(data - 1));
			break;
		case 8://256
			pool256.Reclaim((unsigned char(*)[256])(data - 1));
			break;
		case 9://512
			pool512.Reclaim((unsigned char(*)[512])(data - 1));
			break;
		case 10://1024
			pool1024.Reclaim((unsigned char(*)[1024])(data - 1));
			break;
		case 11://2048
			pool2048.Reclaim((unsigned char(*)[2048])(data - 1));
			break;
		case 12://4096
			pool4096.Reclaim((unsigned char(*)[4096])(data - 1));
			break;
		case 13://8192
			pool8192.Reclaim((unsigned char(*)[8192])(data - 1));
			break;
		default:
			assert(0 && "no such option");
		}
	}

}