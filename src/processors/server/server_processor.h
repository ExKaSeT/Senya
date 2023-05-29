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
#include "../../data_types/request_object.h"
#include "../../data_types/contest_info.h"
#include "../../collections/Map.h"
#include "../../collections/BPlusTree/BPlusTreeMap.h"
#include "../../logger/logger.h"


using namespace boost::interprocess;


struct Storage
{
	std::unique_ptr<MemoryConnection> connection;
	std::unique_ptr<MemoryConnection> client_requested;
	std::queue<std::unique_ptr<MemoryConnection>> clients_to_process;
};

// TODO: add send log
class ServerProcessor : public Processor
{
private:

	std::vector<Storage> storages;
//	std::vector<std::unique_ptr<MemoryConnection>> storages_log;
	std::vector<std::unique_ptr<MemoryConnection>> clients;
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

//			std::string connection_log_name = storageMemName + "log";
//			clients.push_back(std::make_unique<MemoryConnection>(true, connection_log_name));
//			clients.back()->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
		}

		try
		{ named_mutex::remove(mutexNameForConnect.c_str()); }
		catch (...)
		{}
		connection = new MemoryConnection(true, memNameForConnect);
		connection_mutex = new named_mutex(create_only, mutexNameForConnect.c_str());

		connection->sendMessage(SharedObject(this_status_code, SharedObject::RequestResponseCode::OK,
				SharedObject::NULL_DATA));
	}

	~ServerProcessor()
	{
		delete connection;
		delete connection_mutex;
	}

	void process() override
	{
		// clients get connection
		if ((SharedObject::getStatusCode(connection->receiveMessage()) != this_status_code))
		{
			std::string connection_name = "client" + std::to_string(client_id);
			clients.push_back(std::make_unique<MemoryConnection>(true, connection_name));
			clients.back()->sendMessage(SharedObject(this_status_code, SharedObject::RequestResponseCode::OK,
					SharedObject::NULL_DATA));
			client_id++;
			connection->sendMessage(SharedObject(this_status_code, SharedObject::RequestResponseCode::OK,
					connection_name));
			std::cout << "Create connection: " << connection_name << std::endl;
		}

		// processing requests from clients
		for (auto it = clients.begin(); it != clients.end();)
		{
			MemoryConnection* client_connection = it->get();
			if (SharedObject::getStatusCode(client_connection->receiveMessage()) != this_status_code)
			{
				SharedObject message = SharedObject::deserialize(client_connection->receiveMessage());
				std::cout << "Client '" << client_connection->getName() << "' request ~~~~~~~";
				message.print();
				if (message.getRequestResponseCode() == SharedObject::RequestResponseCode::CLOSE_CONNECTION) {
					it = clients.erase(it);
					continue;
				}
				auto dataOpt = message.getData();
				switch (message.getRequestResponseCode())
				{
				case SharedObject::RequestResponseCode::REQUEST:
				{
					if (!dataOpt)
					{
						client_connection->sendMessage(SharedObject(this_status_code,
								SharedObject::RequestResponseCode::ERROR, SharedObject::NULL_DATA));
						it++;
						continue;
					}
					auto contestInfo = ContestInfo::deserialize(RequestObject<ContestInfo>::deserialize(dataOpt.value()).getData());
					auto& storage = storages.at(contestInfo.hashcode() % storages.size());
					storage.clients_to_process.push(std::move(*it));
					it = clients.erase(it);
					continue;
				}
//				case LOG:
//				{
//                    logger_.log("[" + client_connection->getName() + "] " + data, logger::severity::trace);
//					client_connection->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
//					break;
//				}

				default:
				{
					client_connection->sendMessage(SharedObject(this_status_code,
							SharedObject::RequestResponseCode::ERROR, SharedObject::NULL_DATA));
				}
				}
//				for (auto& client : clients)
//					std::cout << client->getName() << " |";
//				std::cout << std::endl;
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
				if (SharedObject::getStatusCode(storage.connection->receiveMessage())
					== SharedObject::getStatusCode(storage.client_requested->receiveMessage()))
				{
					continue;
				}
				auto message = SharedObject::deserialize(storage.connection->receiveMessage());
				message.setStatusCode(this_status_code);
				storage.client_requested->sendMessage(message);
				clients.push_back(std::move(storage.client_requested));
				storage.client_requested = nullptr;
			}
		}

//        for (auto& storage_log : storages_log)
//        {
//            if (SharedObject::getStatusCode(storage_log->receiveMessage()) == this_status_code)
//            {
//                continue;
//            }
//            SharedObject message = SharedObject::deserialize(storage_log->receiveMessage());
//            auto data = message.getData().value();
//            logger_.log("[" + storage_log->getName() + "] " + data, logger::severity::trace);
//            storage_log->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
//        }
	}
};


#endif //PROGC_SRC_PROCESSORS_SERVER_SERVER_PROCESSOR_H
