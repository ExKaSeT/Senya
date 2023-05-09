#include <iostream>
#include "BPlusTree/BPlusTreeMap.h"
#include "BPlusTree/SortedArray.h"
#include "../lab_5/1/src/logger_builder_concrete.h"
#include "BPlusTree/allocators/default_memory.h"

int cmp(int const &a,  int const& b) {
	return a - b;
}

int main()
{
	logger_builder *loggerBuilder = new logger_builder_concrete();
	logger *logger = loggerBuilder->add_stream("console", logger::severity::trace)->construct();
	std::function<int(const int&, const int&)> myFunction = cmp;
	BPlusTreeMap<int, int> map(2, 10, cmp, std::make_shared<Memory_2>(logger));
//	SortedArray<int> *arr = SortedArray<int>::create(20,
//		std::make_shared<Memory_2>(), [](const int& a,  const int& b) {return a - b;});
//	for (int x = 0; x < 20; x += 2) {
//		arr->add(x);
//	}
//
//	for (int x = 0; x < 10; x += 2) {
//		arr->removeElem(x);
//	}
//
//	int l = -8;
//	std::cout << arr->contains(l) << std::endl;
//	arr->forEach([](const int& elem) {std::cout << elem << " ";});
//	arr->destroy();
	return 0;
}