
#ifndef PROGC_SRC_DATA_TYPES_SHARED_OBJECT_H
#define PROGC_SRC_DATA_TYPES_SHARED_OBJECT_H

#include <sstream>
#include <utility>
#include <optional>
#include "../extensions/serializable.h"



/*
request_code:
 10 - add
 11 - contains
 12 - remove
 13 - log

response_code:
 20 - ok
 ?
 */

enum RequestResponseCode {
	ADD = 10,
	CONTAINS = 11,
	REMOVE = 12,
	LOG = 13,
	GET_CONNECTION = 14,
	CLOSE_CONNECTION = 15,
	OK = 20,
};

class SharedObject : public Serializable
{
private:

	char status_code; // первый байт = 0/1: обработались ли данные
	char request_response_code; // код запроса / ответа
	const std::string data; // "null" - NULL

public:

	static inline const std::string NULL_DATA = "null";

	SharedObject(int statusCode, RequestResponseCode requestResponseCode, const Serializable& data)
		: status_code(static_cast<char>(statusCode)),
		  request_response_code(static_cast<char>(requestResponseCode)), data(data.serialize())
	{
	}

	// data may be "null"
	SharedObject(int statusCode, int requestResponseCode, const std::string& data)
		: status_code(static_cast<char>(statusCode)),
		  request_response_code(static_cast<char>(requestResponseCode)), data(data)
	{
	}

	std::string serialize() const override {
		if (status_code == 0 || request_response_code == 0)
			throw std::runtime_error("SharedObject cant serialize '\\0'");
		std::stringstream ss;
		ss << status_code;
		ss << request_response_code;
		ss << data;
		return ss.str();
	}

	static SharedObject deserialize(const char* serializedSharedObject) {
		char status_code = *serializedSharedObject;
		serializedSharedObject++;
		char request_response_code = *serializedSharedObject;
		serializedSharedObject++;
		return {status_code, request_response_code, std::string(serializedSharedObject)};
	}

	static int GetStatusCode(const char* serializedSharedObject)
	{
		return *serializedSharedObject;
	}

	int GetStatusCode() const
	{
		return status_code;
	}
	RequestResponseCode GetRequestResponseCode() const
	{
		return static_cast<RequestResponseCode>(request_response_code);
	}
	std::optional<std::string> GetData() const
	{
		if (data == NULL_DATA)
			return std::nullopt;
		return { data };
	}

	void setStatusCode(int statusCode) {
		status_code = static_cast<char>(statusCode);
	}

	void print() {
		std::cout << "\nStatus code: " << status_code << "\nReqRes code: " << request_response_code << "\nData: " << data << std::endl;
	}
};

#endif //PROGC_SRC_DATA_TYPES_SHARED_OBJECT_H
