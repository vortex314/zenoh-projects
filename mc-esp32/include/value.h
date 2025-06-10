#ifndef _VALUE_H_
#define _VALUE_H_
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <iostream>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
//#define PANIC(S) { printf("  PANIC !! " __FILE__ ":" LINE_STRING " " S "\n");fflush(stdout);}
#define LOG(S) { printf(__FILE__ ":" LINE_STRING " " S "\n");fflush(stdout);}


class GenericValue
{
public:
    // Primitive types we support
    using NullType = std::nullptr_t;
    using IntType = int64_t;
    using FloatType = double;
    using BoolType = bool;
    using StringType = std::string;
    using BytesType = std::vector<uint8_t>;

    // Container types (using shared_ptr to minimize copying)
    using ArrayType = std::vector<GenericValue>;
    using ObjectType = std::unordered_map<std::string, GenericValue>;

    // The actual storage type
    using Value = std::variant<
        NullType,
        IntType,
        FloatType,
        BoolType,
        StringType,
        BytesType,
        std::shared_ptr<ArrayType>,
        std::shared_ptr<ObjectType>>;

private:
    Value value;

public:
    // Constructors for primitive types
    GenericValue() : value(NullType{}) {}
    GenericValue(std::nullptr_t) : value(NullType{}) {}
    GenericValue(int v) : value(static_cast<IntType>(v)) {}
    GenericValue(int64_t v) : value(v) {}
    GenericValue(double v) : value(v) {}
    GenericValue(bool v) : value(v) {}
    GenericValue(const char *v) : value(StringType(v)) {}
    GenericValue(const std::string &v) : value(v) {}
    GenericValue(const std::vector<uint8_t> &v) : value(v) {}

    // Constructor for arrays (move semantics to minimize copies)
    GenericValue(const ArrayType &arr) : value(std::make_shared<ArrayType>(arr)) {}
    GenericValue(ArrayType &&arr) : value(std::make_shared<ArrayType>(std::move(arr))) {}

    // Constructor for objects (move semantics)
    GenericValue(const ObjectType &obj) : value(std::make_shared<ObjectType>(obj)) {}
    GenericValue(ObjectType &&obj) : value(std::make_shared<ObjectType>(std::move(obj))) {}

    // Type checking
    bool isNull() const { return std::holds_alternative<NullType>(value); }
    bool isInt() const { return std::holds_alternative<IntType>(value); }
    bool isFloat() const { return std::holds_alternative<FloatType>(value); }
    bool isBool() const { return std::holds_alternative<BoolType>(value); }
    bool isString() const { return std::holds_alternative<StringType>(value); }
    bool isBytes() const { return std::holds_alternative<BytesType>(value); }
    bool isArray() const { return std::holds_alternative<std::shared_ptr<ArrayType>>(value); }
    bool isObject() const { return std::holds_alternative<std::shared_ptr<ObjectType>>(value); }

    // Getters with type safety
    IntType asInt() const
    {
        if (!isInt())
            PANIC("Not an integer");
        return std::get<IntType>(value);
    }

    FloatType asFloat() const
    {
        if (!isFloat())
            PANIC("Not a float");
        return std::get<FloatType>(value);
    }

    BoolType asBool() const
    {
        if (!isBool())
            PANIC("Not a boolean");
        return std::get<BoolType>(value);
    }

    const StringType &asString() const
    {
        if (!isString())
            PANIC("Not a string");
        return std::get<StringType>(value);
    }

    const BytesType &asBytes() const
    {
        if (!isBytes())
            PANIC("Not a string");
        return std::get<BytesType>(value);
    }

    // Array access
    ArrayType &asArray()
    {
        if (!isArray())
            PANIC("Not an array");
        return *std::get<std::shared_ptr<ArrayType>>(value);
    }

    const ArrayType &asArray() const
    {
        if (!isArray())
            PANIC("Not an array");
        return *std::get<std::shared_ptr<ArrayType>>(value);
    }

    // Object access
    ObjectType &asObject()
    {
        if (!isObject())
            PANIC("Not an object");
        return *std::get<std::shared_ptr<ObjectType>>(value);
    }

    const ObjectType &asObject() const
    {
        if (!isObject())
            PANIC("Not an object");
        return *std::get<std::shared_ptr<ObjectType>>(value);
    }

    // Memory optimization: return approximate memory usage
    size_t memoryUsage() const
    {
        size_t total = sizeof(GenericValue);

        if (isString())
        {
            total += asString().capacity();
        }
        else if (isArray())
        {
            total += sizeof(ArrayType);
            for (const auto &item : asArray())
            {
                total += item.memoryUsage();
            }
        }
        else if (isObject())
        {
            total += sizeof(ObjectType);
            for (const auto &[key, val] : asObject())
            {
                total += key.capacity() + val.memoryUsage();
            }
        }

        return total;
    }

    // Convenience methods for object access
    bool hasKey(const std::string &key) const
    {
        return isObject() && asObject().count(key) > 0;
    }

    GenericValue &operator[](const std::string &key)
    {
        if ( isNull() ) value = std::make_shared<ObjectType>();
        return asObject()[key];
    }

    const GenericValue &operator[](const std::string &key) const
    {
        return asObject().at(key);
    }

    // Convenience methods for array access
    GenericValue &operator[](size_t index)
    {
        if ( isNull() ) value = std::make_shared<ArrayType>();;
        return asArray()[index];
    }

    const GenericValue &operator[](size_t index) const
    {
        return asArray().at(index);
    }

    size_t size() const
    {
        if (isArray())
            return asArray().size();
        if (isObject())
            return asObject().size();
        if (isString())
            return asString().size();
        PANIC("Type doesn't have size");
    }
};

int testGenericValue()
{
    // Create a complex nested structure
    GenericValue person;
    person["name"] = "John Doe";
    person["age"] = 42;
    person["is_active"] = true;

    // Create an array
    GenericValue scores = GenericValue::ArrayType{95, 87, 91};
    person["scores"] = scores;

    // Create a nested object
    GenericValue address;
    address["street"] = "123 Main St";
    address["city"] = "Anytown";
    person["address"] = address;

    /*  // Access values
      std::cout << "Name: " << person["name"].asString() << "\n";
      std::cout << "First score: " << person["scores"][0].asInt() << "\n";
      std::cout << "City: " << person["address"]["city"].asString() << "\n";

      // Memory usage
      std::cout << "Approximate memory usage: " << person.memoryUsage() << " bytes\n";
      */
    return 0;
}

#endif
