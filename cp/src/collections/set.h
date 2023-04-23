

#ifndef PROGC_SRC_COLLECTIONS_SET_H
#define PROGC_SRC_COLLECTIONS_SET_H

#include <cstddef>

template<typename T>
class Set {
public:

	virtual void add(const T& value) = 0;

	virtual T contains(const T& value) const = 0;

	virtual bool remove(const T& value) = 0;

	virtual void clear() = 0;

	virtual size_t size() const = 0;

	virtual bool empty() const = 0;

	virtual ~Set() {}
};

#endif //PROGC_SRC_COLLECTIONS_SET_H
