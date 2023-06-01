#ifndef PROGC_SRC_BPLUSTREE_BPLUSTREEMAP_H
#define PROGC_SRC_BPLUSTREE_BPLUSTREEMAP_H


#include <stdexcept>
#include <memory>
#include <functional>
#include <set>
#include <optional>
#include <iostream>
#include "SortedArray.h"
#include "../allocators/default_memory.h"
#include "../Map.h"


template<typename K, typename V>
class BPlusTreeMap : public Map<K, V>
{
private:
	const int degree;
	const int leafCapacity;
	const int minLeafSize = (leafCapacity + 1) / 2;
	const int maxChildCount = degree;
	const int maxKeysCount = degree - 1;
	const int minChildCount = (maxChildCount + 1) / 2;
	const int minKeysCount = minChildCount - 1;
	const std::shared_ptr<Memory> alloc;
	const std::function<int(const K&, const K&)> compare;
	int size_ = 0;

	struct Entry
	{
		K* key;
		V* value;
	};

	Entry* createEntry(const K& key)
	{
		auto* entry = reinterpret_cast<Entry*>(alloc->allocate(sizeof(Entry)));
		new (entry) Entry;
		entry->key = reinterpret_cast<K*>(alloc->allocate(sizeof(K)));
		entry->value = nullptr;
		new (entry->key) K(key);
		return entry;
	}

	Entry* createEntry(const K& key, const V& value)
	{
		if (std::is_same<V, Null>::value)
		{
			return createEntry(key);
		}
		auto* entry = reinterpret_cast<Entry*>(alloc->allocate(sizeof(Entry)));
		new (entry) Entry;
		entry->key = reinterpret_cast<K*>(alloc->allocate(sizeof(K)));
		entry->value = reinterpret_cast<V*>(alloc->allocate(sizeof(V)));
		new (entry->key) K(key);
		new (entry->value) V(value);
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
			degree(degree), alloc(std::make_shared<DefaultMemory>()), compare(comparator), leafCapacity(leafCapacity)
	{
		init();
	}

	~BPlusTreeMap() override
	{
		Node* current = root;
		bool isLeaf = false;
		while (true)
		{
			Node* toDel = current;
			if (current->isLeaf())
			{
				isLeaf = true;
			}
			else
			{
				current = current->children[0];
			}
			while (toDel->right != nullptr)
			{
				Node* right = toDel->right;
				toDel->entries->forEach([this](auto elem)
				{ destroyEntry(elem); });
				destroyNode(toDel);
				toDel = right;
			}
			toDel->entries->forEach([this](auto elem)
			{ destroyEntry(elem); });
			destroyNode(toDel);
			if (isLeaf)
				break;
		}
	}

	BPlusTreeMap(BPlusTreeMap const&) = delete;

	BPlusTreeMap(BPlusTreeMap&&) = delete;

	BPlusTreeMap operator=(BPlusTreeMap const&) = delete;

	BPlusTreeMap operator=(BPlusTreeMap&&) = delete;

private:
	// returns new node (with more elem if size_ is even) (add != const)
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

	// returns new node (with more elem if size_ is even) (add != const)
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
					sizeof(*(internal->right->children)) * internal->right->entries->getSize());
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
					sizeof(*(internal->children)) * (internal->entries->getSize() - insertIndex));
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
		if (root->isLeaf())
			return;
		std::set<K> leafKeys;
		std::set<K> internalKeys;
		int leafCount = 0;
		Node* node = root;
