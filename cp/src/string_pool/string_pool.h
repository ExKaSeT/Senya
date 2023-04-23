
#ifndef PROGC_SRC_STRING_POOL_STRING_POOL_H
#define PROGC_SRC_STRING_POOL_STRING_POOL_H

#include <iostream>
#include <string>
#include <unordered_map>

class StringPool
{
private:
	// Приватный конструктор, чтобы запретить создание объектов извне
	StringPool() {}

	// Хэш-таблица для хранения строк
	std::unordered_map<std::string, int> string_pool_;

public:
	// Получение единственного экземпляра класса
	static StringPool& instance()
	{
		static StringPool pool;
		return pool;
	}

	// Получение строки из пула
	const std::string& get_string(const std::string& str)
	{
		// Поиск строки в пуле
		auto it = string_pool_.find(str);
		if (it != string_pool_.end())
		{
			// Если строка уже есть в пуле, то возвращаем её
			it->second++;
			return it->first;
		}
		else
		{
			// Иначе добавляем строку в пул и возвращаем её
			auto [new_it, inserted] = string_pool_.emplace(str, 1);
			return new_it->first;
		}
	}

	void unget_string(const std::string& str)
	{
		auto it = string_pool_.find(str);
		if (it != string_pool_.end())
		{
			it->second--;
			if (it->second <= 0)
				string_pool_.erase(it);
		}
	}
};

#endif //PROGC_SRC_STRING_POOL_STRING_POOL_H
