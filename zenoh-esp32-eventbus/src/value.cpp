#include <value.h>
#include <vector>
#include <cstdint>
#include <utility>


   

    Value Value::clone() const
    {
        if (is<NullType>())
        {
            return Value(nullptr);
        }
        else if (is<BoolType>())
        {
            return Value(as<BoolType>());
        }
        else if (is<IntType>())
        {
            return Value(as<IntType>());
        }
        else if (is<FloatType>())
        {
            return Value(as<FloatType>());
        }
        else if (is<StringType>())
        {
            return Value(as<StringType>());
        }
        else if (is<BytesType>())
        {
            return Value(as<BytesType>());
        }
        else if (is<ArrayType>())
        {
            ArrayType newArray;
            const auto &arr = as<ArrayType>();
            newArray.reserve(arr.size());
            for (const auto &item : arr)
            {
                newArray.push_back(item.clone());
            }
            return Value(std::move(newArray));
        }
        else if (is<ObjectType>())
        {
            ObjectType newObject;
            const auto &obj = std::get<ObjectType>(_value);
            for (const auto &[key, value] : obj)
            {
                newObject.emplace(key, value.clone());
            }
            return Value(std::move(newObject));
        }
        return Value(nullptr);
    }

    // Type checking
    /*   bool isNull() const { return std::holds_alternative<NullType>(_value); }
       bool isInt() const { return std::holds_alternative<IntType>(_value); }
       bool isFloat() const { return std::holds_alternative<FloatType>(_value); }
       bool isBool() const { return std::holds_alternative<BoolType>(_value); }
       bool isString() const { return std::holds_alternative<StringType>(_value); }
       bool isBytes() const { return std::holds_alternative<BytesType>(_value); }
       bool isArray() const { return std::holds_alternative<ArrayType>(_value); }
       bool isObject() const { return std::holds_alternative<ObjectType>(_value); }*/

    

   /* Value& Value::operator[](const std::string &key)
    {
        if (is<NullType>())
            _value = ObjectType();
        if (!is<ObjectType>())
            PANIC(" cannot index ");
        return std::get<ObjectType>(_value)[key];
    }

    const Value & Value::operator[](const std::string &key) const
    {
        return as<ObjectType>().at(key);
    }*/

    void Value::add(Value v)
    {
        if (is<Undefined>())
        {
            _value = ArrayType();
        }
        if (!is<ArrayType>())
        {
            PANIC(" cannot index ");
        }
        std::get<ArrayType>(_value).push_back(v);
    }

    // Convenience methods for array access
    Value & Value::operator[](size_t index)
    {
        if (is<Undefined>())
        {
            _value = ArrayType();
        }
        if (!is<ArrayType>())
        {
            PANIC(" cannot index ");
        }

        return std::get<ArrayType>(_value)[index];
    }

   
    size_t Value::size() const
    {
        if (is<ArrayType>())
            return as<ArrayType>().size();
        if (is<ObjectType>())
            return as<ObjectType>().size();
        if (is<StringType>())
            return as<StringType>().size();
        // PANIC("Type doesn't have size");
        return 0;
    }
    // CBOR

