#ifndef SingleThreadRingBufferQueue_H_
#define SingleThreadRingBufferQueue_H_

#include <cassert>
#include "DLLExport.h"
#include "OverrideMemory.h"
//#include "../JackieNetUnitTests/980easylogging++.h"

/// The namespace DataStructures was only added to avoid compiler errors for commonly
/// named data structures  As these data structures are stand-alone, you can use them 
/// outside  of JACKIE_INet for your own projects if you wish.
namespace DataStructures
{
	/// \brief A queue implemented as an array with a read and write index.
	template <typename queue_type, UInt32 QUEUE_INIT_SIZE = 256>
	class JACKIE_EXPORT SingleThreadRingBufferQueue
	{
		private:
		queue_type* array;
		unsigned int head;  // Array index for the head of the queue
		unsigned int tail; // Array index for the tail of the queue
		unsigned int allocation_size;

		public:
		SingleThreadRingBufferQueue()
		{
			allocation_size = head = tail = 0;
			array = 0;
		}
		~SingleThreadRingBufferQueue()
		{
			if( allocation_size > 0 )
				JACKIE_INET::OP_DELETE_ARRAY(array, TRACE_FILE_AND_LINE_);

			array = 0;
			allocation_size = head = tail = 0;
		}
		SingleThreadRingBufferQueue(SingleThreadRingBufferQueue& original_copy)
		{
			// Allocate memory for copy
			if( original_copy.Size() == 0 )
			{
				allocation_size = 0; return;
			}

			//if( allocation_size < original_copy.Size() )
			//{
			//if( allocation_size > 0 )
			//	JACKIE_INET::OP_DELETE_ARRAY(array, TRACE_FILE_AND_LINE_);
			array = JACKIE_INET::OP_NEW_ARRAY<queue_type >(original_copy.Size() + 1, TRACE_FILE_AND_LINE_);
			//allocation_size = original_copy.Size() + 1;
			//}
			for( unsigned int counter = 0; counter < original_copy.Size(); ++counter )
				array[counter] = original_copy.array[( original_copy.head + counter ) % ( original_copy.allocation_size )];

			head = 0;
			tail = original_copy.Size();
			allocation_size = original_copy.Size() + 1;
		}

		bool operator= ( const SingleThreadRingBufferQueue& original_copy )
		{
			if( ( &original_copy ) == this ) return false;

			Clear(TRACE_FILE_AND_LINE_);

			// Allocate memory for copy
			if( original_copy.Size() == 0 )
			{
				allocation_size = 0; // THIS MAY CAUSE MEMORY LEAK !
				return;
			}

			array = JACKIE_INET::OP_NEW_ARRAY<queue_type >(original_copy.Size() + 1, TRACE_FILE_AND_LINE_);

			for( unsigned int counter = 0; counter < original_copy.Size(); ++counter )
				array[counter] = original_copy.array[( original_copy.head + counter ) % ( original_copy.allocation_size )];

			head = 0;
			tail = original_copy.Size();
			allocation_size = original_copy.Size() + 1;
			return true;
		}

		void PushTail(const queue_type& input, const char *file, unsigned int line)
		{
			if( allocation_size == 0 )
			{
				array = JACKIE_INET::OP_NEW_ARRAY<queue_type>(QUEUE_INIT_SIZE, file, line);
				head = 0; tail = 1; array[0] = input; allocation_size = QUEUE_INIT_SIZE;
				return;
			}

			array[tail++] = input;
			if( tail == allocation_size ) tail = 0;
			if( tail == head )
			{
				/// queue gets full and need to allocate more memory.
				queue_type * new_array =
					JACKIE_INET::OP_NEW_ARRAY<queue_type>(allocation_size << 1,
					file, line);

				assert(new_array != 0);
				if( new_array == 0 ) return;

				/// copy old values into new array
				for( unsigned int counter = 0; counter < allocation_size; ++counter )
					new_array[counter] = array[( head + counter ) % ( allocation_size )];

				/// update stats
				head = 0;
				tail = allocation_size;
				allocation_size <<= 1;

				// Delete the old array and move the pointer to the new array
				JACKIE_INET::OP_DELETE_ARRAY(array, file, line);
				array = new_array;
			}
		}
		void PushHead(const queue_type& input, unsigned index, const char *file, unsigned int line)
		{
			assert(index <= Size());
			// Just force a reallocation, will be overwritten
			PushTail(input, file, line);
			if( Size() == 1 ) return;

			/// move all elments after index
			unsigned int writeIndex, readIndex, trueWriteIndex, trueReadIndex;
			writeIndex = Size() - 1;
			readIndex = writeIndex - 1;

			while( readIndex >= index )
			{
				if( head + writeIndex >= allocation_size )
					trueWriteIndex = head + writeIndex - allocation_size;
				else
					trueWriteIndex = head + writeIndex;

				if( head + readIndex >= allocation_size )
					trueReadIndex = head + readIndex - allocation_size;
				else
					trueReadIndex = head + readIndex;

				array[trueWriteIndex] = array[trueReadIndex];

				if( readIndex == 0 )
					break;
				writeIndex--;
				readIndex--;
			}

			if( head + index >= allocation_size )
				trueWriteIndex = head + index - allocation_size;
			else
				trueWriteIndex = head + index;

			array[trueWriteIndex] = input;
		}

