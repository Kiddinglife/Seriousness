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
	static int defaultMapKeyComparison(const key_type &a, const key_type &b)
	{
		if (a < b) return -1; if (a == b) return 0; return 1;
	}

	/// \note IMPORTANT! If you use defaultMapKeyComparison then call IMPLEMENT_DEFAULT_COMPARISON or you will get an unresolved external linker error.
	template <class key_type, class data_type, int(*key_comparison_func)(const key_type&, const key_type&) = defaultMapKeyComparison<key_type> >
	class JACKIE_EXPORT OrderListMap
	{
	public:
		static void IMPLEMENT_DEFAULT_COMPARISON(void)
		{
			DataStructures::defaultMapKeyComparison<key_type>(key_type(), key_type());
		}

		// Has to be a static because the comparison callback for DataStructures::OrderedList is a C function
		static int NodeComparisonFunc(const key_type &a, const MapNode &b)
		{
			return key_comparison_func(a, b.nodeKey);
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


	};
}
#endif