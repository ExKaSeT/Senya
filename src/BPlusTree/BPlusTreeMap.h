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
	// returns new node (with more elem if size is even) (add != const)
	Node* splitLeafNode(Node*& toSplit, Entry*& add)
	{
		Node* newNode = createNode(true);
		toSplit->right = newNode;
		newNode->left = toSplit;
		newNode->parent = toSplit->parent;
		int insertIndex;
		if (toSplit->entries->binarySearch(insertIndex, add))
			throw std::runtime_error("Unexpected");
		int countElemToKeep = (toSplit->entries->getSize() + 1) / 2; // 5 => 3 оставляем
		int moveIndex;
		if (insertIndex < countElemToKeep)
		{
			moveIndex = countElemToKeep - 1;
		}
		else
		{
			moveIndex = countElemToKeep;
		}
		const int size = toSplit->entries->getSize();
		for (int x = moveIndex; x < size; x++)
		{
			newNode->entries->add(toSplit->entries->get(moveIndex));
			toSplit->entries->remove(moveIndex);
		}
		if (insertIndex < countElemToKeep)
		{
			toSplit->entries->add(add);
		}
		else
		{
			newNode->entries->add(add);
		}
		return newNode;
	}

	// changes key of min elem in parent (const Entries)
	void leafNodeChangedMinElem(Node* leafNode, Entry* newMin, Entry* prevMin)
	{
		if (!leafNode)
			throw std::runtime_error("Incorrect args");
		Node* current = leafNode;
		Node* child;
		while (current->parent != nullptr)
		{
			child = current;
			current = current->parent;
			if (current->children[0] != child)
				break;
		}
		if (current->parent == nullptr && current->children[0] == child)
		{
			return; // the smallest key in the tree isn`t stored
		}
		int index;
		bool isFound = current->entries->binarySearch(index, prevMin);
		if (!isFound)
			throw std::runtime_error("Unexpected");
		Entry* keyToRemove = current->entries->get(index);
		current->entries->remove(index);
		destroyEntry(keyToRemove);
		current->entries->add(createEntry(*(newMin->key)));
	}

//	Node* findLeaf(const K& key)
//	{
//		Node *current = root;
//
//	}

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
		if (current->entries->contains(data) >= 0)
		{
			destroyEntry(data);
			return false;
		}
		if (!current->entries->isFull())
		{
			int index = current->entries->add(data);
			if (index == 0)
			{
				leafNodeChangedMinElem(current, data, current->entries->get(1));
			}
			size++;
			return true;
		}
		// try to give elem to the left sibling
		if (!current->left->entries->isFull())
		{
			Entry* toMove = current->entries->get(0);
			current->entries->remove(0);
			current->entries->add(data);
			current->left->entries->add(toMove);
			leafNodeChangedMinElem(current, current->entries->get(0), toMove);
			size++;
			return true;
		}
		// try to give elem to the right sibling
		if (!current->right->entries->isFull())
		{
			// largest value may be data
			Entry* last = current->entries->get(current->entries->getSize() - 1);
			int cmp = compare(*(data->key), *(last->key));
			if (cmp < 0) // data less than last
			{
				current->entries->remove(current->entries->getSize() - 1);
				current->entries->add(data);
				current->right->entries->add(last);
				leafNodeChangedMinElem(current->right, last, current->right->entries->get(1));
			}
			else if (cmp > 0)
			{
				current->right->entries->add(data);
				leafNodeChangedMinElem(current->right, data, current->right->entries->get(1));
			}
			else
			{
				throw std::runtime_error("Unexpected");
			}
			size++;
			return true;
		}
		// TODO: test siblings exchange
		return true;
	}

	void print()
	{
		printRec(root);
	}

private:
	void printRec(Node* node, int depth = 0)
	{
		if (node == nullptr)
		{
			return;
		}

		std::cout << std::string(depth, '\t');

		if (node->isLeaf())
		{
			// Если узел является листом, то выводим его ключи
			for (int i = 0; i < node->entries->getSize(); i++)
			{
				std::cout << *(node->entries->get(i)->key) << " ";
			}
			std::cout << std::endl;
		}
		else
		{
			// Если узел не является листом, то выводим его ключи и рекурсивно вызываем функцию для каждого дочернего узла
			for (int i = 0; i < node->entries->getSize(); i++)
			{
				std::cout << *(node->entries->get(i)->key) << " ";
			}
			std::cout << std::endl;

			for (int i = 0; i < node->entries->getSize(); i++)
			{
				printRec(node->children[i], depth + 1);
			}

			// Рекурсивный вызов для крайнего правого дочернего узла
			printRec(node->children[node->entries->getSize()], depth + 1);
		}
	}
};

#endif //PROGC_SRC_BPLUSTREE_BPLUSTREEMAP_H
