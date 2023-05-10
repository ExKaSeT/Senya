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
	int size = 0;

	struct Entry
	{
		K* key;
		V* value;
	};

	Entry* createEntry(const K& key, const V& value)
	{
		auto* entry = reinterpret_cast<Entry*>(alloc->allocate(sizeof(Entry)));
		entry->key = reinterpret_cast<K*>(alloc->allocate(sizeof(K)));
		entry->value = reinterpret_cast<V*>(alloc->allocate(sizeof(V)));
		memcpy(entry->key, &key, sizeof(K));
		memcpy(entry->value, &value, sizeof(V));
		return entry;
	}

	Entry* createEntry(const K& key)
	{
		auto* entry = reinterpret_cast<Entry*>(alloc->allocate(sizeof(Entry)));
		entry->key = reinterpret_cast<K*>(alloc->allocate(sizeof(K)));
		entry->value = nullptr;
		memcpy(entry->key, &key, sizeof(K));
		return entry;
	}

	void destroyEntry(Entry* entry)
	{
		if (entry->key != nullptr)
			alloc->deallocate(entry->key);
		if (entry->value != nullptr)
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
		node->parent = nullptr;
		node->left = nullptr;
		node->right = nullptr;
		node->entries = nullptr;
		node->children = nullptr;
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
			for (int x = 0; x < maxChildCount; x++)
				node->children[x] = nullptr;
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

	Node* root = nullptr;

	void init()
	{
		if (degree < 2)
			throw std::runtime_error("Degree must be >= 2");
		if (leafCapacity < 2)
			throw std::runtime_error("Leaf capacity must be >= 2");

		root = createNode(true);
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
private:
	// returns new node (with more elem if size is even)
	Node* splitLeafNode(Node*& toSplit, Entry*& add)
	{
		Node* newNode = createNode(true);
		toSplit->right = newNode;
		newNode->left = toSplit;
		int countElemToKeep = (toSplit->entries->getSize() + 1) / 2; // 5 => 3 оставляем
		for (int x = countElemToKeep; x < toSplit->entries->getSize(); x++)
		{
			newNode->entries->add(toSplit->entries->get(x));
		}
		for (int x = countElemToKeep; x < toSplit->entries->getSize(); x++)
		{
			toSplit->entries->remove(countElemToKeep);
		}
		newNode->entries->add(add);
		return newNode;
	}

public:
	bool add(const K& key, const V& value)
	{
		Entry* data = createEntry(key, value);

		if (root->isLeaf())
		{
			if (root->entries->contains(data) >= 0)
			{
				destroyEntry(data);
				return false;
			}
			if (!root->entries->isFull())
			{
				root->entries->add(data);
				size++;
				return true;
			}
			// node is full, split:
			Node* right = splitLeafNode(root, data);
			Node* left = root;
			root = createNode(false);
			right->parent = root;
			left->parent = root;
			root->children[0] = left;
			root->children[1] = right;
			root->entries->add(createEntry(*(right->entries->get(0)->key)));
			size++;
			return true;
		}

		Node* current = root;
		while (!current->isLeaf())
		{
			int indexToInsert = 0;
			bool isFound = current->entries->binarySearch(indexToInsert, data);
			if (isFound)
			{
				destroyEntry(data);
				return false;
			}
			// index to insert new val == needed index of children
			current = current->children[indexToInsert];
		}
		if (current->entries->contains(data))
		{
			destroyEntry(data);
			return false;
		}
		if (!current->entries->isFull())
		{
			int index = current->entries->add(data);
			if (index == 0) // so need to change key in some parent
			{
				// TODO: change key
			}
			size++;
			return true;
		}
		// current leaf is full...
		// TODO: checks...
		return true;
	}
};

#endif //PROGC_SRC_BPLUSTREE_BPLUSTREEMAP_H
