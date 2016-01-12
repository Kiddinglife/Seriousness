/// circular linked list
#ifndef __DoubleLinkedList_H__
#define __DoubleLinkedList_H__

#include "DLLExport.h"
#include "OverrideMemory.h"

namespace DataStructures
{
	template <class LinkedListType>
	class  LinkedList;

	/// will dynalically allocate list node by new
	template <class data_type>
	class JACKIE_EXPORT JackieDoubleLinkedList
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
		JackieDoubleLinkedList Merge(JackieDoubleLinkedList& L1, JackieDoubleLinkedList& L2)
		{
			JackieDoubleLinkedList X;
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
					L1.Remove();
				}
				else
				{
					element = L2.root->item;
					L2.Remove();
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
		JackieDoubleLinkedList Mergesort(const JackieDoubleLinkedList& space)
		{
			unsigned int counter;
			unsigned int list_size = space.list_size;
			ListNode* location = space.root;

			JackieDoubleLinkedList L1;
			JackieDoubleLinkedList L2;

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
			if (leftSubListLeft == 0) leftSubListLeft = space.root;
			if (leftSubListRight == 0) leftSubListRight = space.root->next;
			if (count == 0) count = space.list_size;

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
		JackieDoubleLinkedList()
		{
			root = position = 0; list_size = 0;
		}
		virtual ~JackieDoubleLinkedList()
		{
			Clear();
		}
		JackieDoubleLinkedList(const JackieDoubleLinkedList& original_copy)
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
		JackieDoubleLinkedList operator= (const JackieDoubleLinkedList& original_copy)
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
		JackieDoubleLinkedList& operator++()
		{
			if (this->list_size != 0) position = position->next;
			return *this;
		}
		// Circular_Linked List A; A++;
		JackieDoubleLinkedList& operator++(int)
		{
			return this->operator++();
		}
		/// CircularLinkedList A; --A;
		JackieDoubleLinkedList& operator--()
		{
			if (this->list_size != 0) this->position = this->position->previous;
			return *this;
		}
		// Circular_Linked List A; A--;
		JackieDoubleLinkedList& operator--(int)
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
		/// Add new element in bebind of cursor, cursor always points to the root node without change
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
				return new_node->item;
			}
		}
		inline void Replace(const data_type& input)
		{
			if (this->list_size > 0)
				this->position->item = input;
		}
		void Remove(void)
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
		/// remove the element if it exists
		void Remove(const data_type& input)
		{
			if (Find(input)) Remove();
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
			Remove();
			return element; // return temporary
		}
		void Pop(data_type& data)
		{
			data = Peek();
			Remove();
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
		void Concatenate(const JackieDoubleLinkedList& space)
		{
			if (space.list_size == 0) return;
			if (this->list_size == 0) *this = space;

			ListNode* ptr = space.root;
			this->position = this->root->previous;

			// Cycle through each element in space and add it to the current list
			for (unsigned int counter = 0; counter < space.list_size; counter++)
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
		inline JackieDoubleLinkedList& operator<<(const JackieDoubleLinkedList& space)
		{
			Concatenate(space);
			return *this;
		}

		ListNode * Root() const { return root; }
		ListNode * Cursor() const { return position; }
		void Cursor(ListNode * val) { position = val; }
	};

	template <class LinkedListType>
	class JACKIE_EXPORT LinkedList : public JackieDoubleLinkedList<LinkedListType>
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
		LinkedList Mergesort(const LinkedList& space);
	};

	/// Usually should use this
	/// have all functions same to CircularList and LinkedList, 
	///but faster because no need to allocate new node by new call 
	/// and never free  allocated memory to avoid memory fragments
	/// on the runtime.
	template <class ElemType, unsigned int MAXSIZE = 32>
	class JACKIE_EXPORT StaticCircularLinkedList
	{
	private:
		int size;
		int position; /// cursor
		int position_array_index; /// cursor's array index
		/// 若备用空间链表非空，则返回分配的结点下标，否则返回0 
		inline int Malloc_SSL(void)
		{
			int i = space[0].cur; /// 当前数组第一个元素的cur存的值  就是要返回的第一个备用空闲的下标 
			if (space[0].cur) space[0].cur = space[i].cur;  /// 由于要拿出一个分量来使用了，所以我们就得把它的下一个 分量用来做备用 
			return i;
		}

		/// 将下标为k的空闲结点回收到备用链表 
		inline void Free_SSL(int k)
		{
			space[k].cur = space[0].cur;    /// 把第一个元素的cur值赋给要删除的分量cur */
			space[0].cur = k;               /// 把要删除的分量下标赋值给第一个元素的cur */
		}

