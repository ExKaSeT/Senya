#include <vector>
#include "collections/b-tree/trees/vanilla_b_plus_tree.h"



void kek(VanillaBPlusTree<int, std::vector<std::string>, 2> &tree) {
	std::vector<std::string> vec;
	tree.search(10, vec);
//	std::cout << vec.at(0) << "\n\n";
	vec.emplace_back("loool");
	tree.insert(10, vec);

}

void lol(std::vector<std::string> &v) {
	v.emplace_back("lol");
}

int main()
{
	auto tree = VanillaBPlusTree<int, std::vector<std::string>, 2>();
	std::vector<std::string> vec = {"kek"};
//	tree.insert(10, vec);
	kek(tree);
	tree.search(10, vec);
	std::cout << vec.at(0) << "\n\n";


	return 0;
}