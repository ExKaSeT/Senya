#ifndef MAIN_C_SRC_MEMORY_4_H
#define MAIN_C_SRC_MEMORY_4_H


#include <sstream>
#include "memory.h"
#include "logger/logger.h"


class memory_4 : public memory
{
public:

	enum allocation_method
	{
		first, best, worst
	};

private:
	// available block: -
	// occupied block: size_t | prev_occupied_ptr | next_occupied_ptr

	// data: method | alloc_ptr | logger_ptr | first_occupied_ptr | end_memory_ptr (excluding) | ...
	char* data;

	bool has_logger() const
	{
		char* current = data;
		current += (sizeof(allocation_method) + sizeof(memory*));
		logger* log_ = *reinterpret_cast<logger**>(current);
		return log_ != nullptr;
	};

	void log(std::string const& str, logger::severity severity) const
	{
		char* current = data;
		current += (sizeof(allocation_method) + sizeof(memory*));
		logger* log_ = *reinterpret_cast<logger**>(current);
		if (log_ == nullptr)
			return;
		log_->log(str, severity);
	}

	size_t get_service_block_size() const
	{
		return sizeof(allocation_method) + sizeof(memory*) + sizeof(logger*) + 2 * sizeof(void**);
	}

	allocation_method get_alloc_method() const
	{
		return *reinterpret_cast<allocation_method*>(data);
	}

	void** get_first_occupied_block_address_address() const
	{
		return reinterpret_cast<void**>(data + sizeof(allocation_method) + sizeof(memory*) + sizeof(logger*));
	}

	void* get_memory_end() const
	{
		return *reinterpret_cast<void**>(data + sizeof(allocation_method) + sizeof(memory*) + sizeof(logger*) +
									   sizeof(void**));
	}

	void* get_first_occupied_block_address() const
	{
		return *get_first_occupied_block_address_address();
	}

	size_t get_occupied_block_service_size() const
	{
		return sizeof(size_t) + 2 * sizeof(void**);
	};

	size_t get_block_size(void* occupied_block) const
	{
		return *reinterpret_cast<size_t*>(occupied_block);
	}

	void** get_next_block_address_address(void* occupied_block) const
	{
		return reinterpret_cast<void**>(reinterpret_cast<char*>(occupied_block) + sizeof(size_t) + sizeof(void**));
	}

	void* get_next_block_address(void* occupied_block) const
	{
		return *get_next_block_address_address(occupied_block);
	}

	void** get_prev_block_address_address(void* occupied_block) const
	{
		return reinterpret_cast<void**>(reinterpret_cast<char*>(occupied_block) + sizeof(size_t));
	}

	void* get_prev_block_address(void* occupied_block) const
	{
		return *get_prev_block_address_address(occupied_block);
	}

public:

	memory_4(size_t data_size, allocation_method method, memory* alloc = nullptr, logger* log = nullptr)
	{
		if (data_size < get_service_block_size() + get_occupied_block_service_size())
			throw std::bad_alloc();

		if (alloc == nullptr)
		{
			try
			{
				data = reinterpret_cast<char*>(::operator new(data_size));
			}
			catch (...)
			{
				this->log("Alloc error", logger::severity::error);
				throw;
			}
		}
		else
			data = reinterpret_cast<char*>(alloc->allocate(data_size));

		if (data == nullptr)
		{
			this->log("Alloc error", logger::severity::error);
			throw std::bad_alloc();
		}

		char* current = data;

		auto* alloc_method = reinterpret_cast<allocation_method*>(current);
		*alloc_method = method;
		current += sizeof(allocation_method);

		auto** alloc_ptr = reinterpret_cast<memory**>(current);
		*alloc_ptr = alloc;
		current += sizeof(memory*);

		auto** log_ = reinterpret_cast<logger**>(current);
		*log_ = log;
		current += sizeof(logger*);

		auto** first_occupied_block = reinterpret_cast<void**>(current);
		*first_occupied_block = nullptr;
		current += sizeof(void**);

		auto** end_memory = reinterpret_cast<void**>(current);
		*end_memory = data + data_size;

		if (log != nullptr)
		{
			current += sizeof(void**);
			std::stringstream log_stream;
			log_stream << "Allocated " << data_size - get_service_block_size() << " bytes [ " << current - data
					   << " ] ";
			this->log(log_stream.str(), logger::severity::information);
		}
	}

