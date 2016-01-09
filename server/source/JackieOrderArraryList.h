#ifndef __ORDER_ARRARY_LIST_H__
#define __ORDER_ARRARY_LIST_H__

#include "JackieArrayList.h"

/// The namespace DataStructures was only added to avoid compiler errors for commonly named data structures
/// As these data structures are stand-alone, you can use them outside of RakNet for your own projects if you wish.
namespace DataStructures
{
	template<class a, class b>
	int DefaultOrderArrayListComparsionFunc(const a& va, const b& vb)
	{
		if (va < vb) return -1; if (va == vb) return 0; if (va > vb) return 1;
	}

	///@note IMPORTANT! If you use DefaultOrderArrayListComparsionFunc then call IMPLEMENT_DEFAULT_COMPARISON or you will get an unresolved external linker error.
	template <class key_type, class data_type,
		int(*default_comparison_function)(const key_type&, const data_type&) = DefaultOrderArrayListComparsionFunc<key_type, data_type> >
	class JACKIE_EXPORT JackieOrderArraryList
	{
	protected:
		JackieArrayList<data_type> orderedList;

	public:
		static void IMPLEMENT_DEFAULT_COMPARISON(void)
		{
			DefaultOrderArrayListComparsionFunc<key_type, data_type>(key_type(),
				data_type());
		}

		JackieOrderArraryList(){}
		virtual ~JackieOrderArraryList(){ Clear(); }

		JackieOrderArraryList(const JackieOrderArraryList& original_copy)
		{
			orderedList = original_copy.orderedList;
		}

		JackieOrderArraryList& operator= (const JackieOrderArraryList& original_copy)
		{
			orderedList = original_copy.orderedList;
			return *this;
		}

		/// comparisonFunction must take a key_type and a data_type 
		/// and return <0, ==0, or >0. If the data type has comparison
		/// operators already defined then you can just use defaultComparison
		inline bool Exists(const key_type &key, int(*cf)(const key_type&, const data_type&) = default_comparison_function) const
		{
			bool objectExists;
			GetIndexFromKey(key, objectExists, cf);
			return objectExists;
		}

		/// GetIndexFromKey returns where the insert should go at the 
		/// same time checks if it is there
		unsigned int GetIndexFromKey(const key_type &key, bool& objectExists,
			int(*cf)(const key_type&, const data_type&) = default_comparison_function)
			const
		{
			if (orderedList.Size() == 0)
			{
				objectExists = false;
				return 0;
			}

			int res;
			int index = orderedList.Size() >>1;
			int upperBound = orderedList.Size() - 1;
			int lowerBound = 0;

			while (true)
			{
				res = cf(key, orderedList[index]);
				if (res > 0)
					lowerBound = index + 1;
				else if (res < 0)
					upperBound = index - 1;
				else /// if (res == 0)
				{
					objectExists = true;
					return (unsigned)index;
				}

				index = lowerBound + (upperBound - lowerBound) >> 1;
				if (lowerBound > upperBound)
				{
					objectExists = false;
					return (unsigned)lowerBound; // No match
				}
#ifdef _DEBUG
				if (index < 0 || index >= (int)orderedList.Size())
				{
					// This should never hit unless the comparison function was inconsistent
					assert(index && 0);
					objectExists = false;
					return 0;
				}
#endif // _DEBUG
			}
		}

		data_type GetElementFromKey(const key_type &key, int(*cf)(const key_type&, const data_type&) = default_comparison_function) const
		{
			bool objectExists;
			unsigned int index = GetIndexFromKey(key, objectExists, cf);
#ifdef _DEBUG
			assert(objectExists == true);
#endif // _DEBUG
			return orderedList[index];
		}

		bool GetElementFromKey(const key_type &key, data_type &element,
			int(*cf)(const key_type&, const data_type&) = default_comparison_function) const
		{
			bool objectExists;
			unsigned int index = GetIndexFromKey(key, objectExists, cf);
			if (objectExists) element = orderedList[index];
			return objectExists;
		}

		unsigned int Insert(const key_type &key, const data_type &data, bool assertOnDuplicate,
			int(*cf)(const key_type&, const data_type&) = default_comparison_function)
		{
			(void)assertOnDuplicate;
			bool objectExists;
			unsigned index = GetIndexFromKey(key, objectExists, cf);

			// Don't allow duplicate insertion.
			if (objectExists)
			{
				// This is usually a bug!
				assert(assertOnDuplicate == false);
				return (unsigned)-1;
			}

			if (index >= orderedList.Size())
			{
				orderedList.InsertAtLast(data);
				return orderedList.Size() - 1;
			}
			else
			{
				orderedList.InsertAtIndex(data, index);
				return index;
			}
		}

		unsigned int Remove(const key_type &key, int(*cf)(const key_type&, const data_type&) = default_comparison_function)
		{
			bool objectExists;
			unsigned int index = GetIndexFromKey(key, objectExists, cf);

			// Can't find the element to remove if this assert hits
			if (!objectExists)
			{
				assert(objectExists == true);
				return 0;
			}
			orderedList.RemoveAtIndex(index);
			return index;
		}

		int RemoveIfExists(const key_type &key, int(*cf)(const key_type&, const data_type&) = default_comparison_function)
		{
			bool objectExists;
			unsigned int index = GetIndexFromKey(key, objectExists, cf);

			// Can't find the element to remove if this assert hits
			if (!objectExists)
			{
				return 0;
			}

			orderedList.RemoveAtIndex(index);
			return index;
		}

		inline data_type& operator[] (const unsigned int position) const
		{
			return orderedList[position];
		}

		inline void RemoveAtIndex(const unsigned index)
		{
			orderedList.RemoveAtIndex(index);
		}

		inline void InsertAtIndex(const data_type &data, const unsigned index)
		{
			orderedList.InsertAtIndex(data, index);
		}

		inline void InsertAtLast(const data_type &data)
		{
			orderedList.InsertAtLast(data);
		}

		inline void RemoveFromLast(const unsigned num = 1)
		{
			orderedList.RemoveFromLast(num);
		}

		inline void Clear(bool doNotDeallocate = false)
		{
			orderedList.Clear(doNotDeallocate);
		}

		inline unsigned Size(void) const
		{
			return orderedList.Size();
		}
	};
}

#endif