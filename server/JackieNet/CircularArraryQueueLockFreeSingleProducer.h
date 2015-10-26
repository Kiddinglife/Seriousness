// ============================================================================
// Copyright (c) 2010 Faustino Frechilla
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright 
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//  3. The name of the author may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
/// @file array_lock_free_queue_single_producer.h
/// @brief Definition of a circular array based lock-free queue
///
/// WARNING: This queue is not thread safe when several threads try to insert 
///          elements into the queue. It is allowed to use as many consumers
///          as needed though.
///
/// @author Faustino Frechilla
/// @history
/// Ref  Who                 When         What
///      Faustino Frechilla  11-Jul-2010  Original development
/// @endhistory
/// 
// ============================================================================

#ifndef __ARRAY_LOCK_FREE_QUEUE_SINGLE_PRODUCER_H__
#define __ARRAY_LOCK_FREE_QUEUE_SINGLE_PRODUCER_H__

#include <assert.h> // assert()
#include "BasicTypes.h"
#include "atomic_ops.h"
#include "JACKIE_Atomic.h"

#define ARRAY_LOCK_FREE_Q_DEFAULT_SIZE 256 // 2^16 = 65,536 elements by default

// define this variable if calls to "size" must return the real size of the 
// queue. If it is undefined  that function will try to take a snapshot of 
// the queue, but returned value might be bogus
#undef ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE
//#define ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE 1


/// @brief especialisation of the ArrayLockFreeQueue 
/// to be used when there is only one producer thread
/// No allocation of extra memory for the nodes handling is needed
///
/// WARNING: This queue is not thread safe when several threads try 
/// to insert elements  into the queu
///
/// @ELEM_T     represents the type of elementes pushed and popped from the queue
/// @TOTAL_SIZE size of the queue. It should be a power of 2 to ensure 
/// indexes in the circular queue keep stable when the UInt32 
/// variable that holds the current position rolls over from FFFFFFFF
///            to 0. For instance
///            2    -> 0x02 
///            4    -> 0x04
///            8    -> 0x08
///            16   -> 0x10
///            (...) 
///            1024 -> 0x400
///            2048 -> 0x800
///
///            if queue size is not defined as requested, let's say, for
///            instance 100, when current position is FFFFFFFF (4,294,967,295)
///            index in the circular array is 4,294,967,295 % 100 = 95. 
///            When that value is incremented it will be set to 0, that is the 
///            last 4 elements of the queue are not used when the counter rolls
///            over to 0

template <typename ELEM_T, UInt32 Q_SIZE = ARRAY_LOCK_FREE_Q_DEFAULT_SIZE>
class CircularArraryQueueLockFreeSingleProducer
{
	public:
	/// @brief constructor of the class
	CircularArraryQueueLockFreeSingleProducer();
	virtual ~CircularArraryQueueLockFreeSingleProducer();

	/// @brief returns the current number of items in the queue
	/// It tries to take a snapshot of the size of the queue, but in busy environments
	/// this function might return bogus values. 
	/// If a reliable queue size must be kept you might want to have a look at 
	/// the preprocessor variable in this header file called 'ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE'
	/// it enables a reliable size though it hits overall performance of the queue 
	/// (when the reliable size variable is on it's got an impact of about 20% in time)
	UInt32 size();

	/// @brief push an element at the tail of the queue
	/// @param the element to insert in the queue
	/// Note that the element is not a pointer or a reference, so if you are using large data
	/// structures to be inserted in the queue you should think of instantiate the template
	/// of the queue as a pointer to that large structure
	/// @returns true if the element was inserted in the queue. False if the queue was full
	bool push(const ELEM_T &a_data);

	/// @brief pop the element at the head of the queue
	/// @param a reference where the element in the head of the queue will be saved to
	/// Note that the a_data parameter might contain rubbish if the function returns false
	/// @returns true if the element was successfully extracted from the queue. False if the queue was empty
	bool pop(ELEM_T &a_data);

	private:
	/// @brief array to keep the elements
	ELEM_T m_theQueue[Q_SIZE];

#ifdef ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE
	/// @brief number of elements in the queue
	volatile UInt32 m_count;
	//JACKIE_ATOMIC_LONG m_count;
#endif

	/// @brief where a new element will be inserted
	volatile UInt32 m_writeIndex;

	/// @brief where the next element where be extracted from
	volatile UInt32 m_readIndex;

	/// @brief calculate the index in the circular array that corresponds
	/// to a particular "count" value
	inline UInt32 countToIndex(UInt32 a_count);
};

template <typename ELEM_T, UInt32 Q_SIZE>
CircularArraryQueueLockFreeSingleProducer<ELEM_T, Q_SIZE>::CircularArraryQueueLockFreeSingleProducer() :
m_writeIndex(0),
m_readIndex(0)
#ifdef ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE
,m_count(0)
#endif
{
}

