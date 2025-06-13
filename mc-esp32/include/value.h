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
// #define VAL_OR_RET(V,RF)  { auto __r=RF; if (__r.is_err()) return __r;V=__r.unwrap();}
class Value;

typedef std::shared_ptr<Value> SharedValue;

class Value
{
public:
    // Primitive types we support
    using NullType = std::nullptr_t;
    using IntType = int64_t;
    //   using FloatType = double;
#define FloatType FLOAT_TYPE
    using BoolType = bool;
    using StringType = std::string;
    using BytesType = std::vector<uint8_t>;

    // Container types (using shared_ptr to minimize copying)
    using ArrayType = std::vector<Value>;
    using ObjectType = std::unordered_map<std::string, Value>;

    // The actual storage type
    using InnerValue = std::variant<
        NullType,
        IntType,
        FLOAT_TYPE,
        BoolType,
        StringType,
        BytesType,
        ArrayType,
        ObjectType>;

private:
    InnerValue _value;

public:
    // Constructors for primitive types
    inline Value() : _value(NullType{}) {}
    inline Value(std::nullptr_t) : _value(NullType{}) {}
    inline Value(int v) : _value(static_cast<IntType>(v)) {}
    inline Value(int64_t v) : _value(v) {}
    inline Value(FLOAT_TYPE v) : _value(v) {}
    inline Value(bool v) : _value(v) {}
    inline Value(const char *v) : _value(StringType(v)) {}
    inline Value(const std::string &v) : _value(v) {}
    inline Value(const std::vector<uint8_t> &v) : _value(v) {}

    // Constructor for arrays (move semantics to minimize copies)
    inline Value(const ArrayType &arr) : _value(arr) {}
    inline Value(ArrayType &&arr) : _value(std::move(arr)) {}

    // Constructor for objects (move semantics)
    inline Value(const ObjectType &obj) : _value(obj) {}
    inline Value(ObjectType &&obj) : _value(std::move(obj)) {}

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

    Value &operator[](const std::string &key);

    const Value &operator[](const std::string &key) const;

    inline explicit operator bool()
    {
        return !is<NullType>();
    }

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
    // CBOR
    std::vector<uint8_t> toCbor() const;
    static Result<Value> fromCbor(const uint8_t *data, size_t size);
    static Result<Value> fromCbor(const std::vector<uint8_t> &data);
    // JSON
    std::string toJson(bool pretty = false, int indent = 0) const;
    static Result<Value> fromJson(const uint8_t *data, size_t size);
    static Result<Value> fromJson(const std::string &jsonStr);

    void serializeCbor(std::vector<uint8_t> &output) const;
    void serializeJson(std::string &os, bool pretty, int indent) const;
    void serializeJsonString(std::string &os, const std::string &str) const;
    void serializeJsonArray(std::string &os, bool pretty, int indent) const;
    void serializeJsonObject(std::string &os, bool pretty, int indent) const;
};

#define RET_ER(T, F)                               \
    {                                              \
        auto __r = F;                              \
        if (__r.is_err())                          \
            return Result<T>(__r.rc(), __r.msg()); \
    }

namespace Base64
{
    static const std::string chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static inline bool is_base64(uint8_t c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string encode(const uint8_t *buf, size_t bufLen);

    std::vector<uint8_t> decode(const std::string &encoded_string);
};

void serializeCborInt(std::vector<uint8_t> &output, int64_t value);
void serializeCborString(std::vector<uint8_t> &output, const std::string &str);
void serializeCborByteArray(std::vector<uint8_t> &output, const Value::BytesType &data);
void serializeCborArray(std::vector<uint8_t> &output, const Value::ArrayType &array);
void serializeCborObject(std::vector<uint8_t> &output, const Value::ObjectType &object);
void serializeCborLength(std::vector<uint8_t> &output, uint8_t majorType, size_t length);
template <typename T>
void pushBigEndian(std::vector<uint8_t> &output, T value)
{
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&value);
    for (int i = sizeof(T) - 1; i >= 0; i--)
    {
        output.push_back(bytes[i]);
    }
}

#include <stdexcept>

class CborParser
{
public:
    static Result<Value> parse(const uint8_t *data, size_t size)
    {
        CborParser parser(data, size);
        return parser.parseValue();
    }

private:
    CborParser(const uint8_t *data, size_t size)
        : data(data), size(size), pos(0) {}

    Result<Value> parseValue();
    Result<uint64_t> parseLength(uint8_t minorType);

