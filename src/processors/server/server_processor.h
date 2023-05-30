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


using namespace boost::interprocess;


struct Storage
{
	std::unique_ptr<MemoryConnection> connection;
	std::unique_ptr<MemoryConnection> client_requested;
	std::queue<std::unique_ptr<MemoryConnection>> clients_to_process;
};

class ServerProcessor : public Processor
{
private:

	std::vector<Storage> storages;
	std::vector<std::unique_ptr<MemoryConnection>> clients;
	int client_id = 0;
	const int this_status_code;
	const Connection* connection;
	const named_mutex* connection_mutex;
	ServerLogger& logger;

public:

	ServerProcessor(const int statusCode, const std::string& memNameForConnect,
			const std::string& mutexNameForConnect, const std::vector<std::string>& storageMemNames,
			ServerLogger& serverLogger)
			: this_status_code(statusCode), logger(serverLogger)
	{
		if (storageMemNames.empty())
			throw std::runtime_error("1 storage minimum");

		for (const auto& storageMemName: storageMemNames)
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

		connection->sendMessage(SharedObject(this_status_code, SharedObject::RequestResponseCode::OK,
				SharedObject::NULL_DATA));
	}

	~ServerProcessor() override
	{
		delete connection;
		delete connection_mutex;
	}

	void process() override
	{
		logger.process();

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

			std::stringstream log;
			log << "[SERVER] Create connection: " << connection_name << std::endl;
			std::cout << log.str();
			logger.log(log.str(), logger::severity::debug);
		}

		// processing requests from clients
		for (auto it = clients.begin(); it != clients.end();)
		{
			MemoryConnection* client_connection = it->get();
			if (SharedObject::getStatusCode(client_connection->receiveMessage()) != this_status_code)
			{
				SharedObject message = SharedObject::deserialize(client_connection->receiveMessage());

				std::stringstream log;
				log << "[SERVER] Client '" << client_connection->getName() << "' request:" << std::endl
					<< message.getPrint();
				std::cout << log.str();
				logger.log(log.str(), logger::severity::debug);

				if (message.getRequestResponseCode() == SharedObject::RequestResponseCode::CLOSE_CONNECTION)
				{
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
					auto contestInfo = ContestInfo::deserialize(
							RequestObject<ContestInfo>::deserialize(dataOpt.value()).getData());
					auto& storage = storages.at(contestInfo.hashcode() % storages.size());
					storage.clients_to_process.push(std::move(*it));
					it = clients.erase(it);
					continue;
				}
				default:
				{
					client_connection->sendMessage(SharedObject(this_status_code,
							SharedObject::RequestResponseCode::ERROR, SharedObject::NULL_DATA));
				}
				}
			}
			it++;
		}

		for (auto& storage: storages)
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
	}
};


#endif //PROGC_SRC_PROCESSORS_SERVER_SERVER_PROCESSOR_H
