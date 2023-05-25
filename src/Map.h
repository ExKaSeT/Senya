#ifndef PROGC_MAP_H
#define PROGC_MAP_H


#include <optional>
#include <vector>

template<typename K, typename V>
class Map {
public:

	virtual bool add(const K& key, const V& value) = 0;

	virtual std::optional<V> get(const K& key) = 0;

	virtual bool remove(const K& key) = 0;

	virtual bool set(const K& key, const V& newValue) = 0;

	virtual bool contains(const K& key) = 0;

	virtual std::vector<std::pair<const K&, const V&>> entrySet(const K& minBound, const K& maxBound) = 0;

	virtual size_t size() = 0;

	virtual ~Map() = default;
};


#endif //PROGC_MAP_H
