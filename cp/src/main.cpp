#include <vector>
#include "collections/b-tree/trees/vanilla_b_plus_tree.h"
#include "collections/hash_set.h"
#include "data_types/contest_info.h"


//void kek(VanillaBPlusTree<int, std::vector<std::string>, 2> &tree) {
//	std::vector<std::string> vec;
//	tree.search(10, vec);
////	std::cout << vec.at(0) << "\n\n";
//	vec.emplace_back("loool");
//	tree.insert(10, vec);
//
//}

int main()
{

	auto tree = HashSet<ContestInfo>();
	tree.add(ContestInfo(10, 10));
	tree.add(ContestInfo(10, 10));
	tree.add(ContestInfo(11, 11));
	tree.add(ContestInfo(12, 11));
	ContestInfo contestInfo(10, 10);

	std::cout << tree.contains(ContestInfo(12, 11), contestInfo) << contestInfo.candidate_id << "\n\n";


	return 0;
}