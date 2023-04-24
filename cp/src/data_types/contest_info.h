

#ifndef PROGC_SRC_DATA_TYPES_CONTEST_INFO_H
#define PROGC_SRC_DATA_TYPES_CONTEST_INFO_H

#include <string>
#include "../collections/hashable.h"
#include "string_pool/string_pool.h"


class ContestInfo : public Hashable<ContestInfo>
{
private:

	int candidate_id;
	std::string& last_name;
	std::string& first_name;
	std::string& patronymic;
	std::string& birth_date;
	std::string& resume_link;
	int hr_manager_id;
	int contest_id;
	std::string& programming_language;
	int num_tasks;
	int solved_tasks;
	bool cheating_detected;

public:

	ContestInfo(int candidate_id,
		std::string& last_name,
		std::string& first_name,
		std::string& patronymic,
		std::string& birth_date,
		std::string& resume_link,
		int hr_manager_id,
		int contest_id,
		std::string& programming_language,
		int num_tasks,
		int solved_tasks,
		bool cheating_detected)
		: candidate_id(candidate_id), last_name(last_name), first_name(first_name), patronymic(patronymic),
		  birth_date(birth_date), resume_link(resume_link), hr_manager_id(hr_manager_id), contest_id(contest_id),
		  programming_language(programming_language), num_tasks(num_tasks), solved_tasks(solved_tasks),
		  cheating_detected(cheating_detected)
	{
		StringPool& string_pool = StringPool::instance();

		this->last_name = string_pool.get_string(last_name);
		this->first_name = string_pool.get_string(first_name);
		this->patronymic = string_pool.get_string(patronymic);
		this->birth_date = string_pool.get_string(birth_date);
		this->resume_link = string_pool.get_string(resume_link);
		this->programming_language = string_pool.get_string(programming_language);
	}

	static auto get_obj_for_search(int candidate_id, int contest_id)
	{
		std::string null;
		return ContestInfo(candidate_id, null, null, null, null, null, 0, contest_id, null, 0, 0, false);
	}

	size_t hashcode() const override
	{
		size_t h1 = std::hash<int>()(candidate_id);
		size_t h2 = std::hash<int>()(contest_id);
		return h1 ^ (h2 << 1);
	}

	bool operator==(const ContestInfo& other) const override
	{
		return this->candidate_id == other.candidate_id && this->contest_id == other.contest_id;
	}

	int GetCandidateId() const
	{
		return candidate_id;
	}
	const std::string& GetLastName() const
	{
		return last_name;
	}
	const std::string& GetFirstName() const
	{
		return first_name;
	}
	const std::string& GetPatronymic() const
	{
		return patronymic;
	}
	const std::string& GetBirthDate() const
	{
		return birth_date;
	}
	const std::string& GetResumeLink() const
	{
		return resume_link;
	}
	int GetHrManagerId() const
	{
		return hr_manager_id;
	}
	int GetContestId() const
	{
		return contest_id;
	}
	const std::string& GetProgrammingLanguage() const
	{
		return programming_language;
	}
	int GetNumTasks() const
	{
		return num_tasks;
	}
	int GetSolvedTasks() const
	{
		return solved_tasks;
	}
	bool IsCheatingDetected() const
	{
		return cheating_detected;
	}
};

#endif //PROGC_SRC_DATA_TYPES_CONTEST_INFO_H
