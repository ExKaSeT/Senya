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
	// method | alloc_ptr | logger_ptr | first_available_ptr
	char* m_data;

	bool has_logger() const
	{
		char* current = m_data;
		current += (sizeof(allocation_method) + sizeof(memory*));
		logger* log_ = *reinterpret_cast<logger**>(current);
		return log_ != nullptr;
	};

	void log(std::string const& str, logger::severity severity) const
	{
		char* current = m_data;
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
		return *reinterpret_cast<allocation_method*>(m_data);
	}

	void** get_first_block_address_address() const
	{
		return reinterpret_cast<void**>(m_data + sizeof(allocation_method) + sizeof(memory*) + sizeof(logger*));
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
		if (data_size < get_service_block_size() + sizeof(size_t) + sizeof(void*))
			throw std::bad_alloc();

		if (alloc == nullptr)
		{
			try
			{
				m_data = reinterpret_cast<char*>(::operator new(data_size));
			}
			catch (...)
			{
				this->log("Alloc error", logger::severity::error);
				throw;
			}
		}
		else
			m_data = reinterpret_cast<char*>(alloc->allocate(data_size));

		if (m_data == nullptr)
		{
			this->log("Alloc error", logger::severity::error);
			throw std::bad_alloc();
		}

		char* current = m_data;

		auto* m_method = reinterpret_cast<allocation_method*>(current);
		*m_method = method;
		current += sizeof(allocation_method);

		auto** m_alloc = reinterpret_cast<memory**>(current);
		*m_alloc = alloc;
		current += sizeof(memory*);

		auto** log_ = reinterpret_cast<logger**>(current);
		*log_ = log;
		current += sizeof(logger*);

		auto** first_available_block = reinterpret_cast<void**>(current);
		current += sizeof(void*);
		*first_available_block = reinterpret_cast<void*>(current);

		size_t free_size;
		free_size = data_size - get_service_block_size();

		auto* b_size = reinterpret_cast<size_t*>(current);
		*b_size = free_size - (sizeof(size_t) + sizeof(void*));
		current += sizeof(size_t);

		auto** next_ptr = reinterpret_cast<void**>(current);
		*next_ptr = nullptr;

		if (log != nullptr)
		{
			current += sizeof(size_t);
			std::stringstream log_stream;
			log_stream << "Allocated " << *b_size << " bytes [ " << current - m_data << " ] ";
			this->log(log_stream.str(), logger::severity::information);
		}
	}

	~memory_3() override
	{
		char* current = m_data;
		current += sizeof(allocation_method);
		memory* m_alloc = *reinterpret_cast<memory**>(current);
		if (m_alloc == nullptr)
			::operator delete(m_data);
		else
			m_alloc->deallocate(m_data);
	}

	void* allocate(size_t requested_size) const override
	{
		if (requested_size < sizeof(void*))
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

			if (current_block_size - get_available_block_service_size() >=
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
			throw std::runtime_error("No available memory");
		}

		if (get_block_size(target_block) - get_occupied_block_service_size() - requested_size <
			get_available_block_service_size())
		{
			auto requested_size_overriden = get_block_size(target_block) - get_occupied_block_service_size() -
											get_available_block_service_size();
			requested_size = requested_size_overriden;
		}

		void* update_next_block_to_previous;

		if (requested_size == get_block_size(target_block) - get_occupied_block_service_size())
		{
			update_next_block_to_previous = next_to_target_block;
		}
		else
		{
			update_next_block_to_previous = reinterpret_cast<void*>
			(reinterpret_cast<unsigned char*>(target_block) + get_occupied_block_service_size() + requested_size);

			auto* target_block_leftover_size = reinterpret_cast<size_t*>(update_next_block_to_previous);
			*target_block_leftover_size =
					get_block_size(target_block) - get_occupied_block_service_size() - requested_size;

			auto* target_block_leftover_next_block_address = reinterpret_cast<void**>(target_block_leftover_size + 1);
			*target_block_leftover_next_block_address = next_to_target_block;
		}

		previous_to_target_block == nullptr
		? *get_first_block_address_address() = update_next_block_to_previous
		: *reinterpret_cast<void**>
		(reinterpret_cast<unsigned char*>(previous_to_target_block) + sizeof(size_t)) = update_next_block_to_previous;

		auto* target_block_size_address = reinterpret_cast<size_t*>(target_block);
		*target_block_size_address = requested_size;

		return reinterpret_cast<void*>(target_block_size_address + 1);
	}

//	void* allocate(size_t target_size) const
//	{
//		allocation_method m_method = *reinterpret_cast<allocation_method*>(m_data);
//		void* ptr;
//		if (m_method == first)
//			ptr = first_fit(target_size);
//		else if (m_method == best)
//			ptr = best_fit(target_size);
//		else
//			ptr = worst_fit(target_size);
//
//		if (this->has_logger())
//		{
//			std::stringstream log_stream;
//			if (ptr == nullptr)
//				log_stream << "Not enough memory";
//			else if (m_method == first)
//				log_stream << "Allocated [ FIRST FIT ] [ " << reinterpret_cast<char*>(ptr) - m_data << " ] ";
//			else if (m_method == best)
//				log_stream << "Allocated [ BEST FIT ] [ " << reinterpret_cast<char*>(ptr) - m_data << " ] ";
//			else
//				log_stream << "Allocated [ WORST FIT ] [ " << reinterpret_cast<char*>(ptr) - m_data << " ] ";
//			log(log_stream.str(), logger::severity::information);
//		}
//		return ptr;
//	}

	void deallocate(void* target_to_dealloc) const override
	{
		*const_cast<void**>(&target_to_dealloc) = reinterpret_cast<void*>
		(reinterpret_cast<size_t*>(target_to_dealloc) - 1);

		if (this->has_logger())
		{
			size_t block_size = *reinterpret_cast<size_t*>(target_to_dealloc);
			std::stringstream log_stream;
			char* block_data = reinterpret_cast<char*>(target_to_dealloc) + sizeof(size_t);
			log_stream << "Deallocated [ " << block_data - m_data << " ]: ";
			auto* byte_ptr = reinterpret_cast<unsigned char*>(block_data);
			for (auto i = 0; i < block_size; i++)
			{
				log_stream << static_cast<unsigned short>(byte_ptr[i]) << " ";
			}
			log(log_stream.str(), logger::severity::information);
		}

		void* previous_block = nullptr;
		void* current_block = get_first_block_address();
		if (current_block == nullptr)
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
		if (current_block == nullptr)
		{
			*reinterpret_cast<void**>(reinterpret_cast<size_t*>(previous_block) + 1) = target_to_dealloc;
			auto* block_to_deallocate_size = reinterpret_cast<size_t*>(target_to_dealloc);
			*block_to_deallocate_size -= sizeof(void*);

			*reinterpret_cast<void**>(block_to_deallocate_size + 1) = nullptr;
		}
	}
};


#endif //MAIN_C_SRC_MEMORY_3_H
