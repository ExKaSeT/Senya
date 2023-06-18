#include <random>
#include <algorithm>
#include <list>
#include "logger/logger.h"
#include "logger/logger_builder_concrete.h"
#include "memory_3.h"
#include "memory_4.h"
#include "memory_5.h"


int main()
{
	logger_builder* builder1 = new logger_builder_concrete();

	logger* logger0 = builder1
			->add_stream("log.txt", logger::severity::trace)
			->construct();

	logger_builder* builder2 = new logger_builder_concrete();
	logger* logger1 = builder2
			->add_stream("console", logger::severity::trace)
			->construct();

	logger_builder* builder3 = new logger_builder_concrete();
	logger* logger2 = builder3
			->add_stream("log2.txt", logger::severity::trace)
			->construct();

	memory *allocator1 = new memory_5(100000000, nullptr, logger0);
	memory *allocator2 = new memory_3(1000000, memory_3::allocation_method::first, nullptr, logger1);
	memory *allocator3 = new memory_4(99990, memory_4::allocation_method::best, nullptr, logger2);

	std::list<void*> allocated_blocks;

	srand((unsigned)time(nullptr));
	memory *alc = nullptr;

	for (size_t i = 0; i < 20000; ++i)
	{
		void * ptr;

		switch (rand() % 3)
		{
		case 0:
			alc = allocator2;
			break;
		case 1:
			alc = allocator2;
			break;
		case 2:
			alc = allocator2;
			break;
		}

		switch (rand() % 2)
		{
		case 0:
			try
			{
				ptr = reinterpret_cast<void *>(alc->allocate(rand() % 81 + 20)); // разность макс и мин с включенными границами + минимальное
				allocated_blocks.push_back(ptr);
			}
			catch (std::exception const &ex)
			{
				std::cout << ex.what() << std::endl;
			}
			break;
		case 1:

			if (allocated_blocks.empty())
			{
				break;
			}

			try
			{
				auto iter = allocated_blocks.begin();
				std::advance(iter, rand() % allocated_blocks.size());
				alc->deallocate(*iter);
				allocated_blocks.erase(iter);
			}
			catch (std::exception const &ex)
			{
				std::cout << ex.what() << std::endl;
			}
			break;
		}

		//std::cout << "iter # " << i + 1 << " finish" << std::endl;
	}

	while (!allocated_blocks.empty())
	{
		switch (rand() % 3)
		{
		case 0:
			alc = allocator2;
			break;
		case 1:
			alc = allocator2;
			break;
		case 2:
			alc = allocator2;
			break;
		}

		try
		{
			auto iter = allocated_blocks.begin();
			alc->deallocate(*iter);
			allocated_blocks.erase(iter);
		}
		catch (std::exception const &ex)
		{
			std::cout << ex.what() << std::endl;
		}
	}

	delete allocator3;
	delete allocator2;
	delete allocator1;
	delete logger0;
	delete builder1;
}
