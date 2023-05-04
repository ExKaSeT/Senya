//#include "collections/b-tree/trees/vanilla_b_plus_tree.h"
//#include "collections/hash_set.h"
#include "data_types/contest_info.h"
#include "data_types/shared_object.h"
#include "processors/processor.h"
#include "processors/client/client_processor.h"
#include "processors/server/server_processor.h"
#include "processors/storage/storage_processor.h"



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
//	clientProcessor.add(contestInfo);
	clientProcessor.contains(contestInfo).value().print();

//	ServerProcessor serverProcessor(SERVER_STATUS_CODE, CON_MEM_NAME, CON_MUTEX_NAME, {"storage1", "storage2"});
//	while (true)
//		serverProcessor.process();

//	StorageProcessor storageProcessor(STORAGE_STATUS_CODE, "storage2");
//	while (true)
//		storageProcessor.process();

//	system("chcp 1251");
//	setbuf(stdout, 0);

//	ClientConnection clientConnection(CON_MEM_NAME, CON_MUTEX_NAME);
//	getchar();

//	ServerConnection clientConnection(CON_MEM_NAME, CON_MUTEX_NAME);
//	getchar();





//	auto tree = HashSet<ContestInfo>();
//	ContestInfo contestInfo(99, "last_name", "first_name", "patronymic", "birth_date", "resume_link",
//		991, 992, "programming_language", 993, 994, true);
//
//	SharedObject sharedObject(1, 1, contestInfo.serialize());
//
//	std::cout << sharedObject.serialize() << std::endl;
//
//	auto shared = SharedObject::deserialize(sharedObject.serialize().c_str());
//	ContestInfo::deserialize(shared.GetData()).print();

//	std::cout << contestInfo.getProgrammingLanguage() << "\n";
//
//	std::cout << contestInfo.serialize();
//	contestInfo = ContestInfo::deserialize(contestInfo.serialize());
//	contestInfo.print();


//	tree.add(contestInfo);
//	tree.add(ContestInfo::get_obj_for_search(10, 10));
//	tree.add(ContestInfo::get_obj_for_search(11, 11));
//	tree.add(ContestInfo::get_obj_for_search(12, 11));
//	std::optional<ContestInfo> opt = tree.contains(ContestInfo::get_obj_for_search(99, 992));
//
//	std::cout << (opt ? opt.value().getCandidateId() : -1) << contestInfo.getCandidateId() << "\n\n";

//	StringPool stringPool = StringPool::instance();
//	auto ref = stringPool.get_string("KEK");
//	auto ref1 = stringPool.get_string("KEK");
//	stringPool.unget_string(ref);
//	stringPool.unget_string(ref);
//	stringPool.unget_string(ref);
//	std::cout << ref << " ---\n";


	return 0;
}