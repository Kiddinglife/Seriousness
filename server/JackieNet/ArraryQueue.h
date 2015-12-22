#ifndef __Arrary_Queue_H__
#define __Arrary_Queue_H__

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
	template <typename queue_type, unsigned int QUEUE_INIT_SIZE = 32>
	class JACKIE_EXPORT ArraryQueue
	{
	private:
		queue_type* queueArrary;
		unsigned int head;  // Array index for the head of the queue
		unsigned int tail; // Array index for the tail of the queue
		unsigned int allocation_size;

	public:
		ArraryQueue()
		{
			allocation_size = head = tail = 0;
			queueArrary = 0;
		}
		~ArraryQueue()
		{
			if (allocation_size > 0)
				JACKIE_INET::OP_DELETE_ARRAY(queueArrary, TRACE_FILE_AND_LINE_);

			queueArrary = 0;
			allocation_size = head = tail = 0;
		}
		ArraryQueue(ArraryQueue& original_copy)
		{
			// Allocate memory for copy
			if (original_copy.Size() == 0)
			{
				allocation_size = 0; return;
			}

			//if( allocation_size < original_copy.Size() )
			//{
			//if( allocation_size > 0 )
			//	JACKIE_INET::OP_DELETE_ARRAY(array, TRACE_FILE_AND_LINE_);
			queueArrary = JACKIE_INET::OP_NEW_ARRAY<queue_type >(original_copy.Size() + 1, TRACE_FILE_AND_LINE_);
			//allocation_size = original_copy.Size() + 1;
			//}
			for (unsigned int counter = 0; counter < original_copy.Size(); ++counter)
				queueArrary[counter] = original_copy.queueArrary[(original_copy.head + counter) % (original_copy.allocation_size)];

			head = 0;
			tail = original_copy.Size();
			allocation_size = original_copy.Size() + 1;
		}

		bool operator= (const ArraryQueue& original_copy)
		{
			if ((&original_copy) == this) return false;

			Clear(TRACE_FILE_AND_LINE_);

			// Allocate memory for copy
			if (original_copy.Size() == 0)
			{
				allocation_size = 0; // THIS MAY CAUSE MEMORY LEAK !
				return;
			}

			queueArrary = JACKIE_INET::OP_NEW_ARRAY<queue_type >(original_copy.Size() + 1, TRACE_FILE_AND_LINE_);

			for (unsigned int counter = 0; counter < original_copy.Size(); ++counter)
				queueArrary[counter] = original_copy.queueArrary[(original_copy.head + counter) % (original_copy.allocation_size)];

			head = 0;
			tail = original_copy.Size();
			allocation_size = original_copy.Size() + 1;
			return true;
		}

		/// enqueue at tail of queue
		bool PushTail(const queue_type& input)
		{
			if (allocation_size == 0)
			{
				queueArrary = JACKIE_INET::OP_NEW_ARRAY<queue_type>(
					QUEUE_INIT_SIZE, TRACE_FILE_AND_LINE_);

				assert(queueArrary != 0);
				if (queueArrary == 0)
				{
					notifyOutOfMemory(TRACE_FILE_AND_LINE_);
					return false;
				}

				head = 0;
				tail = 1;
				queueArrary[0] = input;
				allocation_size = QUEUE_INIT_SIZE;
				return true;

			}

			queueArrary[tail++] = input;
			if (tail == allocation_size) tail = 0;
			if (tail == head)
			{
				/// queue gets full and need to allocate more memory.
				queue_type * new_array =
					JACKIE_INET::OP_NEW_ARRAY<queue_type>(allocation_size << 1,
					TRACE_FILE_AND_LINE_);

				assert(new_array != 0);
				if (new_array == 0) return false;

				/// copy old values into new array
				for (unsigned int counter = 0; counter < allocation_size; ++counter)
					new_array[counter] = queueArrary[(head + counter) % (allocation_size)];

				/// update stats
				head = 0;
				tail = allocation_size;
				allocation_size <<= 1;

				// Delete the old array and move the pointer to the new array
				JACKIE_INET::OP_DELETE_ARRAY(queueArrary, TRACE_FILE_AND_LINE_);
				queueArrary = new_array;
			}
			return true;
		}
		bool PopHead(queue_type& ele)
		{
#ifdef _DEBUG
			assert(head != tail);
#endif // _DEBUG
			if (head == tail) return false;

			if (++head == allocation_size) head = 0;
			if (head == 0) ele = queueArrary[allocation_size - 1];
			ele = queueArrary[head - 1];
			return true;
		}

		/// enqueue at the front of queue that is slow operation
		bool InsertAtIndex(const queue_type& input, unsigned index)
		{
			assert(index <= Size());

			// Just force a reallocation, will be overwritten
			if (!PushTail(input)) return false;
			if (Size() == 1) return false;

			/// move all elments after index
			unsigned int writeIndex, readIndex, trueWriteIndex, trueReadIndex;
			writeIndex = Size() - 1;
			readIndex = writeIndex - 1;

			while (readIndex >= index)
			{
				if (head + writeIndex >= allocation_size)
					trueWriteIndex = head + writeIndex - allocation_size;
				else
					trueWriteIndex = head + writeIndex;

				if (head + readIndex >= allocation_size)
					trueReadIndex = head + readIndex - allocation_size;
				else
					trueReadIndex = head + readIndex;

				queueArrary[trueWriteIndex] = queueArrary[trueReadIndex];

				if (readIndex == 0)
					break;
				writeIndex--;
				readIndex--;
			}

			if (head + index >= allocation_size)
				trueWriteIndex = head + index - allocation_size;
			else
				trueWriteIndex = head + index;

			queueArrary[trueWriteIndex] = input;
			return true;
		}
		/// Not a normal thing you do with a queue but can be used for efficiency
		/// will move all elements after the removed elements- slow operation
		void RemoveAtIndex(unsigned int position)
		{
			assert(position < Size());
			assert(head != tail);

			/// queue is empty or position overflow the array we just return
			if (head == tail || position >= Size()) return;

			unsigned int index;
			unsigned int next;

			if (head + position >= allocation_size)
				index = head + position - allocation_size;
			else
				index = head + position;

			next = index + 1;
			if (next == allocation_size) next = 0;

			while (next != tail)
			{
				// Overwrite the previous element
				queueArrary[index] = queueArrary[next];
				index = next;
				if (++next == allocation_size) next = 0;
			}

			// Move the tail back
			tail == 0 ? tail = allocation_size - 1 : --tail;
		}

		// Not a normal thing you do with a queue but can be used for efficiency
		queue_type& operator[] (unsigned int position) const
		{
#ifdef _DEBUG
			assert(position < Size());
#endif // _DEBUG
			return head + position >= allocation_size ?
				queueArrary[head + position - allocation_size] :
				queueArrary[head + position];
		}

		/// pop will update head and tail, overhead of deleting
		/// but peek only return the value without updating head and tail
		queue_type Head(void) const
		{
#ifdef _DEBUG
			assert(head != tail);
#endif // _DEBUG
			return queueArrary[head];
		}
		queue_type Tail(void) const
		{
#ifdef _DEBUG
			assert(head != tail);
#endif // _DEBUG
			return tail != 0 ? queueArrary[tail - 1] : queueArrary[allocation_size - 1];
		}

		bool PopTail(queue_type& ele)
		{
#ifdef _DEBUG
			assert(head != tail);
#endif // _DEBUG
			if (head == tail) return false;

			if (tail != 0)
			{
				--tail;
				ele = queueArrary[tail];
			}
			else
			{
				tail = allocation_size - 1;
				ele = queueArrary[tail];
			}

			return true;
		}
		// Debug: Set pointer to 0, for memory leak detection
		bool PopDeref(queue_type& ele)
		{
			if (++head == allocation_size) head = 0;

			if (head == 0)
			{
				ele = queueArrary[allocation_size - 1];
				queueArrary[allocation_size - 1] = 0;
				return true;
			}

			ele = queueArrary[head - 1];
			queueArrary[head - 1] = 0;

			return true;
		}

		unsigned int Size(void) const
		{
			return head <= tail ? tail - head : allocation_size - head + tail;
		}
		bool IsEmpty(void) const { return head == tail; }
		unsigned int AllocationSize(void) const { return allocation_size; }

		/// Free memory if > QUEUE_INIT_SIZE
		void Clear(void)
		{
			if (allocation_size == 0) return;
			if (allocation_size > QUEUE_INIT_SIZE)
			{
				JACKIE_INET::OP_DELETE_ARRAY(queueArrary, TRACE_FILE_AND_LINE_);
				allocation_size = 0;
				queueArrary = 0;
			}

			head = 0;
			tail = 0;
		}

		/// shrink the size of queue to the minimum size that can still alll the elements
		/// used to decrease the use of memory
		void Shrink2MiniSzie(void)
		{
			if (allocation_size == 0) return;

			// Must be a better way to do this but I'm too dumb to figure it out quickly :)
			unsigned int newAllocationSize = 1;
			while (newAllocationSize <= Size())
				newAllocationSize <<= 1;

			queue_type* new_array = JACKIE_INET::OP_NEW_ARRAY<queue_type >(newAllocationSize, TRACE_FILE_AND_LINE_);

			for (unsigned int counter = 0; counter < Size(); ++counter)
				new_array[counter] = queueArrary[(head + counter) % (allocation_size)];

			tail = Size();
			allocation_size = newAllocationSize;
			head = 0;

			// Delete the old array and move the pointer to the new array
			JACKIE_INET::OP_DELETE_ARRAY(queueArrary, TRACE_FILE_AND_LINE_);
			queueArrary = new_array;
		}

		/// Loop all elements in this queue to see if it is in this queue
		bool Contains(const queue_type& q)
		{
			if (allocation_size == 0) return false;
			unsigned int counter = head;
			while (counter != tail)
			{
				if (queueArrary[counter] == q) return true;
				counter = (counter + 1) % allocation_size;
			}
			return false;
		}

		/// Force a memory allocation to a certain larger size
		void Resize(int size)
		{
			JACKIE_INET::OP_DELETE_ARRAY(queueArrary, TRACE_FILE_AND_LINE_);

			if (size > 0)
				queueArrary = JACKIE_INET::OP_NEW_ARRAY<queue_type>(size, TRACE_FILE_AND_LINE_);
			else
				queueArrary = 0;

			allocation_size = size;
			head = 0;
			tail = 0;
		}
	};
}
#endif