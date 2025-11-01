#ifndef _VALUE_H_
#define _VALUE_H_
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <option.h>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <utility>
#include <result.h>

#define FLOAT_TYPE float
#define VAL_OR_RET(L, F)           \
    {                              \
        auto v = F;                \
        if (v.is_err())            \
            return Result<Value>::Err(v.err()->rc, v.err()->msg);              \
        L = v.unwrap(); \
    }

#define RET_ER(T, F)                               \
    {                                              \
        auto __r = F;                              \
        if (__r.is_err())                          \
            return Result<T>::Err(__r.err()->rc, __r.err()->msg); \
    }
    
class Value;

typedef const Value *SharedValue;

class Value
{
public:
    // Primitive types we support
    using NullType = std::nullptr_t;
    using IntType = int64_t;
    using FloatType = FLOAT_TYPE;
    using BoolType = bool;
    using StringType = std::string;
    using BytesType = std::vector<uint8_t>;

    // Container types (using shared_ptr to minimize copying)
    using ArrayType = std::vector<Value>;
    using ObjectType = std::unordered_map<std::string, Value>;
    struct Undefined
    {
    };

    // The actual storage type
    using InnerValue = std::variant<
        Undefined,
        NullType,
        IntType,
        FloatType,
        BoolType,
        StringType,
        BytesType,
        ArrayType,
        ObjectType>;

private:
    InnerValue _value;

    class Proxy
    {
    private:
        Value &_parent;
        std::string _key;

    public:
        Proxy(Value &value, const std::string &key)
            : _parent(value), _key(key)
        {
            // printf("Proxy created for key: %s\n", key.c_str());
        }

        // Conversion operator for reading
        operator Value() const
        {
            return _parent.get(_key);
        }

        // Assignment operator for writing
        Value operator=(const Value &val)
        {
            return _parent.set(_key, val);
        }
        void array()
        {
            _parent.set(_key, Value(ArrayType{}));
        }

        // Overloaded operator[] to allow nested access
        Proxy operator[](const std::string &key)
        {
            // Ensure the current value is a map
            if (_parent.get(_key).is<Undefined>())
            {
                _parent.set(_key, Value(ObjectType{}));
            }
            return Proxy(_parent.get_map()[_key], key);
        }

        void add(Value v)
        {
            if (_parent.get(_key).is<Undefined>())
            {
                _parent.set(_key, Value(ArrayType{}));
            }
            if (!_parent.get(_key).is<ArrayType>())
            {
                PANIC(" cannot index ");
            }
            std::get<ArrayType>(_parent.get_map()[_key]._value).push_back(v);
        }

        template <typename U, typename F>
        void handle(F &&func) const
        {
            if (!_parent.get(_key).is<U>())
            {
                return; // No action if type doesn't match
            }
            _parent.get(_key).handle<U>([&](const U &v)
                                        { func(v); });
        }

        template <typename U>
        bool is() const
        {
            return _parent.get(_key).is<U>();
        }
        template <typename U>
        const U &as() const
        {
            return _parent.get(_key).as<U>();
        }

        inline explicit operator bool()
        {
            return !_parent.is<NullType>();
        }
    };

public:
    // Constructors for primitive types
    inline Value() : _value(Undefined{}) {}
    inline Value &object()
    {
        _value = ObjectType{};
        return *this;
    }
    inline Value &array()
    {
        _value = ArrayType{};
        return *this;
    }
    /*
    Value &operator=(Value &&other)
    {
        _value = std::move(other._value);
        other._value = Value(Undefined{});
        return *this;
    }

    Value(const Value& other){
        _value = other._value;
    }*/

