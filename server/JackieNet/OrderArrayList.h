#ifndef __ORDER_ARRARY_LIST_H__
#define __ORDER_ARRARY_LIST_H__

#include "ArrayList.h"

/// The namespace DataStructures was only added to avoid compiler errors for commonly named data structures
/// As these data structures are stand-alone, you can use them outside of RakNet for your own projects if you wish.
namespace DataStructures
{
	template<class a, class b>
	int DefaultOrderArrayListComparsionFunc(const a& va, const b& vb)
	{
		if (va < vb) return -1; if (va == vb) return 0; if (va > vb) return 1;
	}

	/// \note IMPORTANT! If you use DefaultOrderArrayListComparsionFunc then call IMPLEMENT_DEFAULT_COMPARISON or you will get an unresolved external linker error.
	template <class key_type, class data_type,
		int(*default_comparison_function)(const key_type&, const data_type&) = DefaultOrderArrayListComparsionFunc<key_type, data_type> >
	class JACKIE_EXPORT OrderArrayList
	{
	public:
		static void IMPLEMENT_DEFAULT_COMPARISON(void)
		{
			DefaultOrderArrayListComparsionFunc<key_type, data_type>(key_type(),
				data_type());
		}

		OrderArrayList(){}
		virtual ~OrderArrayList(){ Clear(); }

		OrderArrayList(const OrderArrayList& original_copy)
		{
			orderedList = original_copy.orderedList;
		}

		OrderArrayList& operator= (const OrderArrayList& original_copy)
		{
			orderedList = original_copy.orderedList;
			return *this;
		}

		/// comparisonFunction must take a key_type and a data_type 
		/// and return <0, ==0, or >0. If the data type has comparison
		/// operators already defined then you can just use defaultComparison
		inline bool Exists(const key_type &key, int(*cf)(const key_type&, const data_type&) = default_comparison_function) const
		{
			return GetIndexFromKey(key, cf) > -1;
		}

		/// GetIndexFromKey returns where the insert should go at the 
		/// same time checks if it is there
		int GetIndexFromKey(const key_type &key,
			int(*cf)(const key_type&, const data_type&) = default_comparison_function)
			const
		{
			if (orderedList.Size() == 0)
			{
				return -1;
			}

			int res;
			int index = orderedList.Size() / 2;
			int upperBound = = orderedList.Size() - 1;
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
					return index;
				}

				index = lowerBound + (upperBound - lowerBound) / 2;
				if (lowerBound > upperBound)
				{
					return -1;
				}
#ifdef _DEBUG
				if (index < 0 || index >= (int)orderedList.Size())
				{
					// This should never hit unless the comparison function was inconsistent
					assert(index && 0);
					return -1;
				}
#endif // _DEBUG
			}
		}

		data_type GetElementFromKey(const key_type &key, int(*cf)(const key_type&, const data_type&) = default_comparison_function) const
		{
			int index = GetIndexFromKey(key, cf);
#ifdef _DEBUG
			assert(index > -1);
#endif // _DEBUG
			return orderedList[index];
		}

		bool GetElementFromKey(const key_type &key, data_type &element, int(*cf)(const key_type&, const data_type&) = default_comparison_function) const
		{
			int index = GetIndexFromKey(key, cf);
			if (index > -1)
			{
				element = orderedList[index];
				return true;
			}
			else
				return false;
		}

		int Insert(const key_type &key, const data_type &data, bool assertOnDuplicate,
			int(*cf)(const key_type&, const data_type&) = default_comparison_function)
		{
			(void)assertOnDuplicate;
			int index = GetIndexFromKey(key, cf);

			// Don't allow duplicate insertion.
			if (index > -1)
			{
				// This is usually a bug!
				assert(assertOnDuplicate == false);
				return -1;
			}

			if (index >= (int)orderedList.Size())
			{
				orderedList.InsertAtLast(data);
				return orderedList.Size() - 1;
			}
			else
			{
				orderedList.InsertAtIndex(data, (unsigned int)index);
				return index;
			}
		}

		int Remove(const key_type &key, int(*cf)(const key_type&, const data_type&) = default_comparison_function)
		{
			int index = GetIndexFromKey(key, cf);

			// Can't find the element to remove if this assert hits
			if (index == -1)
			{
				assert(index > -1);
				return -1;
			}
			orderedList.RemoveAtIndex((unsigned int)index);
			return index;
		}

		int RemoveIfExists(const key_type &key, int(*cf)(const key_type&, const data_type&) = default_comparison_function)
		{
			int index = GetIndexFromKey(key, cf);

			// Can't find the element to remove if this assert hits
			if (index == -1)
			{
				assert(index > -1);
				return -1;
			}

			orderedList.RemoveAtIndex((unsigned int)index);
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

	protected:
		ArrayList<data_type> orderedList;
	};
}

#endif