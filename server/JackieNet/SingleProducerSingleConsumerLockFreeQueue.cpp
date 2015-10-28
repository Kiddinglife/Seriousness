#include "LockFreeQueue.h"
#include "OverrideMemory.h"
//#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
__sync_synchronize();
#endif
namespace DataStructures
{
	LockFreeQueue::
		LockFreeQueue(int nSize) :
		m_pBuffer(0),
		m_nSize(nSize),
		m_nIn(0),
		m_nOut(0)
	{
		//round up to the next power of 2
		if( !IsPower2(nSize) )
		{
			m_nSize = RoundUpPower2(nSize);
		}
		Init();
	}

	LockFreeQueue::
		~LockFreeQueue()
	{
		if( 0 != m_pBuffer )
		{
			JACKIE_INET::OP_DELETE_ARRAY(m_pBuffer, __FILE__, __LINE__);
			m_pBuffer = 0;
		}
	}

	bool LockFreeQueue::Init()
	{
		m_pBuffer = JACKIE_INET::OP_NEW_ARRAY<char>(m_nSize, __FILE__, __LINE__);
		if( m_pBuffer == 0 ) return false;
		m_nIn = m_nOut = 0;
		return true;
	}

	unsigned long LockFreeQueue::RoundUpPower2(unsigned long val)
	{
		if( ( val & ( val - 1 ) ) == 0 ) return val;
		unsigned long maxulong = (unsigned long) ( (unsigned long) ~0 );
		unsigned long andv = ~( maxulong&( maxulong >> 1 ) );
		while( ( andv & val ) == 0 ) andv >>= 1;
		return andv << 1;
	}

	unsigned int LockFreeQueue::PushTail(const unsigned char *buffer, unsigned int len)
	{
		unsigned int l = m_nSize - m_nIn + m_nOut;
		len = len < l ? len : l;
		/*
		 * Ensure that we sample the m_nOut index -before- we
		 * start putting bytes into the UnlockQueue.
		 */
#ifdef _WIN32
		MemoryBarrier();
#else
		__sync_synchronize();
#endif

		/* first put the data starting from fifo->in to buffer end */
		l = m_nSize - ( m_nIn  & ( m_nSize - 1 ) );
		l = len < l ? len : l;
		memcpy(m_pBuffer + ( m_nIn & ( m_nSize - 1 ) ), buffer, l);

		/* then put the rest (if any) at the beginning of the buffer */
		memcpy(m_pBuffer, buffer + l, len - l);

		/*
		 * Ensure that we add the bytes to the kfifo -before-
		 * we update the fifo->in index.
		 */
#ifdef _WIN32
		MemoryBarrier();
#else
		__sync_synchronize();
#endif

		m_nIn += len;

		return len;
	}

	unsigned int LockFreeQueue::PopHead(unsigned char *buffer, unsigned int len)
	{
		unsigned int l = m_nIn - m_nOut;
		len = len < l ? len : l;
		/*
		 * Ensure that we sample the fifo->in index -before- we
		 * start removing bytes from the kfifo.
		 */
#ifdef _WIN32
		MemoryBarrier();
#else
		__sync_synchronize();
#endif
		/* first get the data from fifo->out until the end of the buffer */
		l = m_nSize - ( m_nOut & ( m_nSize - 1 ) );
		l = len < l ? len : l;
		memcpy(buffer, m_pBuffer + ( m_nOut & ( m_nSize - 1 ) ), l);
		/* then get the rest (if any) from the beginning of the buffer */
		memcpy(buffer + l, m_pBuffer, len - l);

		/*
		 * Ensure that we remove the bytes from the kfifo -before-
		 * we update the fifo->out index.
		 */
#ifdef _WIN32
		MemoryBarrier();
#else
		__sync_synchronize();
#endif
		m_nOut += len;
		return len;
	}
}