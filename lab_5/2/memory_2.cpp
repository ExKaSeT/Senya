#include <sstream>
#include "memory_2.h"
#include "logger/logger.h"

memory_2::memory_2(class logger* logger)
{
	this->logger = logger;
}

memory_2::~memory_2() = default;

void* memory_2::allocate(size_t target_size) const
{
	void* mem;
	try
	{
		mem = ::operator new(target_size + sizeof(size_t));
        *reinterpret_cast<size_t *>(mem) = target_size;
	}
	catch (std::bad_alloc& e)
	{
        logger->log("Can't allocate memory", logger::severity::error);
		throw e;
	}

	std::stringstream log;
	log << "Allocated " << target_size << " bytes [ " << mem << " ] ";
	logger->log(log.str(), logger::severity::debug);

    // return reinterpret_cast<unsigned char *>(mem) + sizeof(size_t);
	return reinterpret_cast<size_t *>(mem) + 1;
}

void memory_2::deallocate(void * const target_to_dealloc) const
{
	std::stringstream log;
	log << "Freed [ " << target_to_dealloc << " ] : ";
	auto* bytePtr = reinterpret_cast<unsigned char*>(target_to_dealloc);
    auto const block_size = *(reinterpret_cast<size_t *>(target_to_dealloc) - 1);
	for (auto i = 0; i < block_size; i++)
	{
		log << static_cast<unsigned short>(bytePtr[i]) << " ";
	}
	logger->log(log.str(), logger::severity::debug);

	::operator delete(reinterpret_cast<unsigned char *>(target_to_dealloc) - sizeof(size_t));
}
