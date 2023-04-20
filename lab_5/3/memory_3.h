#ifndef MAIN_C_SRC_MEMORY_3_H
#define MAIN_C_SRC_MEMORY_3_H

#include "memory.h"

class memory_3 : public memory
{
public:

	memory_3();

	virtual void* allocate(size_t target_size) const override;

	virtual void deallocate(void const* const target_to_dealloc) const override;

	memory_3(memory_3 const&) = delete;

	memory_3& operator=(memory_3 const&) = delete;

	~memory_3() override;
};

#endif //MAIN_C_SRC_MEMORY_3_H
