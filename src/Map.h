#ifndef PROGC_MAP_H
#define PROGC_MAP_H


#include <optional>
#include <vector>


template<typename K, typename V>
class Map
{
public:

	class Pair final
	{
	private:

		const K* const key;
		const V* const value;

	public:

		Pair(const K* key, const V* value) : key(key), value(value)
		{
		}

		const K& getKey() const
		{
			return *key;
		}

		std::optional<const V&> getValue() const
		{
			if (value == nullptr)
				return std::nullopt;
			return *value;
		}
	};

	virtual bool add(const K& key, const V& value) = 0;

	virtual std::optional<V> get(const K& key) = 0;

//	virtual bool remove(const K& key) = 0;

	virtual bool set(const K& key, const V& newValue) = 0;

	virtual bool contains(const K& key) = 0;

	virtual std::vector<Pair> entrySet(const K& minBound, const K& maxBound) = 0;

	virtual size_t size() = 0;

	virtual ~Map() = default;
};

class Null final
{
private:

	Null()
	{
	}
public:

	static const Null& value()
	{
		static Null null;
		return null;
	}
};


#endif //PROGC_MAP_H
