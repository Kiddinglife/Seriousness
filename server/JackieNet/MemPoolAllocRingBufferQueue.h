/*!
 * \file MemPoolAllocQueue.h
 *
 * \author mengdi
 * \date Oct 28 2015
 *
 * \internal
 * A thread unsafe Ring Buffer Queue, that also uses a memory pool for allocation
 * Basically, one-thread againest one-MemPoolAllocQueue to avoid using locks
 */
#ifndef MemPoolAllocateElementQueue_H_
#define MemPoolAllocateElementQueue_H_

#include "RingBufferQueue.h"
#include "MemoryPool.h"

namespace DataStructures
{

	/// will call dtor and ctor for each of element
	template <typename structureType,
		unsigned int MEMPOOL_BLOCKS_COUNT_PER_PAGE = 256,
		unsigned int MEMORY_POOL_MAX_FREE_PAGES = 4,
		unsigned int QUEUE_INIT_SIZE = 32>
	class JACKIE_EXPORT MemPoolAllocRingBufferQueueCtorDtor
	{
		protected:
		mutable MemoryPool<structureType, MEMPOOL_BLOCKS_COUNT_PER_PAGE, MEMORY_POOL_MAX_FREE_PAGES> memoryPool;

		RingBufferQueue<structureType*, QUEUE_INIT_SIZE> queue;

		public:
		// Queue operations
		bool PushTail(structureType* s) { return queue.PushTail(s); }


		bool PopHead(structureType*& s)
		{
			return queue.PopHead(s);
		}

		bool IsEmpty(void) { return queue.IsEmpty(); }

		void RemoveAtIndex(unsigned int position)
		{
			queue.RemoveAtIndex(position);
		}

		unsigned int Size(void) { return queue.Size(); }

		// Memory pool operations
		structureType* Allocate()
		{
			structureType *s;
			s = memoryPool.Allocate();
			// Call new operator, memoryPool doesn't do this
			s = new ( (void*) s ) structureType;
			return s;
		}

		/// Call delete operator, memory pool doesn't do this
		void Deallocate(structureType* s)
		{
			s->~structureType();
			memoryPool.Reclaim(s);
		}

		/// reset queue and release memory in pool
		void Clear()
		{
			for( unsigned int i = 0; i < queue.Size(); i++ )
			{
				queue[i]->~structureType();
				memoryPool.Reclaim(queue[i]);
			}
			queue.Clear();
			memoryPool.Clear();
		}
	};

	/// will not  call dtor and ctor for each of element, faster
	template <typename structureType,
		unsigned int MEMPOOL_BLOCKS_COUNT_PER_PAGE = 256,
		unsigned int MEMORY_POOL_MAX_FREE_PAGES = 4,
		unsigned int QUEUE_INIT_SIZE = 32>
	class JACKIE_EXPORT MemPoolAllocRingBufferQueue
	{
		protected:
		mutable MemoryPool<structureType, MEMPOOL_BLOCKS_COUNT_PER_PAGE, MEMORY_POOL_MAX_FREE_PAGES> memoryPool;

		RingBufferQueue<structureType*, QUEUE_INIT_SIZE> queue;

		public:
		// Queue operations
		bool PushTail(structureType* s) { return queue.PushTail(s); }


		bool PopHead(structureType*& s)
		{
			return queue.PopHead(s);
		}

		bool IsEmpty(void) { return queue.IsEmpty(); }

		void RemoveAtIndex(unsigned int position)
		{
			queue.RemoveAtIndex(position);
		}

		unsigned int Size(void) { return queue.Size(); }

		// Memory pool operations
		structureType* Allocate()
		{
			structureType *s;
			s = memoryPool.Allocate();
			return s;
		}

		/// Call delete operator, memory pool doesn't do this
		void Deallocate(structureType* s)
		{
			memoryPool.Reclaim(s);
		}

		/// reset queue and release memory in pool
		void Clear()
		{
			for( unsigned int i = 0; i < queue.Size(); i++ )
			{
				memoryPool.Reclaim(queue[i]);
			}
			queue.Clear();
			memoryPool.Clear();
		}
	};
}
#endif