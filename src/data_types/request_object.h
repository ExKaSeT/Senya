#ifndef PROGC_REQUEST_OBJECT_H
#define PROGC_REQUEST_OBJECT_H


#include <string>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include "../extensions/serializable.h"
#include "../extensions/hashable.h"


template <typename T>
class RequestObject : public Serializable, public Hashable
{
	static_assert(std::is_base_of<Serializable, T>::value,
			"T must extend Serializable");
	static_assert(std::is_base_of<Hashable, T>::value,
			"T must extend Hashable");

private:

	const std::string database;
	const std::string schema;
	const std::string table;
	const std::string data; // "null" - NULL

public:

	static inline const std::string NULL_DATA = "null";

	RequestObject(const T& data, const std::string& database, const std::string& schema, const std::string& table)
			:  data(data.serialize()), database(database), schema(schema), table(table)
	{
	}

	RequestObject(const std::string& database, const std::string& schema, const std::string& table)
			: data(this->NULL_DATA), database(database), schema(schema), table(table)
	{
	}

	std::string serialize() const override {
		std::stringstream os;
		// TODO::
		os <<
		return os.str();
	}

	static RequestObject deserialize(const char* serializedSharedObject)
	{
		char status_code = *serializedSharedObject;
		serializedSharedObject++;
		char request_response_code = *serializedSharedObject;
		serializedSharedObject++;
		return { status_code, request_response_code, std::string(serializedSharedObject) };
	}
}


#endif //PROGC_REQUEST_OBJECT_H
