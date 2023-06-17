#ifndef MAIN_C_SRC_MEMORY_4_H
#define MAIN_C_SRC_MEMORY_4_H


#include <sstream>
#include "memory.h"
#include "logger/logger.h"


class memory_5 : public memory
{
private:
	// available block: is_occupied | degree | prev_available_ptr | next_available_ptr
	// occupied block: is_occupied | degree

	// data: size_t | alloc_ptr | logger_ptr | first_available_ptr | ...
	char* data;

	size_t get_rounded_of_power_2(size_t number) const
	{
		return 1 << static_cast<size_t>(ceil(log2(number)));
	}

	short get_power(size_t number) const
	{
		return static_cast<short>(log2(number));
	}

	bool has_logger() const
	{
		char* current = data;
		current += (sizeof(size_t) + sizeof(memory*));
		logger* log_ = *reinterpret_cast<logger**>(current);
		return log_ != nullptr;
	};

	void log(std::string const& str, logger::severity severity) const
	{
		char* current = data;
		current += (sizeof(size_t) + sizeof(memory*));
		logger* log_ = *reinterpret_cast<logger**>(current);
		if (log_ == nullptr)
			return;
		log_->log(str, severity);
	}

	size_t get_service_block_size() const
	{
		return sizeof(size_t) + sizeof(memory*) + sizeof(logger*) + sizeof(void**);
	}

	size_t get_memory_size() const
	{
		return *reinterpret_cast<size_t*>(data);
	}

	void** get_first_available_block_address_address() const
	{
		return reinterpret_cast<void**>(data + sizeof(size_t) + sizeof(memory*) + sizeof(logger*));
	}

	void* get_first_available_block_address() const
	{
		return *get_first_available_block_address_address();
	}

	size_t get_available_block_service_size() const
	{
		return sizeof(bool) + sizeof(short) + 2 * sizeof(void**);
	};

	size_t get_occupied_block_service_size() const
	{
		return sizeof(bool) + sizeof(short);
	};

	short* get_block_power_address(void* block) const
	{
		return reinterpret_cast<short*>(reinterpret_cast<bool*>(block) + 1);
	}

	short get_block_power(void* block) const
	{
		return *get_block_power_address(block);
	}

	bool block_is_available(void* block) const
	{
		return !(*reinterpret_cast<bool*>(block));
	}

	void** get_next_block_address_address(void* block) const
	{
		return reinterpret_cast<void**>(reinterpret_cast<char*>(block) + sizeof(bool) + sizeof(short) +
										sizeof(void**));
	}

	void* get_next_block_address(void* block) const
	{
		return *get_next_block_address_address(block);
	}

	void** get_prev_block_address_address(void* block) const
	{
		return reinterpret_cast<void**>(reinterpret_cast<char*>(block) + sizeof(bool) + sizeof(short));
	}

	void* get_prev_block_address(void* block) const
	{
		return *get_prev_block_address_address(block);
	}

public:

	memory_5(size_t data_size, memory* alloc = nullptr, logger* log = nullptr)
	{
		if (data_size < get_service_block_size() + get_available_block_service_size())
			throw std::bad_alloc();

		size_t memory_size = get_rounded_of_power_2(data_size - get_service_block_size());
		data_size = memory_size + get_service_block_size();

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

		auto* memory_size_ = reinterpret_cast<size_t*>(current);
		*memory_size_ = memory_size;
		current += sizeof(size_t);

		auto** alloc_ptr = reinterpret_cast<memory**>(current);
		*alloc_ptr = alloc;
		current += sizeof(memory*);

		auto** log_ = reinterpret_cast<logger**>(current);
		*log_ = log;
		current += sizeof(logger*);

		auto** first_available_block = reinterpret_cast<void**>(current);
		current += sizeof(void**);
		*first_available_block = current;

		auto* block_is_occupied = reinterpret_cast<bool*>(current);
		*block_is_occupied = false;
		current += sizeof(bool);

		auto* block_power = reinterpret_cast<short*>(current);
		*block_power = get_power(memory_size);
		current += sizeof(short);

		auto** previous_block = reinterpret_cast<void**>(current);
		*previous_block = nullptr;
		current += sizeof(void**);

		auto** next_block = reinterpret_cast<void**>(current);
		*next_block = nullptr;

		if (log != nullptr)
		{
			current += sizeof(void**);
			std::stringstream log_stream;
			log_stream << "Allocated " << data_size - get_service_block_size() << " bytes [ " << current - data
					   << " ] ";
			this->log(log_stream.str(), logger::severity::information);
		}
	}

	~memory_5() override
	{
		char* current = data;
		current += sizeof(size_t);
		memory* alloc = *reinterpret_cast<memory**>(current);
		if (alloc == nullptr)
			::operator delete(data);
		else
			alloc->deallocate(data);
	}

