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
	std::uniform_int_distribution<> dis(1, 100);

	logger_builder *loggerBuilder = new logger_builder_concrete();
	logger *logger = loggerBuilder->add_stream("console", logger::severity::trace)->construct();
	int count = 0;
	while (true) {
		BPlusTreeMap<int, int> map(3, 4, cmp, std::make_shared<DefaultMemory>());
		for (int x = 100; x > 0; x -= 2) {
			int num = dis(gen);
//			std::cout << "|| ADD || " << num << std::endl;
			map.add(num, num);
//			map.print();
		}
		count++;
		if (count % 100 == 0)
			std::cout << count << std::endl;
	}
//	map.add(40, 40);
//	map.add(42, 42);

	return 0;
}