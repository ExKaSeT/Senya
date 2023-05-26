#ifndef PROGC_SRC_BPLUSTREE_ALLOCATORS_DEFAULT_MEMORY_H
#define PROGC_SRC_BPLUSTREE_ALLOCATORS_DEFAULT_MEMORY_H

#include <new>
#include "memory.h"


class DefaultMemory : public Memory
{

public:

	void* allocate(size_t target_size) const override {
		void *mem;
		try
		{
			mem = ::operator new(target_size);
		}
		catch (std::bad_alloc& e)
		{
			throw e;
		}
		return mem;
	}

	void deallocate(void* const target_to_dealloc) const override {
		::operator delete(target_to_dealloc);
	}

	DefaultMemory() = default;

	DefaultMemory(DefaultMemory const&) = delete;

	DefaultMemory& operator=(DefaultMemory const&) = delete;
};

#endif //PROGC_SRC_BPLUSTREE_ALLOCATORS_DEFAULT_MEMORY_H
