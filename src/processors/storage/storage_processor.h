#ifndef PROGC_SRC_PROCESSORS_STORAGE_STORAGE_PROCESSOR_H
#define PROGC_SRC_PROCESSORS_STORAGE_STORAGE_PROCESSOR_H


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
	const int this_status_code;
	const Connection* connection;
//	const Connection* connection_log;
	std::queue<std::string> to_log;

public:

	StorageProcessor(const int statusCode, const std::string& memName) : this_status_code(statusCode)
	{
		connection = new MemoryConnection(false, memName);
		connection->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
//		connection_log = new MemoryConnection(false, memName + "log");
	}

	~StorageProcessor()
	{
		delete connection;
//		delete connection_log;
	}

	void process() override
	{
		if ((SharedObject::GetStatusCode(connection->receiveMessage()) != this_status_code))
		{
			SharedObject message = SharedObject::deserialize(connection->receiveMessage());
			message.print();
			to_log.push("Receive message:" + message.getPrint());
			if (!message.GetData())
			{
				connection->sendMessage(SharedObject(this_status_code, OK, SharedObject::NULL_DATA));
				return;
			}
			auto data = message.GetData().value();
			std::string response = SharedObject::NULL_DATA;
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
				break;
			case GET_CONNECTION:
				break;
			case OK:
				break;
			case CLOSE_CONNECTION:
				break;
			}
			connection->sendMessage(SharedObject(this_status_code, OK, response));
		}

		// send log processing
//		if (!to_log.empty() && SharedObject::GetStatusCode(connection_log->receiveMessage()) != this_status_code) {
//			connection_log->sendMessage(SharedObject(this_status_code, LOG, to_log.front()));
//			to_log.pop();
//		}
	}
};

#endif //PROGC_SRC_PROCESSORS_STORAGE_STORAGE_PROCESSOR_H