	~memory_4() override
	{
		char* current = data;
		current += sizeof(allocation_method);
		memory* alloc = *reinterpret_cast<memory**>(current);
		if (alloc == nullptr)
			::operator delete(data);
		else
			alloc->deallocate(data);
	}

	void* allocate(size_t requested_size) const override
	{
		char* const memory_end = reinterpret_cast<char*>(get_memory_end());
		char* const memory_begin = data + get_service_block_size();
		char* previous_block = nullptr;
		char* current_block = reinterpret_cast<char*>(get_first_occupied_block_address());
		char* target_block = nullptr;
		char* previous_to_target_block = nullptr;
		char* next_to_target_block = nullptr;
		auto alloc_method = get_alloc_method();
		size_t target_block_size;
		if (alloc_method == best)
			target_block_size = SIZE_MAX;
		else if (alloc_method == worst)
			target_block_size = 0;

		bool allocation_ended = false;

		if (current_block == nullptr) // no occupied blocks
		{
			size_t available_block_size = memory_end - memory_begin;
			if (available_block_size >= requested_size + get_occupied_block_service_size())
			{
				target_block = memory_begin;
			}
			allocation_ended = true;
		}
		// check memory between memory begin and first occupied block
		if (!allocation_ended && memory_begin != current_block)
		{
			size_t available_block_size = current_block - memory_begin;
			if (available_block_size >= requested_size + get_occupied_block_service_size())
			{
				target_block_size = available_block_size;
				target_block = memory_begin;
				next_to_target_block = current_block;
			}
		}

		if (!allocation_ended)
		{
			previous_block = current_block;
			current_block = reinterpret_cast<char*>(get_next_block_address(current_block));
		}
		while (current_block != nullptr)
		{
			size_t available_block_size = current_block - (previous_block + get_occupied_block_service_size() +
														   get_block_size(previous_block));

			if (available_block_size >= requested_size + get_occupied_block_service_size())
			{
				if (alloc_method == first ||
					alloc_method == best &&
					(target_block == nullptr || available_block_size < target_block_size) ||
					alloc_method == worst &&
					(target_block == nullptr || available_block_size > target_block_size))
				{
					target_block_size = available_block_size;
					previous_to_target_block = previous_block;
					target_block = previous_block + get_occupied_block_service_size() +
								   get_block_size(previous_block);
					next_to_target_block = current_block;
				}

				if (alloc_method == first)
				{
					allocation_ended = true;
					break;
				}
			}
			previous_block = current_block;
			current_block = reinterpret_cast<char*>(get_next_block_address(current_block));
		}
		// check memory between last occupied block and memory end
		if (!allocation_ended &&
			memory_end != previous_block + get_occupied_block_service_size() + get_block_size(previous_block))
		{
			size_t available_block_size =
					memory_end - (previous_block + get_occupied_block_service_size() + get_block_size(previous_block));
			if (available_block_size >= requested_size + get_occupied_block_service_size())
			{
				if (alloc_method == first ||
					alloc_method == best &&
					(target_block == nullptr || available_block_size < target_block_size) ||
					alloc_method == worst &&
					(target_block == nullptr || available_block_size > target_block_size))
				{
					target_block_size = available_block_size;
					previous_to_target_block = previous_block;
					target_block = previous_block + get_occupied_block_service_size() + get_block_size(previous_block);
					next_to_target_block = nullptr;
				}
			}
		}

		if (target_block == nullptr)
		{
			this->log("Not available memory", logger::severity::error);
			throw std::bad_alloc();
		}

		auto* target_size = reinterpret_cast<size_t*>(target_block);
		*target_size = requested_size;

		auto** target_previous_block = reinterpret_cast<void**>(target_size + 1);
		*target_previous_block = previous_to_target_block;

		auto** target_next_block = target_previous_block + 1;
		*target_next_block = next_to_target_block;

		if (previous_to_target_block == nullptr)
		{
			*get_first_occupied_block_address_address() = target_block;
		}
		else
		{
			auto** previous_block_next_block = reinterpret_cast<void**>(previous_to_target_block + sizeof(size_t) +
																		sizeof(void**));
			*previous_block_next_block = target_block;
		}

		if (next_to_target_block != nullptr)
		{
			auto** next_block_previous_block = reinterpret_cast<void**>(next_to_target_block + sizeof(size_t));
			*next_block_previous_block = target_block;
		}

		if (this->has_logger())
		{
			char* ptr = target_block + get_occupied_block_service_size();
			std::stringstream log_stream;
			if (alloc_method == first)
				log_stream << "Allocated [FIRST] [ " << ptr - data << " ] ";
			else if (alloc_method == best)
				log_stream << "Allocated [BEST] [ " << ptr - data << " ] ";
			else
				log_stream << "Allocated [WORST] [ " << ptr - data << " ] ";
			this->log(log_stream.str(), logger::severity::information);
		}

		return reinterpret_cast<void*>(target_block + get_occupied_block_service_size());
	}

