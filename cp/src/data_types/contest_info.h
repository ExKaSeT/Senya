

#ifndef PROGC_SRC_DATA_TYPES_CONTEST_INFO_H
#define PROGC_SRC_DATA_TYPES_CONTEST_INFO_H

#include <string>
#include "../collections/hashable.h"

class ContestInfo : public Hashable<ContestInfo> {
private:
	int candidate_id;
	std::string last_name;
	std::string first_name;
	std::string patronymic;
	std::string birth_date;
	std::string resume_link;
	int hr_manager_id;
	int contest_id;
	std::string programming_language;
	int num_tasks;
	int solved_tasks;
	bool cheating_detected;

public:
	ContestInfo(int candidate_id, int contest_id) : candidate_id(candidate_id), contest_id(contest_id) {}

	size_t hashcode() const override {
		size_t h1 = std::hash<int>()(candidate_id);
		size_t h2 = std::hash<int>()(contest_id);
		return h1 ^ (h2 << 1);
	}

	bool operator==(const ContestInfo& other) const override {
		return this->candidate_id == other.candidate_id && this->contest_id == other.contest_id;
	}
};

#endif //PROGC_SRC_DATA_TYPES_CONTEST_INFO_H
