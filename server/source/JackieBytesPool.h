/// written on  [1/19/2016 JACKIE]
#ifndef JACKIE_BYTE_POOL
#define JACKIE_BYTE_POOL

#include "JackieMemoryPool.h"
//#include "JackieSimpleMutex.h"
#include "OverrideMemory.h"
// #define _DISABLE_BYTE_POOL
// #define _THREADSAFE_BYTE_POOL
namespace DataStructures
{
	// Allocate some number of bytes from pools.  Uses the heap if necessary.
	class JACKIE_EXPORT JackieBytesPool
	{
	private:
		JackieMemoryPool<unsigned char[1], 8, 1> pool1;
		JackieMemoryPool<unsigned char[2], 16, 1> pool2;
		JackieMemoryPool<unsigned char[4], 32, 1> pool4;
		JackieMemoryPool<unsigned char[8], 64, 1> pool8;
		JackieMemoryPool<unsigned char[16], 128, 1> pool16;
		JackieMemoryPool<unsigned char[32], 256, 1> pool32;
		JackieMemoryPool<unsigned char[64], 512, 1> pool64;
		JackieMemoryPool<unsigned char[128], 256, 1> pool128;
		JackieMemoryPool<unsigned char[256], 128, 1> pool256;
		JackieMemoryPool<unsigned char[512], 64, 1> pool512;
		JackieMemoryPool<unsigned char[1024], 32, 1> pool1024;
		JackieMemoryPool<unsigned char[2048], 16, 1> pool2048;
		JackieMemoryPool<unsigned char[4096], 8, 1> pool4096;
		JackieMemoryPool<unsigned char[8192], 4, 1> pool8192;
		static JackieBytesPool *instance;

		unsigned int RoundUpPower2(unsigned int val)
		{
			if ((val & (val - 1) == 0)) return val;
			unsigned int maxulong = (unsigned int)((unsigned int)~0);
			unsigned int andv = ~(maxulong&(maxulong >> 1));
			while ((andv & val) == 0) andv >>= 1;
			return andv << 1;
		}

	public:
		JackieBytesPool(){}
		~JackieBytesPool(){}
		unsigned char* Allocate(unsigned int bytesWanted, const char *file, unsigned int line);
		void Release(unsigned char *data, const char *file, unsigned int line);
		static JackieBytesPool* GetInstance(void)
		{
			if (instance == 0)
				instance = JACKIE_INET::OP_NEW<JackieBytesPool>(TRACE_FILE_AND_LINE_);
			return instance;
		}
		static void DestroyInstance(void)
		{
			if (instance != 0)
				JACKIE_INET::OP_DELETE(instance, TRACE_FILE_AND_LINE_);
			instance = 0;
		}
	};
}
#endif
