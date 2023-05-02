
#ifndef PROGC_SRC_STRING_POOL_STRING_POOL_H
#define PROGC_SRC_STRING_POOL_STRING_POOL_H

#include <iostream>
#include <string>
#include <unordered_map>

class StringPool
{
private:
	StringPool()
	{
	}

	std::unordered_map<std::string, int> string_pool_;

public:

	static StringPool& instance()
	{
		static StringPool pool;
		return pool;
	}

	const std::string& get_string(const std::string& str)
	{
		auto it = string_pool_.find(str);
		if (it != string_pool_.end())
		{
			it->second++;

//			std::cout << "get_string: ";
//			for (const auto& elem : string_pool_) {
//				std::cout << elem.first << " + " << elem.second << " | ";
//			}
//			std::cout << "\n";

			return it->first;
		}
		else
		{
			auto [new_it, _] = string_pool_.emplace(str, 1);

//			std::cout << "get_string: ";
//			for (const auto& elem : string_pool_) {
//				std::cout << elem.first << " + " << elem.second << " | ";
//			}
//			std::cout << "\n";

			return new_it->first;
		}


	}

	// не работает нихуя заебался
	void unget_string(const std::string& str)
	{
		auto it = string_pool_.find(str);
		if (it != string_pool_.end())
		{
			it->second--;
			if (it->second <= 0)
				string_pool_.erase(it);
		}

//		std::cout << str << " :unget_string: ";
//		for (const auto& elem : string_pool_) {
//			std::cout << elem.first << " + " << elem.second << " | ";
//		}
//		std::cout << "\n";
	}

	~StringPool() {
		string_pool_.clear();
	}
};

#endif //PROGC_SRC_STRING_POOL_STRING_POOL_H
