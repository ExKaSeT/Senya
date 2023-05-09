#include <sstream>
#include "memory_2.h"


Memory_2::Memory_2(class logger* logger) : logger(logger)
{}

Memory_2::Memory_2() : logger(nullptr)
{}

Memory_2::~Memory_2() = default;

void* Memory_2::allocate(size_t target_size) const
{
	void* mem;
	try
	{
		mem = ::operator new(target_size + sizeof(size_t));
		*reinterpret_cast<size_t*>(mem) = target_size;
	}
	catch (std::bad_alloc& e)
	{
		if (logger != nullptr)
			logger->log("Can't allocate memory", logger::severity::error);
		throw e;
	}

	if (logger != nullptr)
	{
		std::stringstream log;
		log << "Allocated " << target_size << " bytes [ " << mem << " ] ";
		logger->log(log.str(), logger::severity::debug);
	}

	// return reinterpret_cast<unsigned char *>(mem) + sizeof(size_t);
	return reinterpret_cast<size_t*>(mem) + 1;
}

void Memory_2::deallocate(void* const target_to_dealloc) const
{
	void *mem_to_dealloc = reinterpret_cast<void *>(reinterpret_cast<unsigned char*>(target_to_dealloc) - sizeof(size_t));

	if (logger != nullptr)
	{
		std::stringstream log;
		log << "Freed [ " << mem_to_dealloc << " ] : ";
		auto* bytePtr = reinterpret_cast<unsigned char*>(target_to_dealloc);
		auto const block_size = *(reinterpret_cast<size_t*>(target_to_dealloc) - 1);
		for (auto i = 0; i < block_size; i++)
		{
			log << static_cast<unsigned short>(bytePtr[i]) << " ";
		}
		logger->log(log.str(), logger::severity::debug);
	}

	::operator delete(mem_to_dealloc);
}
