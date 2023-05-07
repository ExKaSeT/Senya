#include "logger/logger.h"
#include "logger/logger_builder.h"
#include "memory_2.h"
#include "memory.h"
#include "logger/logger_builder_concrete.h"

int main()
{
	logger_builder* builder = new logger_builder_concrete();

	logger* logger = builder
		->add_stream("console", logger::severity::trace)
		->construct();
    memory_2 memory(logger);
	int *num = reinterpret_cast<int *>(memory.allocate(10 * sizeof(int)));
	*num = 16777217 + 256 + 65536;
	printf("%d\n", *num);
	memory.deallocate(num);

	delete builder;
	delete logger;
	return 0;
}
