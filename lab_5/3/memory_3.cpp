#include "memory_3.h"
#include <cstdlib>

memory_3::memory_3() {}

memory_3::~memory_3() {}

void* memory_3::allocate(size_t target_size) const {
	return std::malloc(target_size);
}

void memory_3::deallocate(void const* const target_to_dealloc) const {
	std::free(const_cast<void*>(target_to_dealloc));
}
