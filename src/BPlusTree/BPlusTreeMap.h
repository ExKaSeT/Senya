#ifndef PROGC_SRC_BPLUSTREE_BPLUSTREEMAP_H
#define PROGC_SRC_BPLUSTREE_BPLUSTREEMAP_H

#include <stdexcept>
#include <memory>
#include <functional>
#include "allocators/memory.h"
#include "allocators/memory_2.h"
#include "SortedArray.h"

template<typename K, typename V>
class BPlusTreeMap
{
private:
	const int degree;
	const int leafCapacity;
	const int maxChildCount = degree;
	const int maxKeysCount = degree - 1;
	const std::shared_ptr<Memory> alloc;
	const std::function<int(const K&, const K&)> compare;

	struct Entry
	{
		K* key;
		V* value;
	};

	Entry* createEntry(const K& key, const V& value)
	{
		Entry* entry = reinterpret_cast<Entry*>(alloc->allocate(sizeof(Entry)));
		entry->key = reinterpret_cast<K*>(alloc->allocate(sizeof(K)));
		entry->value = reinterpret_cast<V*>(alloc->allocate(sizeof(V)));
		memcpy(entry->key, &key, sizeof(K));
		memcpy(entry->value, &value, sizeof(V));
		return entry;
	}

	void destroyEntry(Entry* entry)
	{
		alloc->deallocate(entry->key);
		alloc->deallocate(entry->value);
		alloc->deallocate(entry);
	}

	class Node
	{
	public:
		SortedArray<Entry*>* entries;
		Node** children;
		Node* parent;
		Node* left;
		Node* right;

		bool isLeaf()
		{
			return children == nullptr;
		}
	};

	Node* createNode(bool isLeaf)
	{
		Node* node = reinterpret_cast<Node*>(alloc->allocate(sizeof(Node)));
		if (isLeaf)
		{
			node->entries = SortedArray<Entry*>::create(leafCapacity, alloc,
				[this](Entry* const& a, Entry* const& b) -> int
				{ return compare(*(a->key), *(b->key)); });
		}
		else
		{
			node->entries = SortedArray<Entry*>::create(maxKeysCount, alloc,
				[this](Entry* const& a, Entry* const& b) -> int
				{ return compare(*(a->key), *(b->key)); });
			node->children = reinterpret_cast<Node**>(alloc->allocate(sizeof(Node*) * maxChildCount));
		}
		return node;
	}

	void destroyNode(Node* node)
	{
		if (!node->isLeaf())
		{
			alloc->deallocate(node->children);
		}
		node->entries->destroy();
		alloc->deallocate(node);
	}

	void init()
	{
		if (degree < 2)
			throw std::runtime_error("Degree must be >= 2");

		Node* node = createNode(true);
		for (int x = 0; x < 20; x += 2)
		{
			Entry* e = createEntry(x, x);
			node->entries->add(e);
		}

		node->entries->forEach([](auto& elem){ std::cout << *(elem->key) << " "; });
	}

public:
	explicit BPlusTreeMap(int degree, int leafCapacity, const std::function<int(const K&, const K&)>& comparator,
		const std::shared_ptr<Memory>& alloc) : degree(degree), compare(comparator), alloc(alloc),
												leafCapacity(leafCapacity)
	{
		init();
	}
	explicit BPlusTreeMap(const int degree, int leafCapacity,
		const std::function<int(const K&, const K&)>& comparator) :
		degree(degree), alloc(std::make_shared<Memory_2>()), compare(comparator), leafCapacity(leafCapacity)
	{
		init();
	}

};

#endif //PROGC_SRC_BPLUSTREE_BPLUSTREEMAP_H
