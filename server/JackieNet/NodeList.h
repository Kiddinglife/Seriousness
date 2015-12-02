/// circular linked list
#ifndef __NODE_LIST_H__
#define __NODE_LIST_H__

#include "DLLExport.h"
#include "OverrideMemory.h"

namespace DataStructures
{
	template <class LinkedListType>
	class JACKIE_EXPORT LinkedList;

	template <class data_type>
	class CircularList
	{
	private:
		struct ListNode
		{
			data_type item;
			ListNode* previous;
			ListNode* next;
		};

	protected:
		unsigned int list_size;
		ListNode *root;
		ListNode *position;
		ListNode* FindPointer(const data_type& input);
    
	private:
		CircularList Merge(CircularList L1, CircularList L2);
		CircularList Mergesort(const CircularList& L);

	public:
		CircularList();
		~CircularList();
		CircularList(const CircularList& original_copy);
		// CircularLinkedList(LinkedList<CircularLinkedListType> original_copy) {CircularLinkedList(original_copy);}  // Converts linked list to circular type
		bool operator= (const CircularList& original_copy);
		CircularList& operator++();  // CircularLinkedList A; ++A;
		CircularList& operator++(int);  // Circular_Linked List A; A++;
		CircularList& operator--();  // CircularLinkedList A; --A;
		CircularList& operator--(int);  // Circular_Linked List A; A--;
		bool IsIn(const data_type& input);
		bool Find(const data_type& input);
		void Insert(const data_type& input);
		data_type& Add(const data_type& input); 
		// Adds after the current position
		void Replace(const data_type& input);
		void Del(void);

		unsigned int Size(void);
		data_type& Peek(void);
		data_type Pop(void);
		void Clear(void);
		void Sort(void);
		void Beginning(void);
		void End(void);
		void Concatenate(const CircularList& L);
	};
}

#endif 

/**
* \brief (Circular) Linked List ADT (Doubly Linked Pointer to Node Style) -
*
* \details
* Initilize with the following command
* LinkedList<TYPE>
* OR
* CircularLinkedList<Type>
*
* Has the following member functions
* - size: returns number of elements in the linked list
* - insert(item):  inserts @em item at the current position in
*   the LinkedList.
* - add(item): inserts @em item after the current position in
*   the LinkedList.  Does not increment the position
* - replace(item): replaces the element at the current position @em item.
* - peek:  returns the element at the current position
* - pop:  returns the element at the current position and deletes it
* - del: deletes the current element. Does nothing for an empty list.
* - clear:  empties the LinkedList and returns storage
* - bool IsInitem): Does a linear search for @em item.  Does not set
*   the position to it, only returns true on item found, false otherwise
* - bool find(item): Does a linear search for @em item and sets the current
*   position to point to it if and only if the item is found. Returns true
*   on item found, false otherwise
* - sort: Sorts the elements of the list with a mergesort and sets the
*   current pointer to the first element
* - concatenate(list L): This appends L to the current list
* - ++(prefix): moves the pointer one element up in the list and returns the
*   appropriate copy of the element in the list
* - --(prefix): moves the pointer one element back in the list and returns
*   the appropriate copy of the element in the list
* - beginning - moves the pointer to the start of the list.  For circular
*   linked lists this is first 'position' created.  You should call this
*   after the sort function to read the first value.
* - end - moves the pointer to the end of the list.  For circular linked
*   lists this is one less than the first 'position' created
* The assignment and copy constructor operators are defined
*
* \note
* 1. LinkedList and CircularLinkedList are exactly the same except LinkedList
*    won't let you wrap around the root and lets you jump to two positions
*    relative to the root/
* 2. Postfix ++ and -- can be used but simply call the prefix versions.
*
*
* EXAMPLE:
* @code
* LinkedList<int> A;  // Creates a Linked List of integers called A
* CircularLinkedList<int> B;  // Creates a Circular Linked List of
*          // integers called B
*
* A.Insert(20);  // Adds 20 to A.  A: 20 - current is 20
* A.Insert(5);  // Adds 5 to A.  A: 5 20 - current is 5
* A.Insert(1);  // Adds 1 to A.  A: 1 5 20 - current is 1
*
* A.IsIn1); // returns true
* A.IsIn200); // returns false
* A.Find(5);  // returns true and sets current to 5
* A.Peek();  // returns 5
* A.Find(1);  // returns true and sets current to 1
*
* (++A).Peek();  // Returns 5
* A.Peek(); // Returns 5
*
* A.Replace(10);  // Replaces 5 with 10.
* A.Peek();  // Returns 10
*
* A.Beginning();  // Current points to the beginning of the list at 1
*
* (++A).Peek();  // Returns 5
* A.Peek();  // Returns 10
*
* A.Del();  // Deletes 10.  Current points to the next element, which is 20
* A.Peek();  // Returns 20
*
* A.Beginning();  // Current points to the beginning of the list at 1
*
* (++A).Peek();  // Returns 5
* A.Peek();  // Returns 20
*
* A.Clear(_FILE_AND_LINE_);  // Deletes all nodes in A
*
* A.Insert(5);  // A: 5 - current is 5
* A.Insert(6); // A: 6 5 - current is 6
* A.Insert(7); // A: 7 6 5 - current is 7
*
* A.Clear(_FILE_AND_LINE_);
* B.Clear(_FILE_AND_LINE_);
*
* B.Add(10);
* B.Add(20);
* B.Add(30);
* B.Add(5);
* B.Add(2);
* B.Add(25);
* // Sorts the numbers in the list and sets the current pointer to the
* // first element
* B.sort();
*
* // Postfix ++ just calls the prefix version and has no functional
* // difference.
* B.Peek();  // Returns 2
* B++;
* B.Peek();  // Returns 5
* B++;
* B.Peek();  // Returns 10
* B++;
* B.Peek();  // Returns 20
* B++;
* B.Peek();  // Returns 25
* B++;
* B.Peek();  // Returns 30
* @endcode
*/