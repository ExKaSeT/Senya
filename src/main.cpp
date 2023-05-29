

#include "data_types/contest_info.h"
#include "data_types/shared_object.h"
#include "processors/processor.h"
#include "processors/client/client_processor.h"
#include "processors/server/server_processor.h"
#include "processors/storage/storage_processor.h"
#include "logger/logger_builder.h"
#include "logger/logger_builder_concrete.h"


const std::string CON_MEM_NAME = "con_mem";
const std::string CON_MUTEX_NAME = "con_mutex";
const int SERVER_STATUS_CODE = 1;
const int CLIENT_STATUS_CODE = 2;
const int STORAGE_STATUS_CODE = 3;


int main()
{
	ContestInfo contestInfo(99, "last_name", "first_name", "patronymic", "birth_date", "resume_link",
		991, 992, "programming_language", 993, 994, true);
	ClientProcessor clientProcessor(CLIENT_STATUS_CODE, CON_MEM_NAME, CON_MUTEX_NAME);
	std::cout << clientProcessor.add("database2", "schema1", "table1", contestInfo);
	std::cout << clientProcessor.contains("database2", "schema1", "table1", contestInfo);
	std::cout << clientProcessor.remove("database2", "schema1", "table1", contestInfo);
	std::cout << clientProcessor.contains("database2", "schema1", "table1", contestInfo);


//	logger* logger = logger_builder_concrete::file_construct("log_settings.txt");
//	ServerProcessor serverProcessor(SERVER_STATUS_CODE, CON_MEM_NAME, CON_MUTEX_NAME, {"storage1", "storage2"});
//	while (true)
//		serverProcessor.process();
//	delete logger;


//	StorageProcessor storageProcessor(STORAGE_STATUS_CODE, "storage2");
//	while (true)
//		storageProcessor.process();


	return 0;
}