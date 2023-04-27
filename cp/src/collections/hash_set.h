
#ifndef PROGC_SRC_COLLECTIONS_HASH_MAP_H
#define PROGC_SRC_COLLECTIONS_HASH_MAP_H

#include <type_traits>
#include <vector>
#include <optional>
#include "set.h"
#include "../extensions/hashable.h"
#include "b-tree/trees/vanilla_b_plus_tree.h"


template<typename T>
class HashSet : public Set<T>
{
	static_assert(std::is_base_of<Hashable<T>, T>::value, "Type T must extend Hashable");

private:

	VanillaBPlusTree<size_t, std::vector<T>, 4> data;
	size_t size_ = 0;

public:

	void add(const T& value) override
	{
		size_t hashcode = value.hashcode();
		std::vector<T> list;
		if (data.search(hashcode, list))
		{
			for (auto it = list.begin(); it != list.end(); it++)
			{
				if (value == *it)
				{
					list.erase(it);
					size_--;
					break;
				}
			}
		}
		list.push_back(value);
		data.insert(hashcode, list);
		size_++;
	}

	std::optional<T> contains(const T& value) override
	{
		std::vector<T> list;
		if (!data.search(value.hashcode(), list))
			return std::nullopt;
		for (T elem : list)
		{
			if (value == elem)
				return elem;
		}
		return std::nullopt;
	}

	void remove(const T& value) override
	{
		std::vector<T> list;
		size_t hashcode = value.hashcode();
		if (!data.search(hashcode, list))
			return;
		for (auto it = list.begin(); it != list.end(); it++)
		{
			if (value == *it)
			{
				list.erase(it);
				if (list.empty())
					data.delete_key(value.hashcode());
				else
					data.insert(hashcode, list);
				size_--;
				break;
			}
		}
	}

	void clear() override
	{
		data.clear();
	}

	size_t size() const override
	{
		return size_;
	}

	bool empty() const override
	{
		return size_ > 0;
	}
};

#endif //PROGC_SRC_COLLECTIONS_HASH_MAP_H
