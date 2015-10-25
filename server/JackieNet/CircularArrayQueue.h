#ifndef Address_H_
#define Address_H_

#include <cassert>
#include "DLLExport.h"
#include "OverrideMemory.h"

/// The namespace DataStructures was only added to avoid compiler errors for commonly
/// named data structures  As these data structures are stand-alone, you can use them 
/// outside  of JACKIE_INet for your own projects if you wish.
namespace DataStructures
{
	/// \brief A queue implemented as an array with a read and write index.
	template <class queue_type> class JACKIE_EXPORT CircularArrayQueue
	{
		private:
		queue_type* array;
		unsigned int head;  // Array index for the head of the queue
		unsigned int tail; // Array index for the tail of the queue
		unsigned int allocation_size;

		public:
		CircularArrayQueue()
		{
			allocation_size = head = tail = 0;
			array = 0;
		}
		~CircularArrayQueue()
		{
			if( allocation_size > 0 )
				JACKIE_INET::OP_DELETE_ARRAY(array, TRACE_FILE_AND_LINE_);
		}
		CircularArrayQueue(CircularArrayQueue& original_copy);

		bool operator= ( const CircularArrayQueue& original_copy );

		void Push(const queue_type& input, const char *file, unsigned int line);
		void PushAtHead(const queue_type& input, unsigned index, const char *file, unsigned int line);

		// Not a normal thing you do with a queue but can be used for efficiency
		queue_type& operator[] (unsigned int position) const
		{
			assert(position < Size());
			return head + position >= allocation_size ?
				array[head + position - allocation_size] :
				array[head + position];
		}

		/// Not a normal thing you do with a queue but can be used for efficiency
		void RemoveAtIndex(unsigned int position);

		queue_type Peek(void) const { assert(head != tail); return array[head]; }
		queue_type PeekTail(void) const
		{
			assert(head != tail);
			return tail != 0 ? array[tail - 1] : array[allocation_size - 1];
		}
		queue_type Pop(void)
		{
			assert(head != tail);
			if( ++head == allocation_size ) head = 0;
			if( head == 0 ) return  array[allocation_size - 1];
			return array[head - 1];
		}
		queue_type PopTail(void)
		{
			assert(head != tail);
			if( tail != 0 )
			{
				--tail;
				return array[tail];
			} else
			{
				tail = allocation_size - 1;
				return array[tail];
			}
		}
		// Debug: Set pointer to 0, for memory leak detection
		queue_type PopDeref(void)
		{
			if( ++head == allocation_size )
				head = 0;

			queue_type& q;
			if( head == 0 )
			{
				q = array[allocation_size - 1];
				array[allocation_size - 1] = 0;
				return q;
			}

			q = array[head - 1];
			array[head - 1] = 0;
			return q;
		}

		unsigned int Size(void) const
		{
			return head <= tail ? tail - head : allocation_size - head + tail;
		}
		bool IsEmpty(void) const { return head == tail; }
		unsigned int AllocationSize(void) const { return allocation_size; }
		void Clear(const char *file, unsigned int line);

		void Compress(const char *file, unsigned int line);

		bool Find(const queue_type& q);
		void ClearAndForceAllocation(int size, const char *file, unsigned int line); // Force a memory allocation to a certain larger size
	};
}
#endif