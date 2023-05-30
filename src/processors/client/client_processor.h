#ifndef PROGC_SRC_PROCESSORS_CLIENT_CLIENT_PROCESSOR_H
#define PROGC_SRC_PROCESSORS_CLIENT_CLIENT_PROCESSOR_H


#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <thread>
#include "../../connection/connection.h"
#include "../../connection/memory_connection.h"
#include "../processor.h"
#include "../../data_types/shared_object.h"
#include "../../collections/Map.h"
#include "../../data_types/contest_info.h"
#include "../../data_types/request_object.h"
#include "../../loggers/server_logger/server_logger.h"


using namespace boost::interprocess;


class ClientProcessor : public Processor
{
private:

	const int thisStatusCode;
	const Connection* connection;
	std::string connectionName;
	ServerLogger& logger;

	void waitResponse()
	{
		while (SharedObject::getStatusCode(connection->receiveMessage()) == thisStatusCode)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

public:

	ClientProcessor(const int statusCode, const std::string& memNameForConnect,
			const std::string& mutexNameForConnect, ServerLogger& serverLogger)
			: thisStatusCode(statusCode), logger(serverLogger)
	{
		MemoryConnection connect_connection(false, memNameForConnect);
		named_mutex mutex(open_only, mutexNameForConnect.c_str());
		scoped_lock<named_mutex> lock(mutex);
		connect_connection.sendMessage(SharedObject(thisStatusCode,
				SharedObject::RequestResponseCode::GET_CONNECTION, SharedObject::NULL_DATA));
		while (SharedObject::getStatusCode(connect_connection.receiveMessage()) == thisStatusCode)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		auto data = SharedObject::deserialize(connect_connection.receiveMessage());
		auto memName = data.getData();
		if (!memName)
			throw std::runtime_error("Unable to establish a connection");
		connection = new MemoryConnection(false, memName.value());

		connectionName = memName.value();
		std::stringstream log;
		log << "[CLIENT] Get connection: " << connectionName << std::endl;
		logger.logSync(log.str(), logger::severity::debug);
		std::cout << log.str();
	}

	~ClientProcessor() override
	{
		connection->sendMessage(SharedObject(thisStatusCode,
				SharedObject::RequestResponseCode::CLOSE_CONNECTION, SharedObject::NULL_DATA));
		delete connection;
	}

	bool add(const std::string& database, const std::string& schema, const std::string& table,
			const ContestInfo& value)
	{
		RequestObject<ContestInfo> request(RequestObject<ContestInfo>::RequestCode::ADD,
				value, database, schema, table);
		connection->sendMessage(SharedObject(thisStatusCode,
				SharedObject::RequestResponseCode::REQUEST, request));
		waitResponse();
		auto response = SharedObject::deserialize(connection->receiveMessage());
		if (response.getRequestResponseCode() != SharedObject::RequestResponseCode::OK)
			return false;
		std::string result = response.getData().value();
		if (result == "true")
			return true;
		return false;
	};

	std::optional<ContestInfo> get(const std::string& database, const std::string& schema,
			const std::string& table, const ContestInfo& value)
	{
		RequestObject<ContestInfo> request(RequestObject<ContestInfo>::RequestCode::GET_KEY,
				value, database, schema, table);
		connection->sendMessage(SharedObject(thisStatusCode,
				SharedObject::RequestResponseCode::REQUEST, request));
		waitResponse();
		auto response = SharedObject::deserialize(connection->receiveMessage());
		if (response.getRequestResponseCode() != SharedObject::RequestResponseCode::OK)
			return std::nullopt;
		auto data = response.getData();
		if (!data)
			return std::nullopt;
		return ContestInfo::deserialize(data.value());
	};

	bool contains(const std::string& database, const std::string& schema, const std::string& table,
			const ContestInfo& value)
	{
		RequestObject<ContestInfo> request(RequestObject<ContestInfo>::RequestCode::CONTAINS,
				value, database, schema, table);
		connection->sendMessage(SharedObject(thisStatusCode,
				SharedObject::RequestResponseCode::REQUEST, request));
		waitResponse();
		auto response = SharedObject::deserialize(connection->receiveMessage());
		if (response.getRequestResponseCode() != SharedObject::RequestResponseCode::OK)
			return false;
		std::string result = response.getData().value();
		if (result == "true")
			return true;
		return false;
	};

	bool remove(const std::string& database, const std::string& schema, const std::string& table,
			const ContestInfo& value)
	{
		RequestObject<ContestInfo> request(RequestObject<ContestInfo>::RequestCode::REMOVE,
				value, database, schema, table);
		connection->sendMessage(SharedObject(thisStatusCode,
				SharedObject::RequestResponseCode::REQUEST, request));
		waitResponse();
		auto response = SharedObject::deserialize(connection->receiveMessage());
		if (response.getRequestResponseCode() != SharedObject::RequestResponseCode::OK)
			return false;
		std::string result = response.getData().value();
		if (result == "true")
			return true;
		return false;
	};

	void log(const std::string& message, logger::severity severity)
	{
		logger.logSync(message, severity);
	};

	void process() override
	{
	};
};


#endif //PROGC_SRC_PROCESSORS_CLIENT_CLIENT_PROCESSOR_H
