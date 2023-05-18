#include <iostream>
#include <random>
#include "BPlusTree/BPlusTreeMap.h"
#include "BPlusTree/SortedArray.h"
#include "../lab_5/1/src/logger_builder_concrete.h"
#include "BPlusTree/allocators/default_memory.h"

int cmp(int const &a,  int const& b) {
	return a - b;
}

int main()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 1000);

	logger_builder *loggerBuilder = new logger_builder_concrete();
	logger *logger = loggerBuilder->add_stream("console", logger::severity::trace)->construct();

	int count = 0;
	while (true) {
		BPlusTreeMap<int, int> map(3, 4, cmp, std::make_shared<DefaultMemory>());
		for (int x = 0; x < 100; x ++) {
			int num = dis(gen);
			std::cout << "|| ADD || " << num << std::endl;
			map.add(num, num);
			map.print();
			map.checkCorrectness();
		}
		count++;
		if (count % 100 == 0)
			std::cout << count << std::endl;
	}

//	BPlusTreeMap<int, int> map(3, 4, cmp, std::make_shared<DefaultMemory>());
//	int arr[100];
//	int count = 0;
//	for (int x = 1000; x > 0; x -= 2) {
//		int num = dis(gen);
//		arr[count] = num;
//		count++;
//		for (int x = 0; x < count; x++)
//			std::cout << arr[x] << ", ";
//		std::cout << std::endl;
//		map.add(num, num);
//		map.print();
//	}

//	int arr[] = {94, 67, 76, 5, 29, 40, 81, 17, 44, 30, 38, 10, 88, 12, 43, 53, 18, 46, 9, 2};
//	for (int x = 0; x < 19; x++) {
//		int num = arr[x];
//		map.add(num, num);
//		map.print();
//	}
//	map.add(2, 2);
//	map.print();
	return 0;
}