template <typename ELEM_T, UInt32 Q_SIZE>
CircularArraryQueueLockFreeSingleProducer<ELEM_T, Q_SIZE>::~CircularArraryQueueLockFreeSingleProducer()
{
}

template <typename ELEM_T, UInt32 Q_SIZE>
inline
UInt32 CircularArraryQueueLockFreeSingleProducer<ELEM_T, Q_SIZE>::countToIndex(UInt32 a_count)
{
	// if Q_SIZE is a power of 2 this statement could be also written as 
	// return (a_count & (Q_SIZE - 1));
	return ( a_count % Q_SIZE );
}

template <typename ELEM_T, UInt32 Q_SIZE>
UInt32 CircularArraryQueueLockFreeSingleProducer<ELEM_T, Q_SIZE>::size()
{
#ifdef ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE
	return m_count;
#else
	UInt32 currentWriteIndex = m_writeIndex;
	UInt32 currentReadIndex = m_readIndex;

	// let's think of a scenario where this function returns bogus data
	// 1. when the statement 'currentWriteIndex = m_writeIndex' is run
	// m_writeIndex is 3 and m_readIndex is 2. Real size is 1
	// 2. afterwards this thread is preemted. While this thread is inactive 2 
	// elements are inserted and removed from the queue, so m_writeIndex is 5
	// m_readIndex 4. Real size is still 1
	// 3. Now the current thread comes back from preemption and reads m_readIndex.
	// currentReadIndex is 4
	// 4. currentReadIndex is bigger than currentWriteIndex, so
	// m_totalSize + currentWriteIndex - currentReadIndex is returned, that is,
	// it returns that the queue is almost full, when it is almost empty

	if( currentWriteIndex >= currentReadIndex )
	{
		return ( currentWriteIndex - currentReadIndex );
	} else
	{
		return ( Q_SIZE + currentWriteIndex - currentReadIndex );
	}
#endif // ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE
}

template <typename ELEM_T, UInt32 Q_SIZE>
bool CircularArraryQueueLockFreeSingleProducer<ELEM_T, Q_SIZE>::push(const ELEM_T &a_data)
{
	UInt32 currentReadIndex;
	UInt32 currentWriteIndex;

	currentWriteIndex = m_writeIndex;
	currentReadIndex = m_readIndex;
	if( countToIndex(currentWriteIndex + 1) ==
		countToIndex(currentReadIndex) )
	{
		// the queue is full
		return false;
	}

	// save the date into the q
	m_theQueue[countToIndex(currentWriteIndex)] = a_data;

	// No need to increment write index atomically. It is a 
	// a requierement of this queue that only one thred can push stuff in
	m_writeIndex++;

	// The value was successfully inserted into the queue
#ifdef ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE
	AtomicAdd(&m_count, 1);
	
#endif

	return true;
}

template <typename ELEM_T, UInt32 Q_SIZE>
bool CircularArraryQueueLockFreeSingleProducer<ELEM_T, Q_SIZE>::pop(ELEM_T &a_data)
{
	UInt32 currentMaximumReadIndex;
	UInt32 currentReadIndex;

	do
	{
		// m_maximumReadIndex doesn't exist when the queue is set up as
		// single-producer. The maximum read index is described by the current
		// write index
		currentReadIndex = m_readIndex;
		currentMaximumReadIndex = m_writeIndex;

		if( countToIndex(currentReadIndex) ==
			countToIndex(currentMaximumReadIndex) )
		{
			// the queue is empty or
			// a producer thread has allocate space in the queue but is 
			// waiting to commit the data into it
			return false;
		}

		// retrieve the data from the queue
		a_data = m_theQueue[countToIndex(currentReadIndex)];

		// try to perfrom now the CAS operation on the read index. If we succeed
		// a_data already contains what m_readIndex pointed to before we 
		// increased it
		if( CAS(&m_readIndex, currentReadIndex, ( currentReadIndex + 1 )) )
		{
			// got here. The value was retrieved from the queue. Note that the
			// data inside the m_queue array is not deleted nor reseted
#ifdef ARRAY_LOCK_FREE_Q_KEEP_REAL_SIZE
			AtomicSub(&m_count, 1);
#endif
			return true;
		}

		// it failed retrieving the element off the queue. Someone else must
		// have read the element stored at countToIndex(currentReadIndex)
		// before we could perform the CAS operation        

	} while( 1 ); // keep looping to try again!

	// Something went wrong. it shouldn't be possible to reach here
	assert(0);

	// Add this return statement to avoid compiler warnings
	return false;
}

#endif // __ARRAY_LOCK_FREE_QUEUE_SINGLE_PRODUCER_H__
