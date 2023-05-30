

#include "data_types/contest_info.h"
#include "data_types/shared_object.h"
#include "processors/processor.h"
#include "processors/client/client_processor.h"
#include "processors/server/server_processor.h"
#include "processors/storage/storage_processor.h"
#include "loggers/ostream_logger/logger_builder.h"
#include "loggers/ostream_logger/logger_builder_concrete.h"
#include "loggers/server_logger/server_logger.h"
#include "processors/log_server/log_server_processor.h"


const std::string CON_MEM_NAME = "con_mem";
const std::string CON_MUTEX_NAME = "con_mutex";
const int SERVER_STATUS_CODE = 1;
const int CLIENT_STATUS_CODE = 2;
const int STORAGE_STATUS_CODE = 3;
const int LOG_SERVER_STATUS_CODE = 4;
const std::string LOG_MEM_NAME = "log_mem";
const std::string LOG_MUTEX_NAME = "log_mutex";


int main()
{ // TODO:: with "Decorator" create multiple storage request from client (deletion db, schema, table)
	ContestInfo contestInfo(99, "last_name", "first_name", "patronymic", "birth_date", "resume_link",
			991, 992, "programming_language", 993, 994, true);
	ServerLogger serverLogger(LOG_SERVER_STATUS_CODE, LOG_MEM_NAME, LOG_MUTEX_NAME);
	ClientProcessor clientProcessor(CLIENT_STATUS_CODE, CON_MEM_NAME, CON_MUTEX_NAME, serverLogger);
	std::cout << clientProcessor.add("database2", "schema1", "table1", contestInfo);
	std::cout << clientProcessor.contains("database2", "schema1", "table1", contestInfo);
	std::cout << clientProcessor.remove("database2", "schema1", "table1", contestInfo);
	std::cout << clientProcessor.contains("database2", "schema1", "table1", contestInfo);
	std::cout << clientProcessor.add("database2", "schema1", "table1", contestInfo);
	clientProcessor.get("database2", "schema1", "table1", contestInfo).value().print();


//	ServerLogger serverLogger(LOG_SERVER_STATUS_CODE, LOG_MEM_NAME, LOG_MUTEX_NAME);
//	ServerProcessor serverProcessor(SERVER_STATUS_CODE, CON_MEM_NAME, CON_MUTEX_NAME,
//			{"storage1", "storage2"}, serverLogger);
//	while (true)
//		serverProcessor.process();


//	ServerLogger serverLogger(LOG_SERVER_STATUS_CODE, LOG_MEM_NAME, LOG_MUTEX_NAME);
//	StorageProcessor storageProcessor(STORAGE_STATUS_CODE, "storage2", serverLogger);
//	while (true)
//		storageProcessor.process();


//	logger* logger = logger_builder_concrete::file_construct("log_settings.txt");
//	LogServerProcessor logServerProcessor(LOG_SERVER_STATUS_CODE, LOG_MEM_NAME, LOG_MUTEX_NAME, logger);
//	while (true)
//		logServerProcessor.process();
//	delete logger;

	return 0;
}