		// Not a normal thing you do with a queue but can be used for efficiency
		queue_type& operator[] (unsigned int position) const
		{
			assert(position < Size());
			return head + position >= allocation_size ?
				array[head + position - allocation_size] :
				array[head + position];
		}

		/// Not a normal thing you do with a queue but can be used for efficiency
		void RemoveAtIndex(unsigned int position)
		{
			assert(position < Size());
			assert(head != tail);

			/// queue is empty or position overflow the array we just return
			if( head == tail || position >= Size() ) return;

			unsigned int index;
			unsigned int next;

			if( head + position >= allocation_size )
				index = head + position - allocation_size;
			else
				index = head + position;

			next = index + 1;
			if( next == allocation_size ) next = 0;

			while( next != tail )
			{
				// Overwrite the previous element
				array[index] = array[next];
				index = next;
				if( ++next == allocation_size ) next = 0;
			}

			// Move the tail back
			tail == 0 ? tail = allocation_size - 1 : --tail;
		}

		/// pop will update head and tail, overhead of deleting
		/// but peek only return the value without updating head and tail
		queue_type PeekHead(void) const { assert(head != tail); return array[head]; }
		queue_type PeekTail(void) const
		{
			assert(head != tail);
			return tail != 0 ? array[tail - 1] : array[allocation_size - 1];
		}
		queue_type PopHead(void)
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
		void Clear(const char *file, unsigned int line)
		{
			if( allocation_size == 0 ) return;
			if( allocation_size > QUEUE_INIT_SIZE )
			{
				JACKIE_INET::OP_DELETE_ARRAY(array, file, line);
				allocation_size = 0;
				array = 0;
			}

			head = 0;
			tail = 0;
		}

		/// shrink the size of queue to the minimum size that can still alll the elements
		/// used to decrease the use of memory
		void Shrink2MiniSzie(const char *file, unsigned int line)
		{
			if( allocation_size == 0 ) return;

			// Must be a better way to do this but I'm too dumb to figure it out quickly :)
			unsigned int newAllocationSize = 1;
			while( newAllocationSize <= Size() )
				newAllocationSize <<= 1;

			queue_type* new_array = JACKIE_INET::OP_NEW_ARRAY<queue_type >(newAllocationSize, file, line);

			for( unsigned int counter = 0; counter < Size(); ++counter )
				new_array[counter] = array[( head + counter ) % ( allocation_size )];

			tail = Size();
			allocation_size = newAllocationSize;
			head = 0;

			// Delete the old array and move the pointer to the new array
			JACKIE_INET::OP_DELETE_ARRAY(array, file, line);
			array = new_array;
		}

		/// Loop all elements in this queue to see if it is in this queue
		bool Contains(const queue_type& q)
		{
			if( allocation_size == 0 ) return false;
			unsigned int counter = head;
			while( counter != tail )
			{
				if( array[counter] == q ) return true;
				counter = ( counter + 1 ) % allocation_size;
			}
			return false;
		}

		// Force a memory allocation to a certain larger size
		void Resize(int size, const char *file, unsigned int line)
		{
			JACKIE_INET::OP_DELETE_ARRAY(array, file, line);

			if( size > 0 )
				array = JACKIE_INET::OP_NEW_ARRAY<queue_type>(size, file, line);
			else
				array = 0;

			allocation_size = size;
			head = 0;
			tail = 0;
		}
	};
}
#endif