	void deallocate(void* target_to_dealloc) const override
	{
		*const_cast<void**>(&target_to_dealloc) = reinterpret_cast<void*>
		(reinterpret_cast<char*>(target_to_dealloc) - get_occupied_block_service_size());

		if (this->has_logger())
		{
			size_t block_size = *reinterpret_cast<size_t*>(target_to_dealloc);
			std::stringstream log_stream;
			char* block_data = reinterpret_cast<char*>(target_to_dealloc) + sizeof(size_t);
			log_stream << "Deallocated [ " << block_data - data << " ]: ";
			auto* byte_ptr = reinterpret_cast<char*>(block_data);
			for (auto i = 0; i < block_size; i++)
			{
				log_stream << static_cast<unsigned short>(byte_ptr[i]) << " ";
			}
			this->log(log_stream.str(), logger::severity::information);
		}

		auto* previous_block = *reinterpret_cast<void**>(reinterpret_cast<char*>(target_to_dealloc) +
														 sizeof(size_t));
		auto* next_block = *reinterpret_cast<void**>(reinterpret_cast<char*>(target_to_dealloc) +
													 sizeof(size_t) +
													 sizeof(void**));
		if (previous_block == nullptr && next_block == nullptr)
		{
			*get_first_occupied_block_address_address() = nullptr;
		}
		else if (previous_block == nullptr)
		{
			*get_first_occupied_block_address_address() = next_block;
			auto** next_block_previous_block = reinterpret_cast<void**>(reinterpret_cast<size_t*>(next_block) + 1);
			*next_block_previous_block = nullptr;
		}
		else if (next_block == nullptr)
		{
			auto** previous_block_next_block = reinterpret_cast<void**>(reinterpret_cast<char*>(previous_block) +
																		sizeof(size_t) + sizeof(void**));
			*previous_block_next_block = nullptr;
		}
		else
		{
			auto** next_block_previous_block = reinterpret_cast<void**>(reinterpret_cast<size_t*>(next_block) + 1);
			*next_block_previous_block = previous_block;

			auto** previous_block_next_block = reinterpret_cast<void**>(reinterpret_cast<char*>(previous_block) +
																		sizeof(size_t) + sizeof(void**));
			*previous_block_next_block = next_block;
		}
	}
};


#endif //MAIN_C_SRC_MEMORY_4_H
