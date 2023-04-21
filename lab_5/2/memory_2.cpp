#include <sstream>
#include "memory_2.h"
#include "logger/logger_builder_concrete.h"
#include "logger/logger.h"

memory_2::memory_2()
{
	logger_builder_concrete builder;

	logger = builder
		.add_stream("console", logger::severity::trace)
		->construct();
}

memory_2::~memory_2()
{
	delete logger;
}

void* memory_2::allocate(size_t target_size) const
{
	void* mem;
	try
	{
		mem = ::operator new(target_size);
	}
	catch (std::bad_alloc& e)
	{
		throw e;
	}

	std::stringstream log;
	log << "Allocated " << target_size << " bytes [ " << mem << " ] ";
	logger->log(log.str(), logger::severity::debug);

	return mem;
}

void memory_2::deallocate(void* const target_to_dealloc) const
{
	std::stringstream log;
	log << "Freed [ " << target_to_dealloc << " ] : ";
	auto* bytePtr = static_cast<const unsigned char*>(target_to_dealloc);
	for (size_t i = 0; i < 25; i++)
	{
		log << static_cast<unsigned int>(bytePtr[i]) << " ";
	}
	logger->log(log.str(), logger::severity::debug);

	::operator delete(target_to_dealloc);
}
