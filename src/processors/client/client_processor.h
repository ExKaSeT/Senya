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


using namespace boost::interprocess;


class ClientProcessor : public Processor
{
private:

	const int this_status_code;
	const Connection* connection;

	void waitResponse()
	{
		while (SharedObject::getStatusCode(connection->receiveMessage()) == this_status_code)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

public:

	ClientProcessor(const int statusCode, const std::string& memNameForConnect,
			const std::string& mutexNameForConnect) : this_status_code(statusCode)
	{
		MemoryConnection connect_connection(false, memNameForConnect);
		named_mutex mutex(open_only, mutexNameForConnect.c_str());
		scoped_lock<named_mutex> lock(mutex);
		connect_connection.sendMessage(SharedObject(this_status_code,
				SharedObject::RequestResponseCode::GET_CONNECTION, SharedObject::NULL_DATA));
		while (SharedObject::getStatusCode(connect_connection.receiveMessage()) == this_status_code)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		auto data = SharedObject::deserialize(connect_connection.receiveMessage());
		auto memName = data.getData();
		if (!memName)
			throw std::runtime_error("Unable to establish a connection");
		connection = new MemoryConnection(false, memName.value());
		std::cout << "Get connection: " << memName.value() << std::endl;
	}

	~ClientProcessor()
	{
		connection->sendMessage(SharedObject(this_status_code,
				SharedObject::RequestResponseCode::CLOSE_CONNECTION, SharedObject::NULL_DATA));
		delete connection;
	}

	bool add(const std::string& database, const std::string& schema, const std::string& table,
			const ContestInfo& value)
	{
		RequestObject<ContestInfo> request(RequestObject<ContestInfo>::RequestCode::ADD,
				value, database, schema, table);
		connection->sendMessage(SharedObject(this_status_code,
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

//	std::optional<ContestInfo> get(const RequestObject<ContestInfo>& value)
//	{
//		connection->sendMessage(SharedObject(this_status_code, CONTAINS, value));
//		waitResponse();
//		auto data = SharedObject::deserialize(connection->receiveMessage()).getData();
//		if (!data)
//			return std::nullopt;
//		return { ContestInfo::deserialize(data.value()) };
//	};

	bool contains(const std::string& database, const std::string& schema, const std::string& table,
			const ContestInfo& value)
	{
		RequestObject<ContestInfo> request(RequestObject<ContestInfo>::RequestCode::CONTAINS,
				value, database, schema, table);
		connection->sendMessage(SharedObject(this_status_code,
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
		connection->sendMessage(SharedObject(this_status_code,
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

	void log(const std::string& message)
	{
		// TODO: log
//		connection->sendMessage(SharedObject(this_status_code, LOG, message));
//		waitResponse();
	};

	void process() override
	{

	};
};


#endif //PROGC_SRC_PROCESSORS_CLIENT_CLIENT_PROCESSOR_H
