/// circular linked list
#ifndef __DoubleLinkedList_H__
#define __DoubleLinkedList_H__

#include "DLLExport.h"
#include "OverrideMemory.h"

namespace DataStructures
{
	template <class LinkedListType>
	class  LinkedList;

	template <class data_type>
	class JACKIE_EXPORT CircularList
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
		ListNode *position; /// cursor
		ListNode* FindPointer(const data_type& input)
		{
			if (list_size == 0)
			{
				return 0;
			}

			/// Search for the item starting from the root node and 
			/// incrementing the pointer after every check
			/// If you wind up pointing at the root again you looped 
			/// around the list so didn't find the item, in which case return 0
			ListNode* current = root;
			do
			{
				if (current->item == input) return current;
				current = current->next;
			} while (current != root);
			return 0;
		}

	private:
		CircularList Merge(CircularList& L1, CircularList& L2)
		{
			CircularList X;
			data_type element;
			L1.position = L1.root;
			L2.position = L2.root;
			while ((L1.list_size != 0) && (L2.list_size != 0))
			{
				// Compare the first items of L1 and L2
				// Remove the smaller of the two items from the list
				if (L1.root->item < L2.root->item)
				{
					element = L1.root->item;
					L1.Del();
				}
				else
				{
					element = L2.root->item;
					L2.Del();
				}

				// Add this item to the end of X
				X.Add(element);
				X++;
			}

			// Add the remaining list to X
			if (L1.list_size != 0)
				X.Concatenate(L1);
			else
				X.Concatenate(L2);

			return X;
		}
		CircularList Mergesort(const CircularList& L)
		{
			unsigned int counter;
			unsigned int list_size = L.list_size;
			ListNode* location = L.root;

			CircularList L1;
			CircularList L2;

			// Split the list into two equal size sublists, L1 and L2
			for (counter = 0; counter < list_size / 2; counter++)
			{
				L1.Add(location->item);
				location = location->next;
			}

			for (; counter < list_size; counter++)
			{
				L2.Add(location->item);
				location = location->next;
			}

			//Recursively sort the sublists
			if (L1.list_size > 1) L1 = Mergesort(L1);
			if (L2.list_size > 1) L2 = Mergesort(L2);

			// Merge the two sublists
			return Merge(L1, L2);
		}

		void MergeFixed(const ListNode*& left, const ListNode*& right,
			unsigned int left_size, unsigned int right_size)
		{
			// Compare the first items of L1 and L2
			// change link
			//if (left == mid == right)
			//	return left;
			unsigned int count = right_size;
			ListNode* rightSubListLeft = right;
			while (count > 1)
			{
				rightSubListLeft = right->previous;
				count--;
			}

			count = left_size;
			ListNode* leftSubListRight = left;
			while (count > 1)
			{
				leftSubListRight = left->next;
				count--;
			}

			if (left_size == right_size == 1 && left->item <= right->item)
			{
				return;
			}

			ListNode* Leftt = left;
			ListNode* subListLeft = 0;
			while (left_size != 0 && right_size != 0)
			{
				if (left->item >= right->item)
				{
					if (subListLeft == 0)
						subListLeft = right;

					/// update linkages
					left->previous->next = right;
					


					right = right->next;
					right_size--;
				}
				else
				{
					left = left->next;
					left_size--;
				}
			}
			left = Leftt;
		}

