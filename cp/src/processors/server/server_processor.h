
#ifndef PROGC_SRC_PROCESSORS_SERVER_SERVER_PROCESSOR_H
#define PROGC_SRC_PROCESSORS_SERVER_SERVER_PROCESSOR_H

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <thread>
#include <queue>
#include "../../connection/connection.h"
#include "../../connection/memory_connection.h"
#include "../processor.h"
#include "../../data_types/shared_object.h"
#include "../../collections/set.h"
#include "../../data_types/contest_info.h"
#include "../../collections/hash_set.h"

using namespace boost::interprocess;

struct Storage
{
	std::unique_ptr<Connection> connection;
	std::unique_ptr<Connection> client_requested;
	std::queue<std::unique_ptr<Connection>> clients_to_process;
};

class ServerProcessor : public Processor
{
private:
	// storage : clients
	std::vector<Storage> storages;
	std::vector<std::unique_ptr<Connection>> clients;
	int client_id = 0;
	const int this_status_code;
	const Connection* connection;
	const named_mutex* connection_mutex;

public:

	ServerProcessor(const int statusCode, const std::string& memNameForConnect,
		const std::string& mutexNameForConnect, const std::vector<std::string>& storageMemNames)
		: this_status_code(statusCode)
	{
		if (storageMemNames.empty())
			throw std::runtime_error("1 storage minimum");

		for (const auto& storageMemName : storageMemNames)
		{
			storages.emplace_back();
			storages.back().connection = std::make_unique<MemoryConnection>(true, storageMemName);
			storages.back().client_requested = nullptr;
			storages.back().clients_to_process = {};
		}

		try
		{ named_mutex::remove(mutexNameForConnect.c_str()); }
		catch (...)
		{}
		connection = new MemoryConnection(true, memNameForConnect);
		connection_mutex = new named_mutex(create_only, mutexNameForConnect.c_str());

//		std::string connection_name = "client" + std::to_string(client_id);
//		clients.push_back(std::make_unique<MemoryConnection>(true, connection_name));
//		clients.back()->sendMessage(SharedObject(this_status_code, OK, "null"));
//		client_id++;
		connection->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
	}

	~ServerProcessor()
	{
		delete connection;
		delete connection_mutex;
	}

	void process() override
	{
		// clients get connection
		if ((SharedObject::GetStatusCode(connection->receiveMessage()) != this_status_code))
		{
			std::string connection_name = "client" + std::to_string(client_id);
			clients.push_back(std::make_unique<MemoryConnection>(true, connection_name));
			clients.back()->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
			client_id++;
			connection->sendMessage(SharedObject(this_status_code, OK, connection_name));
			std::cout << "Create connection: " << connection_name << std::endl;
		}

		// processing requests from clients
		for (auto it = clients.begin(); it != clients.end();)
		{
			Connection* client_connection = it->get();
			if ((SharedObject::GetStatusCode(client_connection->receiveMessage()) != this_status_code))
			{
				SharedObject message = SharedObject::deserialize(client_connection->receiveMessage());
				std::cout << "Client request:\n";
				message.print();
				if (!message.GetData())
				{
					connection->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
					return;
				}
				auto data = message.GetData().value();
				switch (message.GetRequestResponseCode())
				{
				case LOG:
				{
					std::cout << "LOG: " << data << std::endl;
					client_connection->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
					break;
				}
				case CLOSE_CONNECTION:
				{
					it = clients.erase(it);
					continue;
				}
				default:
				{
					auto& storage = storages.at(ContestInfo::deserialize(data).hashcode() % storages.size());
					storage.clients_to_process.push(std::move(*it));
					it = clients.erase(it);
					continue;
				}
				}

			}
			it++;
		}

		for (auto& storage : storages)
		{
			if (storage.client_requested == nullptr && !storage.clients_to_process.empty())
			{
				storage.client_requested = std::move(storage.clients_to_process.front());
				storage.clients_to_process.pop();
				storage.connection->sendMessage(SharedObject::deserialize(storage.client_requested->receiveMessage()));
			}

			if (storage.client_requested != nullptr)
			{
				if (SharedObject::GetStatusCode(storage.connection->receiveMessage())
					== SharedObject::GetStatusCode(storage.client_requested->receiveMessage()))
				{
					continue;
				}
				storage.client_requested->sendMessage(SharedObject::deserialize(storage.connection->receiveMessage()));
				clients.push_back(std::move(storage.client_requested));
				storage.client_requested = nullptr;
			}
		}
	}
};

#endif //PROGC_SRC_PROCESSORS_SERVER_SERVER_PROCESSOR_H
