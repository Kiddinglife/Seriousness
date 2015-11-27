#pragma once
namespace DataStructures
{
	/// The default comparison has to be first so it can be called as a default parameter.
	/// It then is followed by MapNode, followed by NodeComparisonFunc
	template <class key_type>
	int defaultMapKeyComparison(const key_type &a, const key_type &b)
	{
		if (a < b) return -1; if (a == b) return 0; return 1;
	}

	/// @note IMPORTANT! If you use defaultMapKeyComparison then call IMPLEMENT_DEFAULT_COMPARISON or you will get an unresolved external linker error.
	template <class key_type, class data_type,
		int(*key_comparison_func)(const key_type&, const key_type&) = defaultMapKeyComparison < key_type >>
	class Map
	{
	public:
		Map();
		~Map();
	};
}

