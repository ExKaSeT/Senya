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

	size_t get_rounded_of_power_2(size_t number)
	{
		return 1 << static_cast<size_t>(ceil(log2(number)));
	}

	short get_power(size_t number)
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

	short get_block_power(void* block) const
	{
		return *reinterpret_cast<short*>(reinterpret_cast<bool*>(block) + 1);
	}

	bool block_is_available(void* block) const
	{
		return !(*reinterpret_cast<bool*>(block));
	}

	void** get_next_block_address_address(void* block) const
	{
		return reinterpret_cast<void**>(reinterpret_cast<char*>(block) + sizeof(bool) + sizeof(short) + sizeof(void**));
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
		if (data_size < get_service_block_size())
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
		size_t size_to_allocate = requested_size + get_available_block_service_size();
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

					auto* space_is_occupied_current = reinterpret_cast<bool*>(current_available_block);
					auto* space_power_current = reinterpret_cast<short*>(space_is_occupied_current + 1);
					auto** prev_block_current = reinterpret_cast<void**>(space_power_current + 1);
					auto** next_block_current = reinterpret_cast<void**>(prev_block_current + 1);

					void* buddie = reinterpret_cast<void*>(reinterpret_cast<char*>(current_available_block) +
														   (1 << power));

					auto* space_is_occupied_buddie = reinterpret_cast<bool*>(buddie);
					auto* space_power_buddie = reinterpret_cast<short*>(space_is_occupied_buddie + 1);
					auto** prev_block_buddie = reinterpret_cast<void**>(space_power_buddie + 1);
					auto** next_block_buddie = reinterpret_cast<void**>(prev_block_buddie + 1);

					if (next_available_block != nullptr)
					{
						auto* space_is_occupied_next = reinterpret_cast<bool*>(next_available_block);
						auto* space_power_next = reinterpret_cast<short*>(space_is_occupied_next + 1);
						auto** prev_block_next = reinterpret_cast<void**>(space_power_next + 1);

						*prev_block_next = buddie;
					}

					*space_is_occupied_buddie = false;
					*space_power_buddie = power;
					*prev_block_buddie = current_available_block;
					*next_block_buddie = next_available_block;

					*space_power_current = power;
					*next_block_current = buddie;

					next_available_block = buddie;
				}

				auto* space_is_occupied_current = reinterpret_cast<bool*>(current_available_block);
				auto* space_power_current = reinterpret_cast<short*>(space_is_occupied_current + 1);
				auto** prev_block_current = reinterpret_cast<void**>(space_power_current + 1);
				auto** next_block_current = reinterpret_cast<void**>(prev_block_current + 1);

				*space_is_occupied_current = true;

				if (prev_available_block == nullptr)
				{
					auto* size_space = reinterpret_cast<size_t*>(data);
					auto** first_available_block = get_first_available_block_address_address();

					*first_available_block = next_available_block;
				}
				else
				{
					auto* space_is_occupied_prev = reinterpret_cast<bool*>(prev_available_block);
					auto* space_power_prev = reinterpret_cast<short*>(space_is_occupied_prev + 1);
					auto** prev_block_prev = reinterpret_cast<void**>(space_power_prev + 1);
					auto** next_block_prev = reinterpret_cast<void**>(prev_block_prev + 1);

					*next_block_prev = next_available_block;
				}

				if (next_available_block != nullptr)
				{
					auto* space_is_occupied_next = reinterpret_cast<bool*>(next_available_block);
					auto* space_power_next = reinterpret_cast<short*>(space_is_occupied_next + 1);
					auto** prev_block_next = reinterpret_cast<void**>(space_power_next + 1);

					*prev_block_next = prev_available_block;
				}

//				if (log_memory != nullptr)
//				{
//					log_memory->log("Allocate block size: " + to_string(1 << get_block_power(current_available_block)),
//							logger::severity::INFORMATION);
//				}

				return space_power_current + 1;
			}

			current_available_block = get_next_block_address(current_available_block);
		}

//		if (log_memory != nullptr)
//		{
//			log_memory->log("Not enough memory to allocate", logger::severity::INFORMATION);
//		}

		return nullptr;
	}

	void* get_buddie(void* block) const
	{
		if (1 << get_block_power(block) == *reinterpret_cast<size_t*>(data))
		{
			return nullptr;
		}

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


//		if (log_memory != nullptr)
//		{
//			log_memory->log("deallocated block (" + to_string(deallocated_block) + ") : " +
//							get_bytes_buddies(deallocated_block), logger::severity::INFORMATION);
//		}

		void* current_block = target_to_dealloc;
		void* prev_block = nullptr;
		void* next_block = get_first_available_block_address();

		while (next_block != nullptr &&
			   reinterpret_cast<char*>(current_block) - reinterpret_cast<char*>(next_block) > 0)
		{
			prev_block = next_block;
			next_block = get_next_block_address(next_block);
		}

		auto* space_is_occupied_current = reinterpret_cast<bool*>(current_block);
		auto* space_power_current = reinterpret_cast<short*>(space_is_occupied_current + 1);
		void** prev_block_current = reinterpret_cast<void**>(space_power_current + 1);
		void** next_block_current = reinterpret_cast<void**>(prev_block_current + 1);

		*space_is_occupied_current = false;
		*prev_block_current = prev_block;
		*next_block_current = next_block;

		if (prev_block == nullptr)
		{
			auto** first_available_block = get_first_available_block_address_address();

			*first_available_block = current_block;
		}
		else
		{
			auto* space_is_occupied_prev = reinterpret_cast<bool*>(prev_block);
			auto* space_power_prev = reinterpret_cast<short*>(space_is_occupied_prev + 1);
			auto** prev_block_prev = reinterpret_cast<void**>(space_power_prev + 1);
			auto** next_block_prev = reinterpret_cast<void**>(prev_block_prev + 1);

			*next_block_prev = current_block;
		}

		if (next_block != nullptr)
		{
			auto* space_is_occupied_next = reinterpret_cast<bool*>(next_block);
			auto* space_power_next = reinterpret_cast<short*>(space_is_occupied_next + 1);
			auto** prev_block_next = reinterpret_cast<void**>(space_power_next + 1);

			*prev_block_next = current_block;
		}

		void* buddie = get_buddie(current_block);

		while (buddie != nullptr &&
			   block_is_available(buddie) &&
			   get_block_power(buddie) == get_block_power(current_block))
		{
			if (reinterpret_cast<char*>(buddie) - reinterpret_cast<char*>(current_block) < 0)
			{
				void* temp = buddie;
				buddie = current_block;
				current_block = temp;
			}

			auto* space_is_occupied_current = reinterpret_cast<bool*>(current_block);
			auto* space_power_current = reinterpret_cast<short*>(space_is_occupied_current + 1);
			void** prev_block_current = reinterpret_cast<void**>(space_power_current + 1);
			void** next_block_current = reinterpret_cast<void**>(prev_block_current + 1);

			void* next_block_buddie = get_next_block_address(buddie);

			*next_block_current = next_block_buddie;
			(*space_power_current)++;

			if (next_block_buddie != nullptr)
			{
				auto* space_is_occupied_next_block_buddie = reinterpret_cast<bool*>(next_block_buddie);
				auto* space_power_next_block_buddie = reinterpret_cast<short*>(space_is_occupied_next_block_buddie + 1);
				auto** space_prev_block_next_block_buddie = reinterpret_cast<void**>(space_power_next_block_buddie + 1);

				*space_prev_block_next_block_buddie = current_block;
			}

			buddie = get_buddie(current_block);
		}
	}
};


#endif //MAIN_C_SRC_MEMORY_4_H
