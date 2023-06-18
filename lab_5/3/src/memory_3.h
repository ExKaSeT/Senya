#ifndef MAIN_C_SRC_MEMORY_3_H
#define MAIN_C_SRC_MEMORY_3_H


#include <sstream>
#include "memory.h"
#include "logger/logger.h"


class memory_3 : public memory
{
public:

	enum allocation_method
	{
		first, best, worst
	};

private:
	// available block: size_t | next_ptr
	// occupied block: size_t

	// data: method | alloc_ptr | logger_ptr | first_available_ptr | ...
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
		return sizeof(allocation_method) + sizeof(memory*) + sizeof(logger*) + sizeof(void**);
	}

	allocation_method get_alloc_method() const
	{
		return *reinterpret_cast<allocation_method*>(data);
	}

	void** get_first_block_address_address() const
	{
		return reinterpret_cast<void**>(data + sizeof(allocation_method) + sizeof(memory*) + sizeof(logger*));
	}

	void* get_first_block_address() const
	{
		return *get_first_block_address_address();
	}

	size_t get_available_block_service_size() const
	{
		return sizeof(size_t) + sizeof(void**);
	};

	size_t get_occupied_block_service_size() const
	{
		return sizeof(size_t);
	};

	size_t get_block_size(void* block_ptr) const
	{
		return *reinterpret_cast<size_t*>(block_ptr);
	}

	void** get_available_next_block_address_address(void* available_block_ptr) const
	{
		return reinterpret_cast<void**>(reinterpret_cast<char*>(available_block_ptr) + sizeof(size_t));
	}

	void* get_available_next_block_address(void* available_block_ptr) const
	{
		return *get_available_next_block_address_address(available_block_ptr);
	}

