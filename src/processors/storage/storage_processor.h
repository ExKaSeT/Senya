#ifndef PROGC_SRC_PROCESSORS_STORAGE_STORAGE_PROCESSOR_H
#define PROGC_SRC_PROCESSORS_STORAGE_STORAGE_PROCESSOR_H


#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <thread>
#include "../../connection/connection.h"
#include "../../connection/memory_connection.h"
#include "../processor.h"
#include "../../data_types/shared_object.h"
#include "../../data_types/contest_info.h"
#include "../../collections/Map.h"
#include "../../collections/BPlusTree/BPlusTreeMap.h"
#include "../../data_types/request_object.h"


using namespace boost::interprocess;


int stringComparer(const std::string& a, const std::string& b)
{
	return a.compare(b);
}

int contestInfoComparer(const ContestInfo& a, const ContestInfo& b)
{
	if (a.getContestId() < b.getContestId())
	{
		return -1;
	}
	else if (a.getContestId() > b.getContestId())
	{
		return 1;
	}
	if (a.getCandidateId() < b.getCandidateId())
	{
		return -1;
	}
	else if (a.getCandidateId() > b.getCandidateId())
	{
		return 1;
	}
	return 0;
}

class StorageProcessor : public Processor
{
private:

	BPlusTreeMap<std::string, std::shared_ptr
			<
					Map<std::string, std::shared_ptr
							<
									Map<std::string, std::shared_ptr
											<
													Map<ContestInfo, Null>
											>
									>
							>
					>
			>
	> db;
	const int this_status_code;
	const Connection* connection;
//	const Connection* connection_log;
	std::queue<std::string> to_log;

public:

	StorageProcessor(const int statusCode, const std::string& memName) : this_status_code(statusCode),
																		 db(3, 3, stringComparer)
	{
		connection = new MemoryConnection(false, memName);
		connection->sendMessage(SharedObject(this_status_code, SharedObject::RequestResponseCode::OK,
				SharedObject::NULL_DATA));
//		connection_log = new MemoryConnection(false, memName + "log");
	}

	~StorageProcessor()
	{
		delete connection;
//		delete connection_log;
	}

	void process() override
	{
		if ((SharedObject::getStatusCode(connection->receiveMessage()) != this_status_code))
		{
			SharedObject message = SharedObject::deserialize(connection->receiveMessage());
			message.print();
			to_log.push("Receive message:" + message.getPrint());
			auto messageData = message.getData();
			if (!messageData || message.getRequestResponseCode() != SharedObject::RequestResponseCode::REQUEST)
			{
				connection->sendMessage(SharedObject(this_status_code, SharedObject::RequestResponseCode::ERROR,
						SharedObject::NULL_DATA));
				return;
			}
			auto request = RequestObject<ContestInfo>::deserialize(messageData.value());
			ContestInfo data = ContestInfo::deserialize(request.getData());
			std::string response = SharedObject::NULL_DATA;

			switch (request.getRequestCode())
			{
			case RequestObject<ContestInfo>::ADD:
			{
				auto schemasOpt = db.get(request.getDatabase());
				if (!schemasOpt)
				{
					auto schemas = std::make_shared<BPlusTreeMap<std::string, std::shared_ptr<Map<std::string,
							std::shared_ptr<Map<ContestInfo, Null>>>>>>(3, 3, stringComparer);
					if (!db.add(request.getDatabase(), schemas))
						throw std::runtime_error("Can't add database");
					schemasOpt.emplace(schemas);
				}
				auto schemas = schemasOpt.value();

				auto tablesOpt = schemas->get(request.getSchema());
				if (!tablesOpt)
				{
					auto tables = std::make_shared<BPlusTreeMap<std::string,
							std::shared_ptr<Map<ContestInfo, Null>>>>(3, 3, stringComparer);
					if (!schemas->add(request.getSchema(), tables))
						throw std::runtime_error("Can't add schema");
					tablesOpt.emplace(tables);
				}
				auto tables = tablesOpt.value();

				auto tableOpt = tables->get(request.getTable());
				if (!tableOpt)
				{
					auto table = std::make_shared<BPlusTreeMap<ContestInfo, Null>>(3, 3, contestInfoComparer);
					if (!tables->add(request.getTable(), table))
						throw std::runtime_error("Can't add table");
					tableOpt.emplace(table);
				}
				auto table = tableOpt.value();

				if (table->add(data, Null::value()))
					response = "true";
				else
					response = "false";
				break;
			}
			case RequestObject<ContestInfo>::CONTAINS:
			{
				bool contains = false;
				auto schemas = db.get(request.getDatabase());
				if (schemas)
				{
					auto tables = schemas.value()->get(request.getSchema());
					if (tables)
					{
						auto table = tables.value()->get(request.getTable());
						if (table)
						{
							contains = table.value()->contains(data);
						}
					}
				}
				if (contains)
					response = "true";
				else
					response = "false";
				break;
			}
			case RequestObject<ContestInfo>::REMOVE:
			{
				bool removed = false;
				auto schemas = db.get(request.getDatabase());
				if (schemas)
				{
					auto tables = schemas.value()->get(request.getSchema());
					if (tables)
					{
						auto table = tables.value()->get(request.getTable());
						if (table)
						{
							removed = table.value()->remove(data);
						}
					}
				}
				if (removed)
					response = "true";
				else
					response = "false";
				break;
			}
			default:
			{
				connection->sendMessage(SharedObject(this_status_code,
						SharedObject::RequestResponseCode::ERROR, response));
			}
			}
			connection->sendMessage(SharedObject(this_status_code,
					SharedObject::RequestResponseCode::OK, response));
		}

		// send log processing
//		if (!to_log.empty() && SharedObject::getStatusCode(connection_log->receiveMessage()) != this_status_code) {
//			connection_log->sendMessage(SharedObject(this_status_code, LOG, to_log.front()));
//			to_log.pop();
//		}
	}
};


#endif //PROGC_SRC_PROCESSORS_STORAGE_STORAGE_PROCESSOR_H