    inline Value parseUnsignedInt(uint64_t value)
    {
        return Value(static_cast<int64_t>(value));
    }

    inline Value parseNegativeInt(uint64_t value)
    {
        return Value(-1 - static_cast<int64_t>(value));
    }

    Result<Value> parseByteString(uint64_t length);

    Result<Value> parseTextString(uint64_t length);

    Result<Value> parseArray(uint64_t length);

    Result<Value> parseMap(uint64_t length);

    inline Result<Value> parseTaggedValue(uint64_t tag)
    {
        // For simplicity, we just skip tags in this implementation
        // A more complete implementation would handle specific tags
        return parseValue();
    }

    Result<Value> parseSimpleValue(uint8_t minorType, uint64_t length);

    // Helper functions
    inline Result<Void> checkAvailable(size_t needed) const
    {
        if (pos + needed > size)
        {
            return Result<Void>(-1, "Unexpected end of CBOR data");
        }
        return ResOk;
    }

    inline Result<uint8_t> readUint8()
    {
        RET_ER(uint8_t, checkAvailable(1));

        return data[pos++];
    }

    inline Result<uint16_t> readUint16()
    {
        RET_ER(uint8_t, checkAvailable(2));
        uint16_t value = (static_cast<uint16_t>(data[pos]) << 8) | data[pos + 1];
        pos += 2;
        return value;
    }

    inline Result<uint32_t> readUint32()
    {
        return checkAvailable(4).and_then([&](Void v)
                                          {
        uint32_t value = (static_cast<uint32_t>(data[pos]) << 24) |
                         (static_cast<uint32_t>(data[pos + 1]) << 16) |
                         (static_cast<uint32_t>(data[pos + 2]) << 8) |
                         data[pos + 3];
        pos += 4;
        return value; });
    }

    inline Result<uint64_t> readUint64()
    {
        return checkAvailable(4).and_then([&](Void v)
                                          {
        uint64_t value = (static_cast<uint64_t>(data[pos]) << 56) |
                         (static_cast<uint64_t>(data[pos + 1]) << 48) |
                         (static_cast<uint64_t>(data[pos + 2]) << 40) |
                         (static_cast<uint64_t>(data[pos + 3]) << 32) |
                         (static_cast<uint64_t>(data[pos + 4]) << 24) |
                         (static_cast<uint64_t>(data[pos + 5]) << 16) |
                         (static_cast<uint64_t>(data[pos + 6]) << 8) |
                         data[pos + 7];
        pos += 8;
        return value; });
    }

    inline Result<float> readFloat()
    {
        return readUint32().and_then([&](uint32_t v)
                                     { return *reinterpret_cast<float *>(&v); });
    }

    inline Result<double> readDouble()
    {
        return readUint64().and_then([&](uint64_t v)
                                     { return *reinterpret_cast<double *>(&v); });
    }

    const uint8_t *data;
    size_t size;
    size_t pos;
};

// Implement the static fromCbor method
inline Result<Value> Value::fromCbor(const uint8_t *data, size_t size)
{
    return CborParser::parse(data, size);
}

#include <stack>
#include <stdexcept>

class JsonParser
{
public:
    static Result<Value> parse(const std::string &jsonStr)
    {
        JsonParser parser(jsonStr);
        return parser.parseValue();
    }

private:
    JsonParser(const std::string &str) : input(str), pos(0) {}

    Result<Value> parseValue();
    Result<Value> parseObject();

    Result<Value> parseArray();
    Result<Value> parseString();
    Result<Value> parseNumber();
    Result<Value> parseBoolean();
    Result<Value> parseNull();

    // Helper functions
    inline char peekChar()
    {
        if (pos >= input.size())
            return '\0';
        return input[pos];
    }

    inline Result<char> getChar()
    {
        if (pos >= input.size())
            return Result<char>(-1, "Unexpected end of JSON");
        return input[pos++];
    }

    inline void skipWhitespace()
    {
        while (pos < input.size() && isspace(input[pos]))
        {
            pos++;
        }
    }

    inline Result<Void> expectChar(char expected)
    {
        char c;
        VAL_OR_RET(c, getChar());
        if (c != expected)
        {
            return Result<Void>(-1, "Expected char not found ");
        }
        return ResOk;
    }

    inline Result<Void> expectString(const std::string &expected)
    {
        for (char c : expected)
        {
            auto r = expectChar(c);
            if (r.is_err())
                return r;
        }
        return ResOk;
    }

    std::string input;
    size_t pos;
};

#endif
