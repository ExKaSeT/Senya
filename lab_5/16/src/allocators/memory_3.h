#ifndef MAIN_C_SRC_MEMORY_3_H
#define MAIN_C_SRC_MEMORY_3_H


#include <sstream>
#include "memory.h"
#include "../logger/logger.h"


class memory_3 : public Memory
{
public:

	enum allocation_method
	{
		first, best, worst
	};

private:
	// method | alloc_ptr | logger_ptr | size_t
	char* m_data;

	void* first_fit(size_t target_size) const
	{
		char* current = m_data;
		current += (sizeof(allocation_method) + sizeof(Memory*) + sizeof(logger*));
		size_t m_size = *reinterpret_cast<size_t*>(current);
		current += sizeof(size_t);
		while (current < m_data + m_size)
		{
			auto* is_free = reinterpret_cast<bool*>(current);
			auto* b_size = reinterpret_cast<size_t*>(current + sizeof(bool));
			if ((*is_free) && *b_size >= target_size)
			{
				*is_free = false;
				char* ptr = current + sizeof(bool) + sizeof(size_t);
				block_split(ptr, target_size);
				return ptr;
			}
			current += *b_size + sizeof(bool) + sizeof(size_t);
		}
		return nullptr;
	}

	void* best_fit(size_t target_size) const
	{
		size_t min_ok = SIZE_MAX;
		char* ptr = nullptr;
		char* current = m_data;
		current += (sizeof(allocation_method) + sizeof(Memory*) + sizeof(logger*));
		size_t m_size = *reinterpret_cast<size_t*>(current);
		current += sizeof(size_t);
		while (current < m_data + m_size)
		{
			auto* is_free = reinterpret_cast<bool*>(current);
			auto* b_size = reinterpret_cast<size_t*>(current + sizeof(bool));
			if (*is_free && *b_size >= target_size)
			{
				if (min_ok > *b_size)
				{
					min_ok = *b_size;
					ptr = current + sizeof(bool) + sizeof(size_t);
				}
			}
			current += sizeof(bool) + sizeof(size_t) + *b_size;
		}
		if (ptr == nullptr)
			return ptr;
		auto* is_free = reinterpret_cast<bool*>(ptr - sizeof(bool) - sizeof(size_t));
		*is_free = false;
		block_split(ptr, target_size);
		return ptr;
	}

	void* worst_fit(size_t target_size) const
	{
		size_t max_ok = 0;
		char* ptr = nullptr;
		char* current = m_data;
		current += (sizeof(allocation_method) + sizeof(Memory*) + sizeof(logger*));
		size_t m_size = *reinterpret_cast<size_t*>(current);
		current += sizeof(size_t);
		while (current < m_data + m_size)
		{
			auto* is_free = reinterpret_cast<bool*>(current);
			auto* b_size = reinterpret_cast<size_t*>(current + sizeof(bool));
			if (*is_free && *b_size >= target_size)
			{
				if (max_ok < *b_size)
				{
					max_ok = *b_size;
					ptr = current + sizeof(bool) + sizeof(size_t);
				}
			}
			current += sizeof(bool) + sizeof(size_t) + *b_size;
		}
		if (ptr == nullptr)
			return ptr;
		auto* is_free = reinterpret_cast<bool*>(ptr - sizeof(bool) - sizeof(size_t));
		*is_free = false;
		block_split(ptr, target_size);
		return ptr;
	}

	void merge_free() const
	{
		char* current = m_data;
		current += (sizeof(allocation_method) + sizeof(Memory*) + sizeof(logger*));
		size_t m_size = *(reinterpret_cast<size_t*>(current));
		current += sizeof(size_t);
		auto* prev_is_free = reinterpret_cast<bool*>(current);
		auto* prev_b_size = reinterpret_cast<size_t*>(current + sizeof(bool));
		if (current + *prev_b_size + sizeof(size_t) + sizeof(bool) < m_data + m_size)
			current += (*prev_b_size + sizeof(size_t) + sizeof(bool));
		else
			return;
		auto* is_free = reinterpret_cast<bool*>(current);
		auto* b_size = reinterpret_cast<size_t*>(current + sizeof(bool));
		current += (sizeof(bool) + sizeof(size_t) + *b_size);
		while (current + sizeof(bool) < m_data + m_size)
		{
			auto* next_is_free = reinterpret_cast<bool*>(current);
			auto* next_b_size = reinterpret_cast<size_t*>(current + sizeof(bool));
			if (*is_free && (*prev_is_free || *next_is_free))
			{
				if (*next_is_free)
				{
					*b_size += *next_b_size + sizeof(size_t) + sizeof(bool);
				}
				if (*prev_is_free)
				{
					*prev_b_size += *b_size + sizeof(size_t) + sizeof(bool);
					b_size = next_b_size;
					is_free = next_is_free;
				}
				if (*next_is_free && *prev_is_free)
				{
					b_size = reinterpret_cast<size_t*>(current + 2 * sizeof(bool) + sizeof(size_t) + *next_b_size);
					is_free = reinterpret_cast<bool*>(current + sizeof(bool) + sizeof(size_t) + *next_b_size);
					*next_b_size += (sizeof(size_t) + sizeof(bool) + *b_size);
				}
			}
			else
			{
				prev_b_size = b_size;
				prev_is_free = is_free;
				b_size = next_b_size;
				is_free = next_is_free;
			}
			current += *next_b_size + sizeof(size_t) + sizeof(bool);
		}
	}

