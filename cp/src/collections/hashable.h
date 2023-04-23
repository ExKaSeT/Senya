
#ifndef PROGC_SRC_COLLECTIONS_HASHABLE_H
#define PROGC_SRC_COLLECTIONS_HASHABLE_H

#include <cstddef>

template<typename T>
class Hashable
{
public:

	[[nodiscard]] virtual size_t hashcode() const = 0;

	[[nodiscard]] virtual bool operator==(const T& other) const = 0;
};

#endif //PROGC_SRC_COLLECTIONS_HASHABLE_H
