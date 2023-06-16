#include <random>
#include <algorithm>
#include "logger/logger.h"
#include "logger/logger_builder_concrete.h"
#include "memory_4.h"


int main()
{
	logger_builder* builder = new logger_builder_concrete();

	logger* logger = builder
			->add_stream("console", logger::severity::trace)
			->construct();

	int numCount = 30;

	memory_4 memory4(240 * sizeof(int), memory_4::allocation_method::worst, nullptr, logger);

	// Создание генератора случайных чисел
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 100);

while (true)
{
	// Создание и заполнение map с рандомными числами и их адресами
	std::map<int, void*> numberMemoryMap;
	for (int i = 0; i <= numCount; ++i)
	{
		int randomNumber = dist(gen);

		while (numberMemoryMap.find(randomNumber) != numberMemoryMap.end())
		{
			randomNumber = dist(gen);
		}
		// Выделение памяти и сохранение адреса
		void* allocatedMemory = memory4.allocate(sizeof(int));
		if (allocatedMemory == nullptr)
			throw std::runtime_error("Not enough mem in alloc");
		int* memoryInteger = reinterpret_cast<int*>(allocatedMemory);
		*memoryInteger = randomNumber;
		numberMemoryMap[randomNumber] = allocatedMemory;
	}

	// Перемешивание порядка освобождения чисел
	std::vector<int> numbers;
	for (const auto& pair : numberMemoryMap)
	{
		numbers.push_back(pair.first);
	}
	std::shuffle(numbers.begin(), numbers.end(), gen);

	// Освобождение чисел в рандомном порядке и проверка валидности
	for (int number : numbers)
	{
		auto it = numberMemoryMap.find(number);
		if (it != numberMemoryMap.end())
		{
			void* memoryAddress = it->second;

			for (auto & it : numberMemoryMap)
			{
				void* memoryAddress = it.second;
				int* memoryInteger = reinterpret_cast<int*>(memoryAddress);
				if (it.first != *memoryInteger)
				{
					throw std::runtime_error("Invalid");
				}
			}

			std::cout << "DELETE: " << it->first << std::endl;

			// Освобождение памяти
			memory4.deallocate(memoryAddress);
			numberMemoryMap.erase(it);
		}
	}
}


	delete logger;
	delete builder;

	return 0;
}
