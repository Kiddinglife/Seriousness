#ifndef SingleProducerSingleConsumerLockFreeQueue_H_
#define SingleProducerSingleConsumerLockFreeQueue_H_

namespace DataStructures
{
	class SingleProducerSingleConsumerLockFreeQueue
	{
		public:
		SingleProducerSingleConsumerLockFreeQueue(int nSize);
		virtual ~SingleProducerSingleConsumerLockFreeQueue();


		unsigned int PushTail(const unsigned char *pBuffer, unsigned int nLen);
		unsigned int PopHead(unsigned char *pBuffer, unsigned int nLen);

		void Clean() { m_nIn = m_nOut = 0; }
		unsigned int GetDataLen() const { return  m_nIn - m_nOut; }

		private:
		bool Init();
		bool IsPower2(unsigned long n) { return ( n != 0 && ( ( n & ( n - 1 ) ) == 0 ) ); };
		unsigned long RoundUpPower2(unsigned long val);

		private:
		unsigned long long padding1[16];
		unsigned int   m_nIn;        /* data is added at offset (in % size) */
		unsigned long long padding2[16];
		unsigned int   m_nOut;        /* data is extracted from off. (out % size) */
		unsigned long long padding3[16];

		char *m_pBuffer;    /* the buffer holding the data */
		unsigned int   m_nSize;        /* the size of the allocated buffer */
	};
}
#endif
