#include "collections/b-tree/trees/vanilla_b_plus_tree.h"
#include "collections/hash_set.h"
#include "data_types/contest_info.h"


int main()
{
	auto tree = HashSet<ContestInfo>();
	ContestInfo contestInfo(99, "last_name", "first_name", "patronymic", "birth_date", "resume_link",
		991, 992, "programming_language", 993, 994, true);
	std::cout << contestInfo.getProgrammingLanguage() << "\n";

	std::cout << contestInfo.serialize();
	contestInfo = ContestInfo::deserialize(contestInfo.serialize());
	contestInfo.print();


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