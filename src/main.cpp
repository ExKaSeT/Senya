#include <iostream>
#include <random>
#include "BPlusTree/BPlusTreeMap.h"
#include "BPlusTree/SortedArray.h"
#include "../lab_5/1/src/logger_builder_concrete.h"
#include "BPlusTree/allocators/default_memory.h"

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

//	BPlusTreeMap<int, int> map(3, 4, cmp, std::make_shared<DefaultMemory>());
//	int arr[1000];
//	int count = 0;
//	for (int x = 0; x < 1000; x++)
//	{
//		int num = dis(gen);
//		arr[count] = num;
//		count++;
//		map.add(num, num);
//	}
////	std::cout << "___________";
////	for (int x = 0; x < 100; x++) {
////		std::cout << arr[x] << ", ";
////	}
//	std::cout << std::endl;
////	map.print();
//	map.checkCorrectness();
//	for (int x = 0; x < 1000; x++) {
//		int num = arr[x];
////		std::cout << "DELETE: " << num << std::endl;
//		map.remove(num);
//		map.checkCorrectness();
////		map.print();
//	}
////	map.print();



//	BPlusTreeMap<int, int> map(3, 4, cmp, std::make_shared<DefaultMemory>());
//	int arr[] = { 21, 93, 58, 42, 53, 91, 90, 89, 31, 71, 82, 94, 63, 100, 29, 90, 62, 72, 53, 90, 51, 69, 8, 30, 7, 85, 77, 24, 19, 75, };
//	for (int x = 0; x < 30; x++)
//	{
//		int num = arr[x];
//		map.add(num, num);
//	}
//	map.print();
//
//	int arr1[] = { 94, 67, 76, 5, 29, 40, 81, 17, 44, 88, 30, 38,   10, 46,  12, 43, 53, 18,  9, 2 };
//	for (int x = 0; x < 30; x++)
//	{
//		int num = arr[x];
//		std::cout << "DELETE: " << num << std::endl;
//		map.remove(num);
//		map.checkCorrectness();
//		map.print();
//	}



	// ADDITION TEST:
	//TODO:: check all memcpy, test addition, repair deletion (error)
	BPlusTreeMap<int, int> map(5, 2, cmp, std::make_shared<DefaultMemory>());
	int arr[100];
	int count = 0;
	for (int x = 0; x < 100; x++) {
		int num = dis(gen);
		arr[count] = num;
		count++;
		for (int x = 0; x < count; x++)
			std::cout << arr[x] << ", ";
		std::cout << std::endl;
		map.add(num, num);
		map.print();
	}


//	BPlusTreeMap<int, int> map(5, 2, cmp, std::make_shared<DefaultMemory>());
//	int arr[] = {591, 188, 897, 711, 976, 830, 461, 744, 44, 653, 944, 124, 991, 651, 752, 689, 636, 201, 463};
//	for (int x = 0; x < 18; x++) {
//		int num = arr[x];
//		std::cout << "ADD " << num << std::endl;
//		map.add(num, num);
////		map.checkCorrectness();
//		map.print();
//	}
//	map.add(463, 463);
//	map.print();

//---------------- delete higher



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