#include <functional>
#include <string>

/* class logger {
public:
    enum severity { trace, debug, information, warning, error, critical };
    virtual logger* log(const std::string& target, severity level) const = 0;
}; */

class memory {
public:
    virtual void* allocate(size_t target_size) const = 0;
    virtual void deallocate(void* target_to_dealloc) const = 0;
    void* operator+=(size_t target_size) const {
        return allocate(target_size);
    }
    void operator-=(void* target_to_dealloc) const {
        deallocate(target_to_dealloc);
    }
};

namespace bgnt {

    class bigint {
    public:
        virtual bigint& add(const bigint& summand) = 0;
        //virtual bigint sum(const bigint& summand) const { return bigint(*this).add(summand); }
        virtual bigint& subtract(const bigint& subtrahend) = 0;
        //virtual bigint subtraction(const bigint& subtrahend) const { return bigint(*this).subtract(subtrahend); }
        virtual bool lower_than(const bigint& other) const = 0;
        virtual bool greater_than(const bigint& other) const = 0;
        virtual bool lower_than_or_equal_to(const bigint& other) const = 0;
        virtual bool greater_than_or_equal_to(const bigint& other) const = 0;
        virtual bool equals(const bigint& other) const = 0;
        virtual bool not_equals(const bigint& other) const = 0;

        virtual bigint& operator+=(const bigint& summand) { return add(summand); }
        //virtual bigint operator+(const bigint& summand) const { return sum(summand); }
        virtual bigint& operator-=(const bigint& subtrahend) { return subtract(subtrahend); }
        //virtual bigint operator-(const bigint& subtrahend) const { return subtraction(subtrahend); }
        virtual bool operator<(const bigint& other) const { return lower_than(other); }
        virtual bool operator>(const bigint& other) const { return greater_than(other); }
        virtual bool operator<=(const bigint& other) const { return lower_than_or_equal_to(other); }
        virtual bool operator>=(const bigint& other) const { return greater_than_or_equal_to(other); }
        virtual bool operator==(const bigint& other) const { return equals(other); }
        virtual bool operator!=(const bigint& other) const { return not_equals(other); }
    };

    class bigint_multiplication {
    public:
        // virtual bigint multiply(const bigint& left_multiplier, const bigint& right_multiplier) const = 0;
    };

    class bigint_division {
    public:
        // virtual bigint divide(const bigint& dividend, const bigint& divider) const = 0;
    };

    class bigint_iterator {
    public:
        virtual ~bigint_iterator() {}
        virtual bigint_iterator& operator++() = 0;
        virtual bool operator==(const bigint_iterator& other) const = 0;
        virtual bool operator!=(const bigint_iterator& other) const = 0;
        virtual const bigint& operator*() const = 0;
        virtual const bigint* operator->() const = 0;
    };
}

template <typename tkey, typename tvalue, typename tkey_comparer = std::less<tkey>>
class associative_container {
public:
    virtual bool find(tkey target_key, tvalue& result_value) = 0;
    virtual void insert(const tkey& key, const tvalue& value) = 0;
    virtual void remove(const tkey& key, tvalue *removed_value = nullptr) = 0;
};
