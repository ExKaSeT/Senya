#ifndef MEMORY_H
#define MEMORY_H

#include <cstddef>

class memory
{
public:
	virtual void* allocate(size_t target_size) const = 0;

	virtual void deallocate(void const* const target_to_dealloc) const = 0;

	virtual ~memory();
};

#endif // MEMORY_H