    //    inline Value() : _value(NullType{}) {}
    inline Value(std::nullptr_t) : _value(NullType{}) {}
    inline Value(int v) : _value(static_cast<IntType>(v)) {}
    inline Value(int64_t v) : _value(v) {}
    inline Value(FloatType v) : _value(v) {}
    inline Value(bool v) : _value(v) {}
    inline Value(const char *v) : _value(StringType(v)) {}
    inline Value(const std::string &v) : _value(v) {}
    inline Value(const std::vector<uint8_t> &v) : _value(v) {}
    inline Value(Undefined v) : _value(v) {}
    // Constructor for arrays (move semantics to minimize copies)
    inline Value(const ArrayType &arr) : _value(arr) {}
    inline Value(ArrayType &&arr) : _value(std::move(arr)) {}

    // Constructor for objects (move semantics)
    inline Value(const ObjectType &obj) : _value(obj) {}
    inline Value(ObjectType &&obj) : _value(std::move(obj)) {}
    // Get the underlying map (converts to map if not already one)
    ObjectType &get_map()
    {
        if (!is<ObjectType>())
        {
            _value = ObjectType{};
        }
        return std::get<ObjectType>(_value);
    }

    // Const version of get_map
    const ObjectType &get_map() const
    {
        if (!is<ObjectType>())
        {
            // return Value(Undefined{});
        }
        return std::get<ObjectType>(_value);
    }
    // Get value (const version, returns Undefined if doesn't exist)
    Value get(const std::string &key) const
    {
        if (!is<ObjectType>())
        {
            return Value(Undefined{});
        }
        const auto &map = get_map();
        auto it = map.find(key);
        if (it == map.end())
        {
            return Value(Undefined{});
        }
        return it->second;
    }

    // Set value (creates if doesn't exist)
    Value set(const std::string &key, Value val)
    {
        return get_map()[key] = val;
    }

    // Overloaded operator[] that returns a proxy
    Proxy operator[](const std::string &key)
    {
        return Proxy(*this, key);
    }

    // Const version of operator[] that returns Undefined for missing keys
    Value operator[](const std::string &key) const
    {
        return get(key);
    }

    /*
    // Overloaded operator[] that returns a proxy
        Proxy operator[](const std::string& key) {
            return Proxy(*this, key);
        }

        // Get value (const version, doesn't create if doesn't exist)
        Value get(const std::string& key) const {
            if (!is<ObjectType>())
                return NullType();
            auto it = std::get<ObjectType>(_value).find(key);
            if ( it ==  std::get<ObjectType>(_value).end() ){
                return NullType();
            }
            return it->second;
        }

        // Set value (creates if doesn't exist)
        Value& set(const std::string& key, const Value& value) {
            if (is<NullType>())
                _value = ObjectType();
            if (!is<ObjectType>())
                PANIC(" cannot index ");
             _value

            return *this;
        }

    */

public:
    template <typename T>
    bool is() const
    {
        return std::holds_alternative<T>(_value);
    }

    template <typename T>
    const T &as() const
    {
        return std::get<T>(_value);
    }

    Value clone() const;

    // Convenience methods for object access
    inline bool hasKey(const std::string &key) const
    {
        return is<ObjectType>() && as<ObjectType>().count(key) > 0;
    }

    inline explicit operator bool()
    {
        return !is<Undefined>();
    }

    /*    //Value &operator[](const std::string &key);

        const Value &operator[](const std::string &key) const;

        inline explicit operator bool()
        {
            return !is<NullType>();
        }
    */
    template <typename U, typename F>
    void handle(F &&func) const
    {
        if (is<U>())
        {
            func(as<U>());
        }
    }

    void add(Value v);

    // Convenience methods for array access
    Value &operator[](size_t index);

    inline const Value &operator[](size_t index) const
    {
        return as<ArrayType>().at(index);
    }

    template <typename U, typename F>
    auto inspect(F &&func) const -> Value
    {
        if (is<U>())
            func(as<U>());
        return *this;
    }

    template <typename U>
    auto set(U &target) const
    {
        if (is<U>())
            target = as<U>();
        return *this;
    }

    inline operator std::string()
    {
        return as<std::string>();
    }
    /*template<typename U>
    operator U(){
        return as<U>();
    }*/

    size_t size() const;
  
};



#endif
