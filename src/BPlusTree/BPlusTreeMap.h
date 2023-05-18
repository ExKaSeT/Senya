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
		if (degree < 3)
			throw std::runtime_error("Degree must be >= 3");
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
	Node* splitLeafNode(Node* toSplit, Entry* add)
	{
		Node* newNode = createNode(true);
		newNode->left = toSplit;
		newNode->right = toSplit->right;
		newNode->parent = toSplit->parent;
		if (toSplit->right != nullptr)
			toSplit->right->left = newNode;
		toSplit->right = newNode;
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

	Entry* findMinEntry(Node* subtree)
	{
		Node* node = subtree;
		while (!node->isLeaf())
		{
			node = node->children[0];
		}
		return node->entries->get(0);
	}

	// returns new node (with more elem if size is even) (add != const)
	Node* splitInternalNode(Node* toSplit, Node* add)
	{
		Node* newNode = createNode(false);
		newNode->left = toSplit;
		newNode->right = toSplit->right;
		newNode->parent = toSplit->parent;
		if (toSplit->right != nullptr)
			toSplit->right->left = newNode;
		toSplit->right = newNode;
		struct NodeAndMin
		{
			Node* node;
			Entry* min;
		};
		SortedArray<NodeAndMin>* arr = SortedArray<NodeAndMin>::create(toSplit->entries->getSize() + 2, alloc,
				[this](NodeAndMin const& a, NodeAndMin const& b) -> int
				{ return compare(*(a.min->key), *(b.min->key)); });
		arr->add({ toSplit->children[0], createEntry(*(findMinEntry(toSplit->children[0])->key)) });
		arr->add({ add, createEntry(*(findMinEntry(add)->key)) });
		for (int x = 0; x < toSplit->entries->getSize(); x++)
		{
			arr->add({ toSplit->children[x + 1], toSplit->entries->get(x) });
		}
		toSplit->entries->clear();
		int mid = arr->getSize() / 2;
		toSplit->children[0] = arr->get(0).node;
		for (int x = 1; x < mid; x++)
		{
			toSplit->entries->add(arr->get(x).min);
			toSplit->children[x] = arr->get(x).node;
		}
		newNode->children[0] = arr->get(mid).node;
		newNode->children[0]->parent = newNode;
		int childIndex = 1;
		for (int x = mid + 1; x < arr->getSize(); x++)
		{
			newNode->entries->add(arr->get(x).min);
			newNode->children[childIndex] = arr->get(x).node;
			newNode->children[childIndex]->parent = newNode;
			childIndex++;
		}
		destroyEntry(arr->get(0).min);
		destroyEntry(arr->get(mid).min);
		arr->destroy();
		return newNode;
	}

	// changes key of min elem in parent (const Entries)
	void leafNodeChangedMinElem(Node* leafNode, Entry* newMin, Entry* prevMin)
	{
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

	bool addKeyToInternalRec(Node* internal, Node* add)
	{
		if (internal->isLeaf())
			throw std::runtime_error("Unexpected");
		Entry* min = findMinEntry(add);
		if (!internal->entries->isFull())
		{
			int insertIndex = internal->entries->add(createEntry(*(min->key)));
			if (insertIndex < 0)
				throw std::runtime_error("Unexpected");
			if (compare(*(min->key), *(findMinEntry(internal->children[0])->key)) < 1)
				throw std::runtime_error("Unexpected");
			insertIndex++;
			memmove(internal->children + insertIndex + 1, internal->children + insertIndex,
					sizeof(*(internal->children)) * (internal->entries->getSize() - insertIndex));
			internal->children[insertIndex] = add;
			add->parent = internal;
			return true;
		}
		if (internal->left != nullptr && !internal->left->entries->isFull())
		{
			Entry* minInFirstChild = findMinEntry(internal->children[0]);
			int cmp = compare(*(min->key), *(minInFirstChild->key));
			if (cmp < 0) // "add" - the lowest node
			{
				int insertIndex = internal->left->entries->add(createEntry(*(min->key)));
				if (insertIndex != internal->left->entries->getSize() - 1)
					throw std::runtime_error("Unexpected");
				internal->left->children[insertIndex + 1] = add;
				add->parent = internal->left;
				return true;
			}
			else if (cmp == 0)
			{
				throw std::runtime_error("Unexpected");
			}
			int insertIndex = internal->left->entries->add(createEntry(*(minInFirstChild->key)));
			if (insertIndex != internal->left->entries->getSize() - 1)
				throw std::runtime_error("Unexpected");
			internal->left->children[insertIndex + 1] = internal->children[0];
			internal->children[0]->parent = internal->left;
			internal->children[0] = nullptr;

			add->parent = internal;
			if (internal->entries->binarySearch(insertIndex, min))
				throw std::runtime_error("Unexpected");
			if (insertIndex == 0)
			{
				internal->children[0] = add;
				leafNodeChangedMinElem(internal, min, minInFirstChild);
				return true;
			}
			Entry* toDel = internal->entries->get(0);
			internal->entries->remove(0);
			leafNodeChangedMinElem(internal, toDel, minInFirstChild);
			destroyEntry(toDel);
			internal->entries->add(createEntry(*(min->key)));
			memmove(internal->children, internal->children + 1, insertIndex * sizeof(*(internal->children)));
			internal->children[insertIndex] = add;
			return true;
		}
		if (internal->right != nullptr && !internal->right->entries->isFull())
		{
			Entry* prevMin = findMinEntry(internal->right->children[0]);
			Entry* minInLastChild = internal->entries->get(internal->entries->getSize() - 1);
			int cmp = compare(*(min->key), *(minInLastChild->key));
			if (cmp > 0) // "add" more than last child in internal
			{
				leafNodeChangedMinElem(internal->right->children[0], min, prevMin);
				int insertIndex = internal->right->entries->add(createEntry(*(prevMin->key)));
				if (insertIndex != 0)
					throw std::runtime_error("Unexpected");
				memmove(internal->right->children + 1, internal->right->children,
						sizeof(*(internal->children)) * internal->right->entries->getSize());
				internal->right->children[0] = add;
				add->parent = internal->right;
				return true;
			}
			else if (cmp == 0)
			{
				throw std::runtime_error("Unexpected");
			}
			leafNodeChangedMinElem(internal->right->children[0], minInLastChild, prevMin);
			int insertIndex = internal->right->entries->add(createEntry(*(prevMin->key)));
			if (insertIndex != 0)
				throw std::runtime_error("Unexpected");
			memmove(internal->right->children + 1, internal->right->children,
					sizeof(*(internal->children)) * internal->right->entries->getSize());
			int indexOfLastChild = internal->entries->getSize();
			internal->right->children[0] = internal->children[indexOfLastChild];
			internal->right->children[0]->parent = internal->right;

			add->parent = internal;
			internal->entries->remove(internal->entries->getSize() - 1);
			destroyEntry(minInLastChild);
			if (internal->entries->binarySearch(insertIndex, min))
				throw std::runtime_error("Unexpected");
			if (insertIndex == internal->entries->getSize())
			{
				internal->children[indexOfLastChild] = add;
				if (internal->entries->add(createEntry(*(min->key))) != indexOfLastChild - 1)
					throw std::runtime_error("Unexpected");
				return true;
			}
			if (compare(*(min->key), *(findMinEntry(internal->children[0])->key)) < 1)
				throw std::runtime_error("Unexpected");
			memmove(internal->children + insertIndex + 2, internal->children + insertIndex + 1,
					sizeof(*(internal->children)) * internal->entries->getSize());
			internal->children[insertIndex + 1] = add;
			if (internal->entries->add(createEntry(*(min->key))) != insertIndex)
				throw std::runtime_error("Unexpected");
			return true;
		}
		Node* newNode = splitInternalNode(internal, add);
		if (internal == root)
		{
			Node* right = newNode;
			Node* left = internal;
			root = createNode(false);
			right->parent = root;
			left->parent = root;
			root->children[0] = left;
			root->children[1] = right;
			root->entries->add(createEntry(*(findMinEntry(right)->key)));
			return true;
		}
		return addKeyToInternalRec(internal->parent, newNode);
	}

public:
	void checkCorrectness()
	{
		Node* node = root;
		while (!node->isLeaf())
		{
			node = node->children[0];
		}
		while (node->right != nullptr) {
			K* minInCurrent = node->entries->get(0)->key;
			K* minInRight = node->right->entries->get(0)->key;
			if (compare(*minInCurrent, *minInRight) >= 0)
				throw std::runtime_error("Check failed: Not ascending order");
			node = node->right;
		}
	}

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
		if (current->left != nullptr && !current->left->entries->isFull())
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
		if (current->right != nullptr && !current->right->entries->isFull())
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
		Entry* prevSmallest = current->entries->get(0);
		Node* newRightNode = splitLeafNode(current, data);
		if (prevSmallest != current->entries->get(0))
		{
			leafNodeChangedMinElem(current, current->entries->get(0), prevSmallest);
		}
		return addKeyToInternalRec(current->parent, newRightNode);
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