//		for (int x = 0; x < node->entries->getSize(); x++) {
//			internalKeys.insert(*(node->entries->get(x)->key));
//		}
		while (!node->isLeaf())
		{
			Node* internal = node;
			while (true)
			{
				if (internal->isLeaf())
					throw std::runtime_error("Check failed: Internal - leaf?");
				for (int x = 0; x < internal->entries->getSize(); x++)
				{
					int prevSize = internalKeys.size();
					internalKeys.insert(*(internal->entries->get(x)->key));
					if (internalKeys.size() == prevSize)
						throw std::runtime_error("Check failed: Internal contains duplicate");
				}
				if (internal->right == nullptr)
					break;
				internal = internal->right;
			}
			node = node->children[0];
		}
		while (true)
		{
			leafCount++;
			if (node->right != nullptr)
			{
				K* minInCurrent = node->entries->get(0)->key;
				K* minInRight = node->right->entries->get(0)->key;
				if (compare(*minInCurrent, *minInRight) >= 0)
					throw std::runtime_error("Check failed: Not ascending order");
			}
			if (!node->isLeaf())
				throw std::runtime_error("Check failed: Leaf not leaf?");
			for (int x = 0; x < node->entries->getSize(); x++)
			{
				int prevSize = leafKeys.size();
				leafKeys.insert(*(node->entries->get(x)->key));
				if (leafKeys.size() == prevSize)
					throw std::runtime_error("Check failed: Leaf contains duplicate");
			}
			if (node->right == nullptr)
				break;
			node = node->right;
		}
		if (leafCount != internalKeys.size() + 1)
			throw std::runtime_error("Check failed: Incorrect structure?");
	}

	bool add(const K& key, const V& value) override
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
				size_++;
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
			size_++;
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
		size_++;
		if (!current->entries->isFull())
		{
			int index = current->entries->add(data);
			if (index == 0)
			{
				leafNodeChangedMinElem(current, data, current->entries->get(1));
			}
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

private:
	void afterNodeMerge(Node* toDelete, Entry* min)
	{
		Node* current = toDelete->parent;
		int childIndex;
		if (min == nullptr)
		{
			childIndex = 0;
		}
		else
		{
			childIndex = current->entries->contains(min) + 1;
		}
		if (current->children[childIndex] != toDelete)
			throw std::runtime_error("Unexpected (afterNodeMerge)");
		if (min != nullptr && childIndex == 0 && compare(*(min->key), *(current->entries->get(0)->key)) >= 0)
			throw std::runtime_error("Unexpected (afterNodeMerge)");
		if (toDelete->right != nullptr)
			toDelete->right->left = toDelete->left;
		if (toDelete->left != nullptr)
			toDelete->left->right = toDelete->right;
		destroyNode(toDelete);
		memmove(current->children + childIndex, current->children + childIndex + 1,
				(current->entries->getSize() - childIndex) * sizeof(*(current->children)));
		if (childIndex == 0)
		{
			Entry* toDel = current->entries->get(0);
			current->entries->remove(0);
			destroyEntry(toDel);
		}
		else
		{
			Entry* toDel = current->entries->get(childIndex - 1);
			current->entries->remove(childIndex - 1);
			destroyEntry(toDel);
		}
		if (current == root)
		{
			if (current->entries->isEmpty())
			{
				root = current->children[0];
				destroyNode(current);
				root->parent = nullptr;
				if (root->left != nullptr || root->right != nullptr || root->isLeaf())
					throw std::runtime_error("Unexpected (afterNodeMerge)");
			}
			return;
		}
		if (current->entries->getSize() >= minKeysCount)
		{
			if (childIndex == 0)
			{
				leafNodeChangedMinElem(current, findMinEntry(current), min);
			}
			return;
		}
		if (current->left == nullptr && current->right == nullptr)
			throw std::runtime_error("Unexpected (afterNodeMerge)");
		if (current->left != nullptr)
		{
			Node* left = current->left;
			// try to take child from left
			if (left->entries->getSize() > minKeysCount)
			{
				int indexOfLast = left->entries->getSize() - 1;
				Entry* toMove = left->entries->get(indexOfLast);
				left->entries->remove(indexOfLast);
				Entry* minInCurrent = findMinEntry(current);
				Entry* prevMin = childIndex == 0 ? min : minInCurrent;
				leafNodeChangedMinElem(current, toMove, prevMin);
				destroyEntry(toMove);
				memmove(current->children + 1, current->children,
						(current->entries->getSize() + 1) * sizeof(*(current->children)));
				current->children[0] = left->children[indexOfLast + 1];
				left->children[indexOfLast + 1] = nullptr;
				current->children[0]->parent = current;
				current->entries->add(createEntry(*(minInCurrent->key)));
				return;
			}
			// merge with left
			int leftChildIndex = left->entries->getSize() + 1;
			if (left->entries->add(createEntry(*(findMinEntry(current)->key))) != leftChildIndex - 1)
				throw std::runtime_error("Unexpected (afterNodeMerge)");
			left->children[leftChildIndex] = current->children[0];
			left->children[leftChildIndex]->parent = left;
			leftChildIndex++;
			for (int x = 0; x < current->entries->getSize(); x++)
			{
				if (left->entries->add(current->entries->get(x)) != leftChildIndex - 1)
					throw std::runtime_error("Unexpected (afterNodeMerge)");
				left->children[leftChildIndex] = current->children[x + 1];
				left->children[leftChildIndex]->parent = left;
				leftChildIndex++;
			}
			if (childIndex != 0 || min == nullptr)
			{
				min = findMinEntry(current);
			}
		}
		else
		{
			Node* right = current->right;
			// try to take child from right
			if (right->entries->getSize() > minKeysCount)
			{
				Entry* rightMin = findMinEntry(right);
				Entry* toDel = right->entries->get(0);
				right->entries->remove(0);
				leafNodeChangedMinElem(right, toDel, rightMin);
				destroyEntry(toDel);
				int indexOfChild = current->entries->add(createEntry(*(rightMin->key))) + 1;
				if (indexOfChild != current->entries->getSize())
					throw std::runtime_error("Unexpected (afterNodeMerge)");
				current->children[indexOfChild] = right->children[0];
				current->children[indexOfChild]->parent = current;
				right->children[0] = nullptr;
				memmove(right->children, right->children + 1,
						(right->entries->getSize() + 1) * sizeof(*(right->children)));
				leafNodeChangedMinElem(current, findMinEntry(current), min);
				return;
			}
			// merge with right
			Entry* minInRight = findMinEntry(right);
			right->entries->add(createEntry(*(minInRight->key)));
			memmove(right->children + current->entries->getSize() + 1, right->children,
					right->entries->getSize() * sizeof(*(right->children)));
			memcpy(right->children, current->children,
					(current->entries->getSize() + 1) * sizeof(*(right->children)));
			for (int x = 0; x <= current->entries->getSize(); x++)
			{
				right->children[x]->parent = right;
			}
			for (int x = 0; x < current->entries->getSize(); x++)
			{
				right->entries->add(current->entries->get(x));
			}
			leafNodeChangedMinElem(right, findMinEntry(right), minInRight);
			min = nullptr;
		}
		afterNodeMerge(current, min);
	}

public:
	bool remove(const K& key) override
	{
		Entry* data = createEntry(key);

		Node* current = root;
		while (!current->isLeaf())
		{
			int index = 0;
			bool isFound = current->entries->binarySearch(index, data);
			if (isFound)
			{
				current = current->children[index + 1];
			}
			else
			{
				current = current->children[index];
			}
		}
		int index = current->entries->contains(data);
		destroyEntry(data);
		if (index < 0)
		{
			return false;
		}
		size_--;
		data = current->entries->get(index);
		current->entries->remove(index);

		if (current == root) // root == leaf
		{
			destroyEntry(data);
			return true;
		}
		if (current->entries->getSize() >= minLeafSize)
		{
			if (index == 0)
			{
				leafNodeChangedMinElem(current, current->entries->get(0), data);
			}
			destroyEntry(data);
			return true;
		}
		if (current->left == nullptr && current->right == nullptr)
			throw std::runtime_error("Unexpected");
		bool mergeWithRight = false;
		if (current->left != nullptr)
		{
			Node* left = current->left;
			// try to take elem from left
			if (left->entries->getSize() > minLeafSize)
			{
				int indexOfLast = left->entries->getSize() - 1;
				Entry* toMove = left->entries->get(indexOfLast);
				left->entries->remove(indexOfLast);
				if (current->entries->add(toMove) != 0)
					throw std::runtime_error("Unexpected");
				Entry* prevMin = index == 0 ? data : current->entries->get(1);
				leafNodeChangedMinElem(current, toMove, prevMin);
				destroyEntry(data);
				return true;
			}
			// merge with left
			for (int x = 0; x < current->entries->getSize(); x++)
			{
				left->entries->add(current->entries->get(x));
			}
		}
		else
		{
			Node* right = current->right;
			// try to take elem from right
			if (right->entries->getSize() > minLeafSize)
			{
				Entry* toMove = right->entries->get(0);
				right->entries->remove(0);
				leafNodeChangedMinElem(right, right->entries->get(0), toMove);
				current->entries->add(toMove);
				destroyEntry(data);
				return true;
			}
			// merge with right
			Entry* minInRight = findMinEntry(right);
			for (int x = 0; x < current->entries->getSize(); x++)
			{
				right->entries->add(current->entries->get(x));
			}
			leafNodeChangedMinElem(right, right->entries->get(0), minInRight);
			mergeWithRight = true;
		}
		if (current->parent == root && root->entries->getSize() == 1)
		{
			destroyEntry(root->entries->get(0));
			destroyNode(root);
			if (current->left != nullptr)
			{
				root = current->left;
			}
			else
			{
				root = current->right;
			}
			destroyNode(current);
			root->right = nullptr;
			root->left = nullptr;
			root->parent = nullptr;
			if (!root->isLeaf())
				throw std::runtime_error("Unexpected (remove)");
			return true;
		}
		if (mergeWithRight)
		{
			afterNodeMerge(current, nullptr);
		}
		else
		{
			if (index == 0)
			{
				afterNodeMerge(current, data);
			}
			else
			{
				afterNodeMerge(current, current->entries->get(0));
			}
		}
		destroyEntry(data);
		return true;
	}

	std::optional<V> get(const K& key) override
	{
		if (std::is_same<V, Null>::value)
		{
			return std::nullopt;
		}
		Entry* data = createEntry(key);
		Node* current = root;
		while (!current->isLeaf())
		{
			int index = 0;
			bool isFound = current->entries->binarySearch(index, data);
			if (isFound)
			{
				current = current->children[index + 1];
			}
			else
			{
				current = current->children[index];
			}
		}
		int index = current->entries->contains(data);
		destroyEntry(data);
		if (index < 0)
		{
			return std::nullopt;
		}
		return *(current->entries->get(index)->value);
	}

	bool set(const K& key, const V& newValue) override
	{
		if (std::is_same<V, Null>::value)
		{
			return false;
		}
		Entry* data = createEntry(key);
		Node* current = root;
		while (!current->isLeaf())
		{
			int index = 0;
			bool isFound = current->entries->binarySearch(index, data);
			if (isFound)
			{
				current = current->children[index + 1];
			}
			else
			{
				current = current->children[index];
			}
		}
		int index = current->entries->contains(data);
		destroyEntry(data);
		if (index < 0)
		{
			return false;
		}
		data = current->entries->get(index);
		*data->value = newValue;
		return true;
	}

	bool contains(const K& key) override
	{
		Entry* data = createEntry(key);
		Node* current = root;
		while (!current->isLeaf())
		{
			int index = 0;
			bool isFound = current->entries->binarySearch(index, data);
			if (isFound)
			{
				current = current->children[index + 1];
			}
			else
			{
				current = current->children[index];
			}
		}
		int index = current->entries->contains(data);
		destroyEntry(data);
		if (index < 0)
		{
			return false;
		}
		return true;
	}

	size_t size() override
	{
		return size_;
	}

	std::vector<typename Map<K, V>::Pair> entrySet(const K& minBound, const K& maxBound) override
	{
		if (compare(minBound, maxBound) > 0)
			throw std::runtime_error("Incorrect args");
		std::vector<typename Map<K, V>::Pair> list;
		Entry* data = createEntry(minBound);
		Node* current = root;
		while (!current->isLeaf())
		{
			int index = 0;
			bool isFound = current->entries->binarySearch(index, data);
			if (isFound)
			{
				current = current->children[index + 1];
			}
			else
			{
				current = current->children[index];
			}
		}
		int index;
		if (current->entries->binarySearch(index, data))
		{
			Entry* entry = current->entries->get(index);
			list.emplace_back(entry->key, entry->value);
			index++;
		}
		destroyEntry(data);
		while (true)
		{
			if (index >= current->entries->getSize())
			{
				if (current->right == nullptr)
					return std::move(list);
				current = current->right;
				index = 0;
			}
			Entry* entry = current->entries->get(index);
			if (compare(*(entry->key), maxBound) > 0)
				return std::move(list);
			list.emplace_back(entry->key, entry->value);
			index++;
		}
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
			for (int i = 0; i < node->entries->getSize(); i++)
			{
				std::cout << *(node->entries->get(i)->key) << " ";
			}
			std::cout << std::endl;
		}
		else
		{
			for (int i = 0; i < node->entries->getSize(); i++)
			{
				std::cout << *(node->entries->get(i)->key) << " ";
			}
			std::cout << std::endl;

			for (int i = 0; i < node->entries->getSize(); i++)
			{
				printRec(node->children[i], depth + 1);
			}
			printRec(node->children[node->entries->getSize()], depth + 1);
		}
	}
};


#endif //PROGC_SRC_BPLUSTREE_BPLUSTREEMAP_H
