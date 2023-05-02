
#ifndef PROGC_SRC_CONNECTION_CONNECTION_H
#define PROGC_SRC_CONNECTION_CONNECTION_H

#include <string>
#include <optional>
#include <boost/interprocess/mapped_region.hpp>
#include "../extensions/serializable.h"


class Connection
{
public:

	virtual const char* receiveMessage() const = 0;

	virtual void sendMessage(const Serializable&) const = 0;

	virtual ~Connection() {};
};

#endif //PROGC_SRC_CONNECTION_CONNECTION_H
