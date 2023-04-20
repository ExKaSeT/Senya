#include "logger/logger.h"
#include "memory_2.h"
#include "memory.h"

int main()
{
	memory_2 memory;
	int *num = reinterpret_cast<int *>(memory.allocate(sizeof(int)));
	*num = 10;
	printf("%d\n", *num);
	memory.deallocate(num);
	return 0;
}
