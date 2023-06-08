#include <iostream>
#include <random>
#include "BPlusTree/BPlusTreeMap.h"
#include "BPlusTree/SortedArray.h"
#include "../lab_5/1/src/logger_builder_concrete.h"
#include "./allocators/default_memory.h"


int cmp(int const& a, int const& b)
{
	return a - b;
}

int main()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 1000);

	logger_builder* loggerBuilder = new logger_builder_concrete();
	logger* logger = loggerBuilder->add_stream("console", logger::severity::trace)->construct();


//	while (true) {
		BPlusTreeMap<int, Null> map(3, 3, cmp, std::make_shared<DefaultMemory>());
		int arrLen = 30;
		int arr[arrLen];
		int count = 0;
		for (int x = 0; x < arrLen; x++)
		{
			int num = dis(gen);
			arr[count] = num;
			count++;
			map.checkCorrectness();
			for (int x = 0; x < count; x++) {
				std::cout << arr[x] << ", ";
			}
			std::cout << std::endl;
			map.add(num, Null::value());
			map.print();
		}
		map.print();
		auto iter = map.begin();
		for (int x = 0; x < arrLen; x++)
		{
			std::cout << iter.getPair().getKey() << "; ";
			iter += 1;
		}
//		for (int x = 0; x < arrLen; x++)
//		{
//			int num = arr[x];
//			std::cout << "DELETE: " << num << std::endl;
//			map.remove(num);
//			map.checkCorrectness();
//			map.print();
//		}
//	}

//	int arr[] = {33, 98, 74, 98, 18, 52, 94, 29, 4, 82, 64, 11};
//	for (int x = 0; x < 11; x++) {
//		int num = arr[x];
//		map.add(num, Null::value());
//		map.print();
//	}
//
//	map.add(11, Null::value());
//	map.print();



//	for (int x = 0; x < arrLen; x++) {
//		int num = arr[x];
////		std::cout << "DELETE: " << num << std::endl;
//		map.remove(num);
//		map.checkCorrectness();
////		map.print();
//	}





//	BPlusTreeMap<int, int> map(4, 2, cmp, std::make_shared<DefaultMemory>());
//	int arr[] = {292, 138, 116, 19, 69, 93, 78, 80, 114, 241, 167, 218, 280, 213, 159, 138, 54, 53, 20, 263, 133, 123, 218, 262, 164, 247, 101, 213, 6,
//			6, 20, 241, 55, 55, 241, 31, 155, 6, 51, 217, 136, 298, 166, 120, 189, 47, 4, 106, 250, 182, 119, 292, 258, 250, 53, 263, 295, 166, 189, 262, 15,
//			4, 2, 9, 118, 39, 202, 213, 52, 15, 71, 127, 200, 111, 128, 211, 236, 222, 152, 29, 266, 50, 135, 240, 232, 84, 89, 190, 220, 271, 49, 81, 113,
//			248, 226, 281, 142, 59, 82, 73, 257, 47,
//	};
//	int arrLen = 100;
//	for (int x = 0; x < arrLen; x++)
//	{
//		int num = arr[x];
//		map.add(num, num);
//	}
//	map.print();
//
//	for (int x = 0; x < arrLen; x++)
//	{
//		int num = arr[x];
//		if (num == 52)
//			break;
//		std::cout << "DELETE: " << num << std::endl;
//		map.remove(num);
//		map.checkCorrectness();
//		map.print();
//	}
//
//	map.remove(52);
//	map.print();




	// ADDITION TEST:


//	int count = 0;
//	while (true) {
//		BPlusTreeMap<int, int> map(3, 4, cmp, std::make_shared<DefaultMemory>());
//		for (int x = 0; x < 100; x ++) {
//			int num = dis(gen);
//			std::cout << "|| ADD || " << num << std::endl;
//			map.add(num, num);
//			map.print();
//			map.checkCorrectness();
//		}
//		count++;
//		if (count % 100 == 0)
//			std::cout << count << std::endl;
//	}

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