#ifndef __ORDER__LIST_MAP_H__
#define __ORDER__LIST_MAP_H__

#include "OrderArrayList.h"
#include "DLLExport.h"
#include "OverrideMemory.h"

/// If I want to change this to a red-black tree, this is a good site: 
/// http://www.cs.auckland.ac.nz/software/AlgAnim/red_black.html
/// This makes insertions and deletions faster.  
/// But then traversals are slow, while they are currently fast.
namespace DataStructures
{
	/// The default comparison has to be first so it can be called as a default parameter.
	/// It then is followed by MapNode, followed by NodeComparisonFunc
	template <class key_type>
	extern int defaultMapKeyComparison(const key_type &a, const key_type &b)
	{
		if (a < b) return -1; if (a == b) return 0; return 1;
	}

	/// \note IMPORTANT! If you use defaultMapKeyComparison then call IMPLEMENT_DEFAULT_COMPARISON or you will get an unresolved external linker error.
	template <class key_type, class data_type,
		int(*key_comparison_func)(const key_type&, const key_type&) = defaultMapKeyComparison<key_type> >
	class JACKIE_EXPORT OrderListMap
	{
	public:
		static void IMPLEMENT_DEFAULT_COMPARISON(void)
		{
			defaultMapKeyComparison<key_type>(key_type(), key_type());
		}

	private:
		struct Node
		{
			key_type nodeKey;
			data_type nodeData;

			Node(){}
			Node(key_type _key, data_type _data) :
				nodeKey(_key), nodeData(_data) {}

			Node(const Node & input)
			{
				nodeKey = input.nodeKey;
				nodeData = input.nodeData;
			}

			Node& operator = (const Node& input)
			{
				nodeKey = input.nodeKey;
				nodeData = input.nodeData;
				return *this;
			}
		};

		/// Has to be a static because the comparison callback 
		/// for DataStructures::OrderedList is a C function
		static int NodeComparisonFunc(const key_type &a, const Node &b)
		{
			return key_comparison_func(a, b.nodeKey);
		}

	protected:
		OrderArrayList< key_type, Node, &OrderListMap::NodeComparisonFunc > mapNodeList;
		unsigned int lastSearchIndex;
		key_type lastSearchKey;
		bool lastSearchIndexValid;

	public:
		OrderListMap() : lastSearchIndexValid(false){}
		~OrderListMap() { Clear(); }
		OrderListMap(const OrderListMap& original_copy)
		{
			mapNodeList = original_copy.mapNodeList;
			lastSearchIndex = original_copy.lastSearchIndex;
			lastSearchKey = original_copy.lastSearchKey;
			lastSearchIndexValid = original_copy.lastSearchIndexValid;
		}
		OrderListMap& operator= (const OrderListMap& original_copy)
		{
			if (this != &original_copy)
			{
				mapNodeList = original_copy.mapNodeList;
				lastSearchIndex = original_copy.lastSearchIndex;
				lastSearchKey = original_copy.lastSearchKey;
				lastSearchIndexValid = original_copy.lastSearchIndexValid;
			}
			return *this;
		}
		inline void Clear(void)
		{
			lastSearchIndexValid = false;
			mapNodeList.Clear(false);
		}

		data_type& Get(const key_type &key) const
		{
			bool objExists;
			unsigned index = mapNodeList.GetIndexFromKey(key, objExists);
#ifdef _DEBUG
			assert(objExists == true);
#endif // _DEBUG
			return mapNodeList[index].nodeData;
		}

		data_type Pop(const key_type &key)
		{
			bool objextExists;
			unsigned index = mapNodeList.GetIndexFromKey(key, objextExists);
#ifdef _DEBUG
			assert(objextExists == true);
#endif // _DEBUG
			data_type tmp = mapNodeList[index].nodeData;
			mapNodeList.RemoveAtIndex(index);
			lastSearchIndexValid = false;
			return tmp;
		}

		bool Pop(const key_type &key, data_type& data)
		{
			bool objextExists;
			unsigned index = mapNodeList.GetIndexFromKey(key, objextExists);
			if (!objextExists)
				return false;

			data = mapNodeList[index].nodeData;
			mapNodeList.RemoveAtIndex(index);
			lastSearchIndexValid = false;
			return true;
		}

		// Add if needed
		void Set(const key_type &key, const data_type &data)
		{
			bool objectExists;
			unsigned index = mapNodeList.GetIndexFromKey(key, objectExists);

			objectExists ? mapNodeList[index].nodeData = data :
				mapNodeList.Insert(key, Node(key, data), true);
		}

		// Must already exist
		void SetExisting(const key_type &key, const data_type &data)
		{
			bool objectExists;
			unsigned index = mapNodeList.GetIndexFromKey(key, objectExists);
			if (!objectExists) return;
			mapNodeList[index].nodeData = data;
		}
		// Must add
		void SetNew(const key_type &key, const data_type &data)
		{
#ifdef _DEBUG
			bool objectExists;
			mapNodeList.GetIndexFromKey(key, objectExists);
			assert(objectExists == false);
#endif // _DEBUG
			mapNodeList.Insert(key, Node(key, data), true);
		}

		bool Has(const key_type &key) const
		{
			bool objectExists;
			mapNodeList.GetIndexFromKey(key, objectExists);
			return objectExists;
		}
		bool Delete(const key_type &key)
		{
			bool objectExists;
			unsigned index = mapNodeList.GetIndexFromKey(key, objectExists);
			if (objectExists)
			{
				lastSearchIndexValid = false;
				mapNodeList.RemoveAtIndex(index);
				return true;
			}
			else
			{
				return false;
			}
		}

		inline data_type& operator[] (const unsigned int position) const
		{
			return mapNodeList[position].nodeData;
		}

		inline key_type GetKeyAtIndex(const unsigned int position) const
		{
			return mapNodeList[position].nodeKey;
		}
		unsigned int GetIndexAtKey(const key_type &key)
		{
			bool objectExists;
			unsigned index = mapNodeList.GetIndexFromKey(key, objectExists);
			if (objectExists)
				return index;
			else
				assert(objectExists == true);
		}
		void RemoveAtIndex(const unsigned index);
	};
}
#endif