		/// return index of array
		/// @param [out] linkedIndex the index of linked list
		/// for example, 
		/// e =2, A[2]=1 -> A[5]=2 -> A[0]=3, will return 5, 
		/// linkedIndex will be assigned as 2
		int FindPointer(const ElemType& e, int& linkedIndex)
		{
			/// empty pointer
			if (size == 0)
			{
				linkedIndex = 0;
				return 0;
			}

			linkedIndex = 1;
			int i = space[MAXSIZE - 1].cur;
			while (i)
			{
				if (space[i].data == e)
				{
					return i;
				}
				i = space[i].cur;
				linkedIndex++;
			}

			return 0;
		}

	public:
		struct ListNode
		{
			ElemType data;
			int cur; /* 0 means no element to be pointed to */
		};

		ListNode* space;

		StaticCircularLinkedList()
		{
			space = JACKIE_INET::OP_NEW_ARRAY<ListNode>(MAXSIZE, TRACE_FILE_AND_LINE_);
			for (int i = 0; i < MAXSIZE - 1; i++)
			{
				space[i].cur = i + 1;
			}
			size = space[MAXSIZE - 1].cur = 0; /// 目前静态链表为空，最后一个元素的cur为0 
			position = 1;
			position_array_index = 1;
		}
		virtual ~StaticCircularLinkedList()
		{
			JACKIE_INET::OP_DELETE_ARRAY(space, TRACE_FILE_AND_LINE_);
		}

		/// 初始条件：静态链表space已存在。操作结果：返回space中数据元素个数 */
		inline int Size(void)
		{
			return size;
		}

		/// @param [in] int & i is the order of linked elements, 
		/// not the order they are stored in array, 
		/// will be reassigned to the inserted element array index
		/// @param [in] const ElemType & e
		/// @brief insert @praram e before the element with linked index of @param i
		//bool Insert(int &i, const ElemType& e)
		bool Insert(int i, const ElemType& e)
		{
			if (i < 1 || i > Size() + 1) return false;

			int j = Malloc_SSL();   /// 获得空闲分量的下标 
			if (j)
			{
				int k = MAXSIZE - 1;   /// 注意k首先是最后一个元素的下标 
				space[j].data = e;   /// 将数据赋值给此分量的data 
				for (int l = 1; l <= i - 1; l++)  { k = space[k].cur; } /// 找到第i个元素之前的位置 
				space[j].cur = space[k].cur;  /// 把第i个元素之前的cur赋值给新元素的cur 
				space[k].cur = j; /// 把新元素的下标赋值给第i个元素之前元素的ur 
				if (size > 0 && i <= position)
					position++;
				size++;
				//i = j;
				return true;
			}
			return false;
		}
		/// insert new element in front of cursor
		inline bool Insert(const ElemType& input)
		{
			return Insert(position, input);
		}

		/// Add new element in bebind of cursor
		inline bool Add(const ElemType& input)
		{
			return Insert(position + 1, input);
		}

		/// 删除在space中第i个数据元素 
		bool RemoveInternal(int i)
		{
			//i++;
			if (i < 1 || i > Size()) return false;

			int k = MAXSIZE - 1;
			int j = 1;
			for (; j <= i - 1; j++){ k = space[k].cur; }
			j = space[k].cur;
			space[k].cur = space[j].cur;
			size--;
			if (space[j].cur == 0)
			{
				position = size;
				position_array_index = k;
			}
			else
			{
				position--;
				position_array_index = space[j].cur;
			}
			Free_SSL(j);
			return true;
		}
		/// remove the element at position, 
		/// the  position now points to the elemrnt after the removed one
		inline bool Remove()
		{
			return RemoveInternal(position);
		}
		/// remove last elment
		inline bool RemoveAtLast(void)
		{
			return RemoveInternal(size);
		}
		/// remove the element if it exists
		inline bool Remove(const ElemType& input)
		{
			if (Find(input))
				return Remove();
			else
				return false;
		}
		bool Has(const ElemType& input)
		{
			int i = 0;
			if (FindPointer(input, i) != 0 && i)
				return true;
			else
				return false;
		}

		/// position will be changed to point to the found element
		int Find(const ElemType& input)
		{
			int pos;
			int return_value = FindPointer(input, pos);
			if (return_value != 0 && pos)
			{
				position = pos;
				position_array_index = return_value;
				return position;
			}
			return 0; // Can't find the item don't do anything
		}

		/// return the cursor item
		inline ElemType& Peek(void)
		{
			return space[position_array_index].data;
		}
		/// return and remove the cursor item
		ElemType Pop(void)
		{
			ElemType element = Peek();
			Remove();
			return element; // return temporary
		}
		bool Pop(ElemType& data)
		{
			data = Peek();
			return Remove();
		}
		void Print()
		{
			printf("ArrayCircularList Elements(%d):\n", Size());
			int j = 0;
			int i = space[MAXSIZE - 1].cur;
			while (i)
			{
				printf("%d ", space[i]);
				i = space[i].cur;
				j++;
			}
			printf("\n");
		}
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
* - concatenate(list space): This appends space to the current list
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