/*!
 * \file MemPoolAllocQueue.h
 *
 * \author mengdi
 * \date Oct 28 2015
 *
 * \internal
 * A threadsafe Ring Buffer Queue, that also uses a memory pool for allocation
 */
#ifndef MemPoolAllocateElementQueue_H_
#define MemPoolAllocateElementQueue_H_

#include "RingBufferQueue.h"
#include "MemoryPool.h"
#include "JACKIE_Simple_Mutex.h"

namespace DataStructures
{
	template <typename structureType,
		UInt32 MEMPOOL_BLOCKS_COUNT_PER_PAGE = 256, 
		UInt32 MEMORY_POOL_MAX_FREE_PAGES = 4, 
		UInt32 QUEUE_INIT_SIZE = 32>
	class JACKIE_EXPORT MemPoolAllocQueue
	{
		protected:
		mutable MemoryPool<structureType, MEMPOOL_BLOCKS_COUNT_PER_PAGE, MEMORY_POOL_MAX_FREE_PAGES> memoryPool;
		JACKIE_Simple_Mutex memoryPoolMutex;

		RingBufferQueue<structureType*, QUEUE_INIT_SIZE> queue;
		JACKIE_Simple_Mutex queueMutex;

		public:
		// Queue operations
		void PushTail(structureType *s)
		{
			queueMutex.Lock();
			queue.PushTail(s);
			queueMutex.Unlock();
		}

		structureType *PopHeadInaccurate(void)
		{
			structureType *s;
			if( queue.IsEmpty() ) return 0;
			queueMutex.Lock();
			if( queue.IsEmpty() == false )
				s = queue.PopHead();
			else
				s = 0;
			queueMutex.Unlock();
			return s;
		}
		structureType *PopHead(void)
		{
			structureType *s;
			queueMutex.Lock();
			if( queue.IsEmpty() )
			{
				queueMutex.Unlock();
				return 0;
			}
			s = queue.PopHead();
			queueMutex.Unlock();
			return s;
		}

		bool IsEmpty(void)
		{
			bool isEmpty;
			//queueMutex.Lock();
			isEmpty = queue.IsEmpty();
			//queueMutex.Unlock();
			return isEmpty;
		}
		void RemoveAtIndex(unsigned int position)
		{
			queueMutex.Lock();
			queue.RemoveAtIndex(position);
			queueMutex.Unlock();
		}
		unsigned int Size(void)
		{
			//queueMutex.Lock();
			return queue.Size();
			//queueMutex.Unlock();
		}

		// Memory pool operations
		structureType *Allocate(const char *file, unsigned int line)
		{
			structureType *s;
			memoryPoolMutex.Lock();
			s = memoryPool.Allocate();
			memoryPoolMutex.Unlock();
			// Call new operator, memoryPool doesn't do this
			s = new ( (void*) s ) structureType;
			return s;
		}
		/// Call delete operator, memory pool doesn't do this
		void Deallocate(structureType *s, const char *file, unsigned int line)
		{
			memoryPoolMutex.Lock();
			s->~structureType();
			memoryPool.Reclaim(s);
			memoryPoolMutex.Unlock();
		}

		void Clear(const char *file, unsigned int line)
		{
			memoryPoolMutex.Lock();
			for( unsigned int i = 0; i < queue.Size(); i++ )
			{
				queue[i]->~structureType();
				memoryPool.Reclaim(queue[i]);
			}
			queue.Clear();
			memoryPool.Clear();
			memoryPoolMutex.Unlock();
		}
	};
}
#endif