		void MergeSortFixed(const ListNode* leftSubListLeft = 0,
			const ListNode* leftSubListRight = 0,
			unsigned int count = 0)
		{
			if (leftSubListLeft == 0) leftSubListLeft = L.root;
			if (leftSubListRight == 0) leftSubListRight = L.root->next;
			if (count == 0) count = L.list_size;

			ListNode* rightSubListRight = leftSubListRight;
			// Split the list into two equal size sublists, left and right
			unsigned int rightSubListSize;
			for (rightSubListSize = 0; rightSubListSize < count / 2; rightSubListSize++)
			{
				leftSubListRight = leftSubListRight->previous;
			}

			//Recursively sort the sublists
			//if (left->next != right || right->previous != left)
			//	MergeSortFixed(left, right, count - i);
			unsigned int leftSubListSize = count - rightSubListSize;
			if (leftSubListLeft != leftSubListRight && leftSubListSize > 1)
			{
				MergeSortFixed(leftSubListLeft, leftSubListRight, leftSubListSize);
			}

			//if (right->next->next != r || r->previous != right->next || right->next != r)
			//	MergeSortFixed(left, right, i);

			ListNode* rightSubListLeft = leftSubListRight->next;
			if (leftSubListLeft != leftSubListRight && rightSubListSize > 1)
			{
				MergeSortFixed(rightSubListLeft, rightSubListRight, rightSubListSize);
			}

			// Merge the two sublists
			MergeFixed(leftSubListLeft, rightSubListRight, leftSubListSize, rightSubListSize);
		}
	public:
		CircularList()
		{
			root = position = 0; list_size = 0;
		}
		virtual ~CircularList()
		{
			Clear();
		}
		CircularList(const CircularList& original_copy)
		{
			if (original_copy.list_size == 0)
			{
				root = 0;
				position = 0;
				list_size = 0;
			}
			else if (original_copy.list_size == 1)
			{
				root = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
				root->next = root;
				root->previous = root;
				list_size = 1;
				position = root;
				root->item = original_copy.root->item;
			}
			else
			{
				ListNode *save_position = 0;
				ListNode *last;

				/// Setup the first part of the root node
				ListNode * original_copy_pointer = original_copy.root;

				root = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
				position = root;
				root->item = original_copy.root->item;

				if (original_copy_pointer == original_copy.position)
					save_position = position;

				do
				{
					/// Save the current element
					last = position;
					/// Point to the next node in the source list
					original_copy_pointer = original_copy_pointer->next;
					/// Create a new node and point position to it
					position = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
					/// Copy the item to the new node
					position->item = original_copy_pointer->item;
					/// find the current position from original copy
					if (original_copy_pointer == original_copy.position)
						save_position = position;
					/// Set the previous pointer for the new node
					position->previous = last;
					/// Set the next pointer for the old node to the new node
					last->next = position;
				} while (original_copy_pointer->next != original_copy.root);

				/// Complete the circle.  
				/// Set the next pointer of the newest node to the root 
				/// and the previous pointer of the root to the newest node
				position->next = root;
				root->previous = position;
				list_size = original_copy.list_size;
				position = save_position;
#ifdef _DEBUG
				assert(position != 0);
#endif // _DEBUG
			}
		}
		// CircularLinkedList(LinkedList<CircularLinkedListType> original_copy) {CircularLinkedList(original_copy);}  // Converts linked list to circular type
		CircularList operator= (const CircularList& original_copy)
		{
			if (this != &original_copy)
			{
				Clear();

				if (original_copy.list_size == 0)
				{
					root = 0;
					position = 0;
					list_size = 0;
				}
				else if (original_copy.list_size == 1)
				{
					root = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
					root->next = root;
					root->previous = root;
					list_size = 1;
					position = root;
					root->item = original_copy.root->item;
				}
				else
				{
					ListNode *save_position = 0;
					ListNode *last;

					/// Setup the first part of the root node
					ListNode * original_copy_pointer = original_copy.root;

					root = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
					position = root;
					root->item = original_copy.root->item;

					if (original_copy_pointer == original_copy.position)
						save_position = position;

					do
					{
						/// Save the current element
						last = position;
						/// Point to the next node in the source list
						original_copy_pointer = original_copy_pointer->next;
						/// Create a new node and point position to it
						position = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
						/// Copy the item to the new node
						position->item = original_copy_pointer->item;
						/// find the current position from original copy
						if (original_copy_pointer == original_copy.position)
							save_position = position;
						/// Set the previous pointer for the new node
						position->previous = last;
						/// Set the next pointer for the old node to the new node
						last->next = position;
					} while (original_copy_pointer->next != original_copy.root);

					/// Complete the circle.  
					/// Set the next pointer of the newest node to the root 
					/// and the previous pointer of the root to the newest node
					position->next = root;
					root->previous = position;
					list_size = original_copy.list_size;
					position = save_position;
#ifdef _DEBUG
					assert(position != 0);
#endif // _DEBUG
				}
			}

			return *this;
		}
		/// CircularLinkedList A; ++A;
		CircularList& operator++()
		{
			if (this->list_size != 0) position = position->next;
			return *this;
		}
		// Circular_Linked List A; A++;
		CircularList& operator++(int)
		{
			return this->operator++();
		}
		/// CircularLinkedList A; --A;
		CircularList& operator--()
		{
			if (this->list_size != 0) this->position = this->position->previous;
			return *this;
		}
		// Circular_Linked List A; A--;
		CircularList& operator--(int)
		{
			return this->operator--();
		}
		inline bool Has(const data_type& input)
		{
			if (FindPointer(input) != 0)
				return true;
			else
				return false;
		}
		/// position will be changed to point to the found element
		bool Find(const data_type& input)
		{
			ListNode * return_value = FindPointer(input);
			if (return_value != 0)
			{
				position = return_value;
				return true;
			}
			else
				return false; // Can't find the item don't do anything
		}
		/// insert new element in front of cursor
		void Insert(const data_type& input)
		{
			if (list_size == 0)
			{
				root = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
				root->item = input;
				root->next = root;
				root->previous = root;
				list_size = 1;
				position = root;
			}
			else if (list_size == 1)
			{
				position = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
				root->next = position;
				root->previous = position;
				position->previous = root;
				position->next = root;
				position->item = input;
				root = position;
				// Since we're inserting into a 1 element list the old root is now the second item
				list_size = 2;
			}
			else
			{
				ListNode* new_node = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
				new_node->item = input;
				position->previous->next = new_node;
				new_node->previous = position->previous;
				position->previous = new_node;
				new_node->next = position;

				/// Since the root pointer is bound to a node rather than an index
				/// this moves it back if you insert an element at the root
				if (position == root)
				{
					root = new_node;
					position = root;
				}

				// Increase the recorded size of the list by one
				list_size++;
			}
		}
		/// Add new element in bebind of cursor, cursor points to the root node
		data_type& Add(const data_type& input)
		{
			if (this->list_size == 0)
			{
				this->root = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
				this->root->item = input;
				this->root->next = this->root;
				this->root->previous = this->root;
				this->list_size = 1;
				this->position = this->root;
				return this->position->item;
			}
			else if (list_size == 1)
			{
				this->position = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
				this->root->next = this->position;
				this->root->previous = this->position;
				this->position->previous = this->root;
				this->position->next = this->root;
				this->position->item = input;
				this->list_size = 2;
				this->position = this->root; // Don't move the position from the root
				return this->position->item;
			}
			else
			{
				ListNode* new_node = JACKIE_INET::OP_NEW<ListNode>(TRACE_FILE_AND_LINE_);
				new_node->item = input;
				// Point last of B to A
				new_node->previous = this->position;
				// Point next of B to C
				new_node->next = (this->position->next);
				(this->position->next)->previous = new_node;
				// Point next of A to B
				(this->position->next) = new_node;
				// Increase the recorded size of the list by one
				this->list_size++;
				// return *(new_node->item);
				return new_node->item;
			}
		}
		inline void Replace(const data_type& input)
		{
			if (this->list_size > 0)
				this->position->item = input;
		}
		void Del(void)
		{
			if (this->list_size == 0)
				return;
			else if (list_size == 1)
			{
				JACKIE_INET::OP_DELETE(this->root, TRACE_FILE_AND_LINE_);
				root = position = 0;
				list_size = 0;
			}
			else
			{
				position->previous->next = position->next;
				position->next->previous = position->previous;
				ListNode * new_position = position->next;
				if (position == root)
					root = new_position;
				JACKIE_INET::OP_DELETE(position, TRACE_FILE_AND_LINE_);
				position = new_position;
				list_size--;
			}
		}
		inline unsigned int Size(void)
		{
			return list_size;
		}
		/// return the cursor item
		inline data_type& Peek(void)
		{
			return position->item;
		}
		/// return and del the cursor item
		data_type Pop(void)
		{
			data_type element = Peek();
			Del();
			return element; // return temporary
		}
		void Pop(data_type& data)
		{
			data = Peek();
			Del();
		}
		/// free memory and reset to empty
		void Clear(void)
		{
			if (list_size == 0)
			{
				return;
			}
			else if (list_size == 1)
			{
				JACKIE_INET::OP_DELETE(root, TRACE_FILE_AND_LINE_);
			}
			else
			{
				ListNode* current = root;
				ListNode* temp;
				do
				{
					temp = current;
					current = current->next;
					JACKIE_INET::OP_DELETE(temp, TRACE_FILE_AND_LINE_);
				} while (current != root);
			}

			root = position = 0;
			list_size = 0;
		}
		void Sort(void)
		{
			if (this->list_size <= 1) return;
			// Call equal operator to assign result of mergesort to current object
			*this = Mergesort(*this);
			this->position = this->root;
		}
		/// move cursor to root
		inline void Beginning(void)
		{
			if (this->root != 0) this->position = this->root;
		}
		/// move cursor to end
		inline void End(void)
		{
			if (this->root != 0) this->position = this->root->previous;
		}
		void Concatenate(const CircularList& L)
		{
			if (L.list_size == 0) return;
			if (this->list_size == 0) *this = L;

			ListNode* ptr = L.root;
			this->position = this->root->previous;

			// Cycle through each element in L and add it to the current list
			for (unsigned int counter = 0; counter < L.list_size; counter++)
			{
				// Add item after the current item pointed to
				Add(ptr->item);

				// Update pointers.  
				/// Moving ptr keeps the current pointer at the end of the 
				/// list since the add function does not move the pointer
				ptr = ptr->next;
				this->position = this->position->next;
			}
		}
		inline CircularList& operator<<(const CircularList& L)
		{
			Concatenate(L);
			return *this;
		}

		ListNode * Root() const { return root; }
		ListNode * Cursor() const { return position; }
		void Cursor(ListNode * val) { position = val; }
	};

	template <class LinkedListType>
	class JACKIE_EXPORT LinkedList : public CircularList<LinkedListType>
	{
	public:
		LinkedList(){}

		LinkedList(const LinkedList& original_copy);
		virtual ~LinkedList();
		bool operator= (const LinkedList<LinkedListType>& original_copy);
		LinkedList& operator++();  // LinkedList A; ++A;
		LinkedList& operator++(int);  // Linked List A; A++;
		LinkedList& operator--();  // LinkedList A; --A;
		LinkedList& operator--(int);  // Linked List A; A--;

	private:
		LinkedList Merge(LinkedList L1, LinkedList L2);
		LinkedList Mergesort(const LinkedList& L);
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