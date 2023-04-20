#ifndef MAIN_C_SRC_MEMORY_3_H
#define MAIN_C_SRC_MEMORY_3_H

#include "memory.h"
#include "logger/logger.h"
#include "logger/logger_builder.h"


class memory_2 : public memory
{
public:

	const logger* logger;

	virtual void* allocate(size_t target_size) const override;

	virtual void deallocate(void const* const target_to_dealloc) const override;

	memory_2(memory_2 const&) = delete;

	memory_2& operator=(memory_2 const&) = delete;

	memory_2();

	~memory_2() override;
};

#endif //MAIN_C_SRC_MEMORY_3_H
