

#ifndef PROGC_SRC_CONNECTION_MEMORY_CONNECTION_H
#define PROGC_SRC_CONNECTION_MEMORY_CONNECTION_H

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include "./connection.h"
#include "../extensions/serializable.h"

using namespace boost::interprocess;

class MemoryConnection : public Connection
{
private:

	std::string mem_name;
	mapped_region* mreg;
	const bool is_server;

public:

	MemoryConnection(bool isServer, const std::string& memoryName) : is_server(isServer), mem_name(memoryName)
	{
		if (is_server)
		{
			try
			{ shared_memory_object::remove(mem_name.c_str()); }
			catch (...)
			{}
			shared_memory_object shm(create_only, mem_name.c_str(), read_write);
			shm.truncate(1024);
			mreg = new mapped_region(shm, read_write);
		}
		else
		{
			shared_memory_object shm(open_only, mem_name.c_str(), read_write);
			mreg = new mapped_region(shm, read_write);
		}
	}

	~MemoryConnection() override
	{
		if (is_server)
		{
			shared_memory_object::remove(mem_name.c_str());
		}
		delete mreg;
	}

	const char* receiveMessage() const override
	{
		return static_cast<const char*>(mreg->get_address());
	}

	// первый байт (статус) изменяется после записи данных
	void sendMessage(const Serializable& data) const override
	{
		std::string str = data.serialize();
		const char* data_str = str.c_str();
		char* address = static_cast<char*>(mreg->get_address());
//		std::cout << str << ";" << std::string(static_cast<char*>(mreg->get_address())) << "\n";
		std::sprintf(address + 1, data_str + 1);
//		std::cout << static_cast<char*>(mreg->get_address()) << "\n;\n";
		*address = *data_str;
	}
};

#endif //PROGC_SRC_CONNECTION_MEMORY_CONNECTION_H
