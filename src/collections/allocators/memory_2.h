#ifndef MAIN_C_SRC_MEMORY_2_H
#define MAIN_C_SRC_MEMORY_2_H

#include "memory.h"
#include "../../loggers/logger.h"
#include "../../loggers/ostream_logger/logger_builder.h"

class Memory_2 : public Memory
{
private:

	const logger* logger;

public:

	void* allocate(size_t target_size) const override;

	void deallocate(void* const target_to_dealloc) const override;

	Memory_2(Memory_2 const&) = delete;

	Memory_2& operator=(Memory_2 const&) = delete;

	explicit Memory_2(class logger* logger);

	Memory_2();

	~Memory_2() override;
};

#endif //MAIN_C_SRC_MEMORY_2_H