	void* allocate(size_t requested_size) const override
	{
		// after dealloc block additionally has 2 * sizeof(void**)
		size_t size_to_allocate = requested_size < 2 * sizeof(void**) ? 2 * sizeof(void**) : requested_size;
		size_to_allocate += get_occupied_block_service_size();

		void* current_available_block = get_first_available_block_address();

		while (current_available_block != nullptr)
		{
			short power = get_block_power(current_available_block);

			if ((1 << power) >= size_to_allocate)
			{
				void* prev_available_block = get_prev_block_address(current_available_block);
				void* next_available_block = get_next_block_address(current_available_block);

				while (((1 << power) >> 1) > size_to_allocate)
				{
					power--;

					void* buddie = reinterpret_cast<void*>(reinterpret_cast<char*>(current_available_block) +
														   (1 << power));
					auto* buddie_is_occupied = reinterpret_cast<bool*>(buddie);

					if (next_available_block != nullptr)
						*get_prev_block_address_address(next_available_block) = buddie;

					*buddie_is_occupied = false;
					*get_block_power_address(buddie) = power;
					*get_prev_block_address_address(buddie) = current_available_block;
					*get_next_block_address_address(buddie) = next_available_block;

					*get_block_power_address(current_available_block) = power;
					*get_next_block_address_address(current_available_block) = buddie;

					next_available_block = buddie;
				}

				auto* current_is_occupied = reinterpret_cast<bool*>(current_available_block);
				*current_is_occupied = true;

				if (prev_available_block == nullptr)
				{
					auto** first_available_block = get_first_available_block_address_address();
					*first_available_block = next_available_block;
				}
				else
				{
					*get_next_block_address_address(prev_available_block) = next_available_block;
				}

				if (next_available_block != nullptr)
					*get_prev_block_address_address(next_available_block) = prev_available_block;

				if (this->has_logger())
				{
					char* ptr = reinterpret_cast<char*>(get_block_power_address(current_available_block) + 1);
					std::stringstream log_stream;
					log_stream << "Allocated " << size_to_allocate - get_occupied_block_service_size() << " bytes [ "
							   << ptr - data << " ] ";
					this->log(log_stream.str(), logger::severity::information);
				}

				return get_block_power_address(current_available_block) + 1;
			}

			current_available_block = get_next_block_address(current_available_block);
		}

		this->log("Not available memory", logger::severity::error);
		throw std::bad_alloc();
	}

	void* get_buddie(void* block) const
	{
		if (1 << get_block_power(block) == *reinterpret_cast<size_t*>(data))
			return nullptr;

		void* begin_address = data + get_service_block_size();

		size_t block_size = 1 << get_block_power(block);
		size_t relative_address = reinterpret_cast<char*>(block) - reinterpret_cast<char*>(begin_address);
		size_t result_xor = relative_address ^ block_size;

		return reinterpret_cast<void*>(reinterpret_cast<char*>(begin_address) + result_xor);
	}

	void deallocate(void* target_to_dealloc) const override
	{
		*const_cast<void**>(&target_to_dealloc) = reinterpret_cast<void*>
		(reinterpret_cast<char*>(target_to_dealloc) - get_occupied_block_service_size());

		if (this->has_logger())
		{
			size_t block_size = get_rounded_of_power_2(get_block_power(target_to_dealloc))
								- get_occupied_block_service_size();
			std::stringstream log_stream;
			char* block_data = reinterpret_cast<char*>(target_to_dealloc) + get_occupied_block_service_size();
			log_stream << "Deallocated [ " << block_data - data << " ]: ";
			auto* byte_ptr = reinterpret_cast<char*>(block_data);
			for (auto i = 0; i < block_size; i++)
			{
				log_stream << static_cast<unsigned short>(byte_ptr[i]) << " ";
			}
			this->log(log_stream.str(), logger::severity::information);
		}

		char* current_block = reinterpret_cast<char*>(target_to_dealloc);
		char* prev_block = nullptr;
		char* next_block = reinterpret_cast<char*>(get_first_available_block_address());

		while (next_block != nullptr && current_block > next_block)
		{
			prev_block = next_block;
			next_block = reinterpret_cast<char*>(get_next_block_address(next_block));
		}

		auto* current_is_occupied = reinterpret_cast<bool*>(current_block);
		*current_is_occupied = false;
		*get_prev_block_address_address(current_block) = prev_block;
		*get_next_block_address_address(current_block) = next_block;

		if (prev_block == nullptr)
		{
			auto** first_available_block = get_first_available_block_address_address();
			*first_available_block = current_block;
		}
		else
		{
			*get_next_block_address_address(prev_block) = current_block;
		}

		if (next_block != nullptr)
			*get_prev_block_address_address(next_block) = current_block;

		char* buddie = reinterpret_cast<char*>(get_buddie(current_block));

		while (buddie != nullptr && block_is_available(buddie) &&
			   get_block_power(buddie) == get_block_power(current_block))
		{
			if (buddie < current_block)
			{
				auto* temp = buddie;
				buddie = current_block;
				current_block = temp;
			}

			void* next_block_buddie = get_next_block_address(buddie);
			*get_next_block_address_address(current_block) = next_block_buddie;
			(*get_block_power_address(current_block))++;

			if (next_block_buddie != nullptr)
				*get_prev_block_address_address(next_block_buddie) = current_block;

			buddie = reinterpret_cast<char*>(get_buddie(current_block));
		}
	}
};


#endif //MAIN_C_SRC_MEMORY_4_H
