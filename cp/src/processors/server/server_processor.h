
#ifndef PROGC_SRC_PROCESSORS_SERVER_SERVER_PROCESSOR_H
#define PROGC_SRC_PROCESSORS_SERVER_SERVER_PROCESSOR_H

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <thread>
#include "../../connection/connection.h"
#include "../../connection/memory_connection.h"
#include "../processor.h"
#include "../../data_types/shared_object.h"
#include "../../collections/set.h"
#include "../../data_types/contest_info.h"
#include "../../collections/hash_set.h"

using namespace boost::interprocess;

class StorageProcessor : public Processor
{
private:

	HashSet<ContestInfo> set;
	std::vector<std::unique_ptr<Connection>> clients;
	int client_id = 0;
	const int this_status_code;
	const Connection* connection;
	const named_mutex* connection_mutex;

public:

	StorageProcessor(const int statusCode, const std::string& memNameForConnect,
		const std::string& mutexNameForConnect) : this_status_code(statusCode)
	{
		try
		{ named_mutex::remove(mutexNameForConnect.c_str()); }
		catch (...)
		{}
		connection = new MemoryConnection(true, memNameForConnect);
		connection_mutex = new named_mutex(create_only, mutexNameForConnect.c_str());

		std::string connection_name = "client" + std::to_string(client_id);
		clients.push_back(std::make_unique<MemoryConnection>(true, connection_name));
		clients.back()->sendMessage(SharedObject(this_status_code, OK, "null"));
		client_id++;
		connection->sendMessage(SharedObject(this_status_code, OK, connection_name));
	}

	~StorageProcessor()
	{
		delete connection;
		delete connection_mutex;
	}

	void process() override
	{
		if ((SharedObject::GetStatusCode(connection->receiveMessage()) != this_status_code))
		{
			std::string connection_name = "client" + std::to_string(client_id);
			clients.push_back(std::make_unique<MemoryConnection>(true, connection_name));
			clients.back()->sendMessage(SharedObject(this_status_code, OK, "null"));
			client_id++;
			connection->sendMessage(SharedObject(this_status_code, OK, connection_name));
		}


		for (auto it = clients.begin(); it != clients.end(); )
		{
			Connection* client_connection = it->get();
			if ((SharedObject::GetStatusCode(client_connection->receiveMessage()) != this_status_code))
			{
				std::cout << std::string(client_connection->receiveMessage()) << ";\n";
				SharedObject message = SharedObject::deserialize(client_connection->receiveMessage());
				if (!message.GetData())
				{
					return;
				}
				auto data = message.GetData().value();
				std::string response = "null";
				switch (message.GetRequestResponseCode())
				{
				case ADD:
				{
					set.add(ContestInfo::deserialize(data));
					break;
				}
				case CONTAINS:
				{
					auto result = set.contains(ContestInfo::deserialize(data));
					if (result)
						response = result.value().serialize();
					break;
				}
				case REMOVE:
				{
					set.remove(ContestInfo::deserialize(data));
					break;
				}
				case LOG:
				{
					std::cout << "LOG: " << data;
					break;
				}
				case GET_CONNECTION:
					break;
				case OK:
					break;
				case CLOSE_CONNECTION:
				{
					it = clients.erase(it);
					continue;
				}
				}
				client_connection->sendMessage(SharedObject(this_status_code, OK, response));
			}
			it++;
		}
	}
};

#endif //PROGC_SRC_PROCESSORS_SERVER_SERVER_PROCESSOR_H