	void block_split(char* block_ptr, size_t target_size) const
	{
		if (block_ptr == nullptr)
			return;
		auto* b_size = reinterpret_cast<size_t*>(block_ptr - sizeof(size_t));
		if (*b_size > target_size + sizeof(bool) + sizeof(size_t))
		{
			auto* next_is_free = reinterpret_cast<bool*>(block_ptr + target_size);
			*next_is_free = true;
			auto* next_size = reinterpret_cast<size_t*>(next_is_free + sizeof(bool));
			*next_size = *b_size - target_size - sizeof(bool) - sizeof(size_t);
			*b_size = target_size;
		}
	}

	bool has_logger() const
	{
		char* current = m_data;
		current += (sizeof(allocation_method) + sizeof(Memory*));
		logger* log_ = *reinterpret_cast<logger**>(current);
		return log_ != nullptr;
	};

	void log(std::string const& str, logger::severity severity) const
	{
		char* current = m_data;
		current += (sizeof(allocation_method) + sizeof(Memory*));
		logger* log_ = *reinterpret_cast<logger**>(current);
		if (log_ == nullptr)
			return;
		log_->log(str, severity);
	}

public:

	memory_3(size_t data_size, allocation_method method, Memory* alloc = nullptr, logger* log = nullptr)
	{
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

		auto** m_alloc = reinterpret_cast<Memory**>(current);
		*m_alloc = alloc;
		current += sizeof(Memory*);

		auto** log_ = reinterpret_cast<logger**>(current);
		*log_ = log;
		current += sizeof(logger*);

		auto* m_size = reinterpret_cast<size_t*>(current);
		*m_size = (data_size - (sizeof(allocation_method) + sizeof(Memory*) + sizeof(logger*) + sizeof(size_t)));
		current += sizeof(size_t);

		auto* is_free = reinterpret_cast<bool*>(current);
		*is_free = true;
		current += sizeof(bool);

		auto* b_size = reinterpret_cast<size_t*>(current);
		*b_size = *m_size - (sizeof(bool) + sizeof(size_t));

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
		Memory* m_alloc = *reinterpret_cast<Memory**>(current);
		if (m_alloc == nullptr)
			::operator delete(m_data);
		else
			m_alloc->deallocate(m_data);
	}

	void* allocate(size_t target_size) const override
	{
		allocation_method m_method = *reinterpret_cast<allocation_method*>(m_data);
		void* ptr;
		if (m_method == first)
			ptr = first_fit(target_size);
		else if (m_method == best)
			ptr = best_fit(target_size);
		else
			ptr = worst_fit(target_size);

		if (this->has_logger())
		{
			std::stringstream log_stream;
			if (ptr == nullptr)
				log_stream << "Not enough memory";
			else if (m_method == first)
				log_stream << "Allocated [ FIRST FIT ] [ " << reinterpret_cast<char*>(ptr) - m_data << " ] ";
			else if (m_method == best)
				log_stream << "Allocated [ BEST FIT ] [ " << reinterpret_cast<char*>(ptr) - m_data << " ] ";
			else
				log_stream << "Allocated [ WORST FIT ] [ " << reinterpret_cast<char*>(ptr) - m_data << " ] ";
			log(log_stream.str(), logger::severity::information);
		}

		return ptr;
	}

	void deallocate(void* target_to_dealloc) const override
	{
		if (target_to_dealloc == nullptr)
			return;
		char* current = reinterpret_cast<char*>(target_to_dealloc);
		auto* is_free = reinterpret_cast<bool*>(current - sizeof(size_t) - sizeof(bool));
		*is_free = true;

		if (this->has_logger())
		{
			size_t b_size = *reinterpret_cast<size_t*>(current - sizeof(size_t));
			std::stringstream log_stream;
			log_stream << "Deallocated [ " << current - m_data << " ]: ";
			auto* byte_ptr = reinterpret_cast<unsigned char*>(current);
			for (auto i = 0; i < b_size; i++)
			{
				log_stream << static_cast<unsigned short>(byte_ptr[i]) << " ";
			}
			log(log_stream.str(), logger::severity::information);
		}

		merge_free();
	}
};


#endif //MAIN_C_SRC_MEMORY_3_H