public:

	memory_3(size_t data_size, allocation_method method, memory* alloc = nullptr, logger* log = nullptr)
	{
		if (data_size < get_service_block_size() + get_available_block_service_size())
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

		auto** first_available_block = reinterpret_cast<void**>(current);
		current += sizeof(void*);
		*first_available_block = reinterpret_cast<void*>(current);

		size_t free_size;
		free_size = data_size - get_service_block_size();

		auto* block_size = reinterpret_cast<size_t*>(current);
		*block_size = free_size - get_available_block_service_size();
		current += sizeof(size_t);

		auto** next_ptr = reinterpret_cast<void**>(current);
		*next_ptr = nullptr;

		if (log != nullptr)
		{
			current += sizeof(size_t);
			std::stringstream log_stream;
			log_stream << "Allocated " << *block_size << " bytes [ " << current - data << " ] ";
			this->log(log_stream.str(), logger::severity::information);
		}
	}

	~memory_3() override
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
		if (requested_size < sizeof(void*)) // because after deallocate block has (size_t, void*)
			requested_size = sizeof(void*);

		void* previous_block = nullptr;
		void* current_block = get_first_block_address();
		void* target_block = nullptr;
		void* previous_to_target_block = nullptr;
		void* next_to_target_block = nullptr;
		auto alloc_method = get_alloc_method();

		while (current_block != nullptr)
		{
			auto current_block_size = get_block_size(current_block);
			auto* next_block = get_available_next_block_address(current_block);

			if (current_block_size + get_available_block_service_size() >=
				requested_size + get_occupied_block_service_size())
			{
				if (alloc_method == first ||
					alloc_method == best &&
					(target_block == nullptr || current_block_size < get_block_size(target_block)) ||
					alloc_method == worst &&
					(target_block == nullptr || current_block_size > get_block_size(target_block)))
				{
					previous_to_target_block = previous_block;
					target_block = current_block;
					next_to_target_block = next_block;
				}

				if (alloc_method == first)
					break;
			}
			previous_block = current_block;
			current_block = next_block;
		}

		if (target_block == nullptr)
		{
			this->log("Not available memory", logger::severity::error);
			throw std::bad_alloc();
		}

		bool is_requested_size_overridden = false;
		if (get_block_size(target_block) + get_available_block_service_size() - get_occupied_block_service_size() -
			requested_size < get_available_block_service_size())
		{
			// increased to the next block, because can`t split
			requested_size = get_block_size(target_block);
			is_requested_size_overridden = true;
		}

		void* update_next_block_in_previous;

		if (is_requested_size_overridden)
		{
			update_next_block_in_previous = next_to_target_block;
		}
		else
		{
			update_next_block_in_previous = reinterpret_cast<void*>
			(reinterpret_cast<char*>(target_block) + get_occupied_block_service_size() + requested_size);

			auto* target_block_leftover_size = reinterpret_cast<size_t*>(update_next_block_in_previous);
			*target_block_leftover_size =
					get_block_size(target_block) - get_occupied_block_service_size() - requested_size;

			auto* target_block_leftover_next_block_address = reinterpret_cast<void**>(target_block_leftover_size + 1);
			*target_block_leftover_next_block_address = next_to_target_block;
		}

		previous_to_target_block == nullptr
		? *get_first_block_address_address() = update_next_block_in_previous
		: *reinterpret_cast<void**>
		(reinterpret_cast<char*>(previous_to_target_block) + sizeof(size_t)) = update_next_block_in_previous;

		auto* target_block_size_address = reinterpret_cast<size_t*>(target_block);
		*target_block_size_address = requested_size;

		if (this->has_logger())
		{
			char* ptr = reinterpret_cast<char*>(target_block_size_address + 1);
			std::stringstream log_stream;
			if (alloc_method == first)
				log_stream << "Allocated [FIRST] [ " << ptr - data << " ] ";
			else if (alloc_method == best)
				log_stream << "Allocated [BEST] [ " << ptr - data << " ] ";
			else
				log_stream << "Allocated [WORST] [ " << ptr - data << " ] ";
			this->log(log_stream.str(), logger::severity::information);
		}

		return reinterpret_cast<void*>(target_block_size_address + 1);
	}

	void deallocate(void* target_to_dealloc) const override
	{
		*const_cast<void**>(&target_to_dealloc) = reinterpret_cast<void*>
		(reinterpret_cast<size_t*>(target_to_dealloc) - 1);

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

		void* previous_block = nullptr;
		void* current_block = get_first_block_address();
		if (current_block == nullptr) // target is only one available
		{
			*get_first_block_address_address() = target_to_dealloc;

			auto* block_to_deallocate_size = reinterpret_cast<size_t*>(target_to_dealloc);
			*block_to_deallocate_size -= sizeof(void*);

			*reinterpret_cast<void**>(block_to_deallocate_size + 1) = nullptr;

			return;
		}
		while (current_block != nullptr)
		{
			if (target_to_dealloc < current_block)
			{
				previous_block == nullptr
				? *get_first_block_address_address() = target_to_dealloc
				: *reinterpret_cast<void**>(reinterpret_cast<size_t*>(previous_block) + 1) = target_to_dealloc;

				auto* block_to_deallocate_size = reinterpret_cast<size_t*>(target_to_dealloc);
				*block_to_deallocate_size -= sizeof(void*);

				*reinterpret_cast<void**>(block_to_deallocate_size + 1) = current_block;
				break;
			}
			previous_block = current_block;
			current_block = get_available_next_block_address(current_block);
		}
		if (current_block == nullptr) // target is last free block
		{
			*reinterpret_cast<void**>(reinterpret_cast<size_t*>(previous_block) + 1) = target_to_dealloc;
			auto* block_to_deallocate_size = reinterpret_cast<size_t*>(target_to_dealloc);
			*block_to_deallocate_size -= sizeof(void*);

			*reinterpret_cast<void**>(block_to_deallocate_size + 1) = nullptr;
		}
		else if (reinterpret_cast<char*>(target_to_dealloc) + get_available_block_service_size() +
				 get_block_size(target_to_dealloc) == current_block) // check and merge with right
		{
			void* next_block = get_available_next_block_address(current_block);
			auto* target_size = reinterpret_cast<size_t*>(target_to_dealloc);
			*target_size = get_block_size(target_to_dealloc) + get_block_size(current_block) +
						   get_available_block_service_size();
			auto** next_ptr = reinterpret_cast<void**>(target_size + 1);
			*next_ptr = next_block;
		}
		if (previous_block != nullptr &&
			reinterpret_cast<char*>(previous_block) + get_available_block_service_size() +
			get_block_size(previous_block) == target_to_dealloc) // check and merge with left
		{
			void* next_block = get_available_next_block_address(target_to_dealloc);
			auto* prev_block_size = reinterpret_cast<size_t*>(previous_block);
			*prev_block_size = get_block_size(previous_block) + get_block_size(target_to_dealloc) +
							   get_available_block_service_size();
			auto** prev_next_ptr = reinterpret_cast<void**>(prev_block_size + 1);
			*prev_next_ptr = next_block;
		}
	}
};


#endif //MAIN_C_SRC_MEMORY_3_H
