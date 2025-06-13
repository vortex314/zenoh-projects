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
#include <option.h>

#pragma once

class Value;

typedef std::shared_ptr< Value> SharedValue;

class Value
{
public:
    // Primitive types we support
    using NullType = std::nullptr_t;
    using IntType = int64_t;
    //   using FloatType = double;
#define FloatType double
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
        double,
        BoolType,
        StringType,
        BytesType,
        ArrayType,
        ObjectType>;

private:
    InnerValue _value;

public:
    // Constructors for primitive types
    Value() : _value(NullType{}) {}
    Value(std::nullptr_t) : _value(NullType{}) {}
    Value(int v) : _value(static_cast<IntType>(v)) {}
    Value(int64_t v) : _value(v) {}
    Value(double v) : _value(v) {}
    Value(bool v) : _value(v) {}
    Value(const char *v) : _value(StringType(v)) {}
    Value(const std::string &v) : _value(v) {}
    Value(const std::vector<uint8_t> &v) : _value(v) {}

    // Constructor for arrays (move semantics to minimize copies)
    Value(const ArrayType &arr) : _value(arr) {}
    Value(ArrayType &&arr) : _value(std::move(arr)) {}

    // Constructor for objects (move semantics)
    Value(const ObjectType &obj) : _value(obj) {}
    Value(ObjectType &&obj) : _value(std::move(obj)) {}

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

    Value clone() const
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
            const auto &obj = as<ObjectType>();
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

    // Convenience methods for object access
    bool hasKey(const std::string &key) const
    {
        return is<ObjectType>() && as<ObjectType>().count(key) > 0;
    }

    Value &operator[](const std::string &key)
    {
        if (is<NullType>())
            _value = ObjectType();
        if (!is<ObjectType>())
            PANIC(" cannot index ");
        return std::get<ObjectType>(_value)[key];
    }

    const Value &operator[](const std::string &key) const
    {
        return as<ObjectType>().at(key);
    }

    explicit operator bool()
    {
        return !is<NullType>();
    }

    template <typename U,typename F>
    void handle(F&& func) const {
        if ( is<U>() ) {
            func( as<U>() );
        }
    }


    void add(Value v)
    {
        if (is<NullType>())
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
    Value &operator[](size_t index)
    {
        if (is<NullType>())
        {
            _value = ArrayType();
        }
        if (!is<ArrayType>())
        {
            PANIC(" cannot index ");
        }

        return std::get<ArrayType>(_value)[index];
    }

    const Value &operator[](size_t index) const
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

    operator std::string()
    {
        return as<std::string>();
    }
    /*template<typename U>
    operator U(){
        return as<U>();
    }*/

    size_t size() const
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
    std::vector<uint8_t> toCbor() const;
    static Result<Value> fromCbor(const uint8_t *data, size_t size);
    static Result<Value> fromCbor(const std::vector<uint8_t> &data);
    // JSON
    std::string toJson(bool pretty = false, int indent = 0) const;
    static Result<Value> fromJson(const uint8_t *data, size_t size);
    static Result<Value> fromJson(const std::string &jsonStr);

    void serializeCbor(std::vector<uint8_t> &output) const;
    void serializeJson(std::ostream &os, bool pretty, int indent) const;
    void serializeString(std::ostream &os, const std::string &str) const;
    void serializeArray(std::ostream &os, bool pretty, int indent) const;
    void serializeObject(std::ostream &os, bool pretty, int indent) const;
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

    std::string encode(const uint8_t *buf, size_t bufLen)
    {
        std::string ret;
        int i = 0;
        int j = 0;
        uint8_t char_array_3[3];
        uint8_t char_array_4[4];

        while (bufLen--)
        {
            char_array_3[i++] = *(buf++);
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; i < 4; i++)
                    ret += chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; j < i + 1; j++)
                ret += chars[char_array_4[j]];

            while (i++ < 3)
                ret += '=';
        }

        return ret;
    }

    std::vector<uint8_t> decode(const std::string &encoded_string)
    {
        size_t in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        uint8_t char_array_4[4], char_array_3[3];
        std::vector<uint8_t> ret;

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
        {
            char_array_4[i++] = encoded_string[in_];
            in_++;
            if (i == 4)
            {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;

            for (j = 0; j < 4; j++)
                char_array_4[j] = chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++)
                ret.push_back(char_array_3[j]);
        }

        return ret;
    }
}

#include <vector>
#include <cstdint>
#include <utility>

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

// Serialize to CBOR binary data
std::vector<uint8_t> Value::toCbor() const
{
    std::vector<uint8_t> output;
    serializeCbor(output);
    return output;
}

// Deserialize from CBOR
Result<Value> Value::fromCbor(const std::vector<uint8_t> &data)
{
    return fromCbor(data.data(), data.size());
}

void Value::serializeCbor(std::vector<uint8_t> &output) const
{
    if (is<NullType>())
    {
        // CBOR null (0xf6)
        output.push_back(0xf6);
    }
    else if (is<BoolType>())
    {
        // CBOR boolean (0xf4 for false, 0xf5 for true)
        output.push_back(as<BoolType>() ? 0xf5 : 0xf4);
    }
    else if (is<IntType>())
    {
        serializeCborInt(output, as<IntType>());
    }
    else if (is<FloatType>())
    {
        // CBOR double-precision float (0xfb)
        output.push_back(0xfb);
        double d = as<FloatType>();
        const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&d);
        // Network byte order (big-endian)
        for (int i = sizeof(double) - 1; i >= 0; i--)
        {
            output.push_back(bytes[i]);
        }
    }
    else if (is<StringType>())
    {
        serializeCborString(output, as<StringType>());
    }
    else if (is<BytesType>())
    {
        serializeCborByteArray(output, as<BytesType>());
    }
    else if (is<ArrayType>())
    {
        serializeCborArray(output, as<ArrayType>());
    }
    else if (is<ObjectType>())
    {
        serializeCborObject(output, as<ObjectType>());
    }
}

void serializeCborInt(std::vector<uint8_t> &output, int64_t value)
{
    if (value >= 0)
    {
        if (value < 24)
        {
            output.push_back(static_cast<uint8_t>(value));
        }
        else if (value <= 0xff)
        {
            output.push_back(0x18);
            output.push_back(static_cast<uint8_t>(value));
        }
        else if (value <= 0xffff)
        {
            output.push_back(0x19);
            pushBigEndian(output, static_cast<uint16_t>(value));
        }
        else if (value <= 0xffffffff)
        {
            output.push_back(0x1a);
            pushBigEndian(output, static_cast<uint32_t>(value));
        }
        else
        {
            output.push_back(0x1b);
            pushBigEndian(output, value);
        }
    }
    else
    {
        const uint64_t posValue = -1 - value;
        if (posValue < 24)
        {
            output.push_back(static_cast<uint8_t>(0x20 + posValue));
        }
        else if (posValue <= 0xff)
        {
            output.push_back(0x38);
            output.push_back(static_cast<uint8_t>(posValue));
        }
        else if (posValue <= 0xffff)
        {
            output.push_back(0x39);
            pushBigEndian(output, static_cast<uint16_t>(posValue));
        }
        else if (posValue <= 0xffffffff)
        {
            output.push_back(0x3a);
            pushBigEndian(output, static_cast<uint32_t>(posValue));
        }
        else
        {
            output.push_back(0x3b);
            pushBigEndian(output, posValue);
        }
    }
}

void serializeCborString(std::vector<uint8_t> &output, const std::string &str)
{
    // Major type 3 (text string)
    serializeCborLength(output, 0x60, str.size());
    output.insert(output.end(), str.begin(), str.end());
}

void serializeCborByteArray(std::vector<uint8_t> &output, const Value::BytesType &data)
{
    // Major type 2 (byte string)
    serializeCborLength(output, 0x40, data.size());
    output.insert(output.end(), data.begin(), data.end());
}

void serializeCborArray(std::vector<uint8_t> &output, const Value::ArrayType &array)
{
    // Major type 4 (array)
    serializeCborLength(output, 0x80, array.size());
    for (const auto &item : array)
    {
        item.serializeCbor(output);
    }
}

void serializeCborObject(std::vector<uint8_t> &output, const Value::ObjectType &object)
{
    // Major type 5 (map)
    serializeCborLength(output, 0xa0, object.size());
    for (const auto &[key, value] : object)
    {
        serializeCborString(output, key);
        value.serializeCbor(output);
    }
}

void serializeCborLength(std::vector<uint8_t> &output, uint8_t majorType, size_t length)
{
    if (length < 24)
    {
        output.push_back(majorType + static_cast<uint8_t>(length));
    }
    else if (length <= 0xff)
    {
        output.push_back(majorType + 24);
        output.push_back(static_cast<uint8_t>(length));
    }
    else if (length <= 0xffff)
    {
        output.push_back(majorType + 25);
        pushBigEndian(output, static_cast<uint16_t>(length));
    }
    else if (length <= 0xffffffff)
    {
        output.push_back(majorType + 26);
        pushBigEndian(output, static_cast<uint32_t>(length));
    }
    else
    {
        output.push_back(majorType + 27);
        pushBigEndian(output, static_cast<uint64_t>(length));
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

    Result<Value> parseValue()
    {
        if (pos >= size)
            return Result<Value>(-1, "Unexpected end of CBOR data");

        uint8_t initialByte = data[pos++];
        uint8_t majorType = initialByte >> 5;
        uint8_t minorType = initialByte & 0x1f;
        auto r = parseLength(minorType);
        RET_ER(Value, r);
        uint64_t length = r.unwrap();

        switch (majorType)
        {
        case 0:
            return parseUnsignedInt(length); // unsigned integer
        case 1:
            return parseNegativeInt(length); // negative integer
        case 2:
            return parseByteString(length); // byte string
        case 3:
            return parseTextString(length); // text string
        case 4:
            return parseArray(length); // array
        case 5:
            return parseMap(length); // map
        case 6:
            return parseTaggedValue(length); // tag
        case 7:
            return parseSimpleValue(minorType, length); // simple value/float
        default:
            return Result<Value>(-1, "Invalid CBOR major type");
        }
    }

    Result<uint64_t> parseLength(uint8_t minorType)
    {
        if (minorType < 24)
        {
            return Result<uint64_t>(minorType);
        }
        else if (minorType == 24)
        {
            return readUint8();
        }
        else if (minorType == 25)
        {
            return readUint16();
        }
        else if (minorType == 26)
        {
            return readUint32();
        }
        else if (minorType == 27)
        {
            return readUint64();
        }
        else if (minorType == 31)
        {
            // Indefinite length (not fully supported here)
            return Result<uint64_t>(-1, "Indefinite length items not supported");
        }
        else
        {
            return Result<uint64_t>(-1, "Reserved minor type");
        }
    }

    Value parseUnsignedInt(uint64_t value)
    {
        return Value(static_cast<int64_t>(value));
    }

    Value parseNegativeInt(uint64_t value)
    {
        return Value(-1 - static_cast<int64_t>(value));
    }

    Result<Value> parseByteString(uint64_t length)
    {
        return checkAvailable(length).and_then([&](auto _v)
                                               {
            Value::BytesType byteArray(data + pos, data + pos + length);
        pos += length;
        return Value(std::move(byteArray)); });
    }

    Result<Value> parseTextString(uint64_t length)
    {
        return checkAvailable(length).and_then([&](auto _v)
                                               {
        std::string str(reinterpret_cast<const char *>(data + pos), length);
        pos += length;
        return Value(std::move(str)); });
    }

    Result<Value> parseArray(uint64_t length)
    {
        Value::ArrayType array;
        array.reserve(length);
        for (uint64_t i = 0; i < length; i++)
        {
            auto r = parseValue().and_then([&](const Value& v)
                                           { array.push_back(v); return 0;});
            if (r.is_err())
                return r;
        }
        return Value(std::move(array));
    }

    Result<Value> parseMap(uint64_t length)
    {
        Value::ObjectType object;
        for (uint64_t i = 0; i < length; i++)
        {
            // Key must be a string in our implementation
            auto k = parseValue();
            if (k.is_err() || k.unwrap().is<Value::StringType>())
            {
                return Result<Value>(-1, "CBOR map key must be a string");
            }
            parseValue().and_then([&](auto v)
                                  { object.emplace(k.unwrap().as<Value::StringType>(), v); return 0; });
        }
        return Value(std::move(object));
    }

    Result<Value> parseTaggedValue(uint64_t tag)
    {
        // For simplicity, we just skip tags in this implementation
        // A more complete implementation would handle specific tags
        return parseValue();
    }

    Result<Value> parseSimpleValue(uint8_t minorType, uint64_t length)
    {
        if (minorType == 20)
            return Value(false);
        if (minorType == 21)
            return Value(true);
        if (minorType == 22)
            return Value(nullptr);
        if (minorType == 23)
            return Value(nullptr); // undefined (treated as null)

        if (minorType == 25)
        {
            // Half-precision float (not directly supported)
            return Result<Value>(-1, "Half-precision float not supported");
        }
        else if (minorType == 26)
        {
            // Single-precision float
            return readFloat().and_then([&](auto f)
                                        { return Value(static_cast<double>(f)); });
        }
        else if (minorType == 27)
        {
            return readDouble().and_then([&](auto f)
                                         { return Value(static_cast<double>(f)); });
        }

        return Result<Value>(-1, "Unsupported simple value");
    }

    // Helper functions
    Result<Void> checkAvailable(size_t needed) const
    {
        if (pos + needed > size)
        {
            return Result<Void>(-1, "Unexpected end of CBOR data");
        }
        return ResOk;
    }

    Result<uint8_t> readUint8()
    {
        RET_ER(uint8_t, checkAvailable(1));

        return data[pos++];
    }

    Result<uint16_t> readUint16()
    {
        RET_ER(uint8_t, checkAvailable(2));
        uint16_t value = (static_cast<uint16_t>(data[pos]) << 8) | data[pos + 1];
        pos += 2;
        return value;
    }

    Result<uint32_t> readUint32()
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

    Result<uint64_t> readUint64()
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

    Result<float> readFloat()
    {
        return readUint32().and_then([&](uint32_t v)
                                     { return *reinterpret_cast<float *>(&v); });
    }

    Result<dmainouble> readDouble()
    {
        return readUint64().and_then([&](uint64_t v)
                                     { return *reinterpret_cast<double *>(&v); });
    }

    const uint8_t *data;
    size_t size;
    size_t pos;
};

// Implement the static fromCbor method
Result<Value> Value::fromCbor(const uint8_t *data, size_t size)
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

    Result<Value> parseValue()
    {
        skipWhitespace();
        char c = peekChar();

        if (c == '{')
        {
            return parseObject();
        }
        else if (c == '[')
        {
            return parseArray();
        }
        else if (c == '"')
        {
            return parseString();
        }
        else if (c == 't' || c == 'f')
        {
            return parseBoolean();
        }
        else if (c == 'n')
        {
            return parseNull();
        }
        else if (c == '-' || (c >= '0' && c <= '9'))
        {
            return parseNumber();
        }
        else
        {
            return Result<Value>(-1, "Unexpected character in JSON");
        }
    }

    Result<Value> parseObject()
    {
        RET_ER(Value, expectChar('{'));
        Value::ObjectType obj;

        skipWhitespace();
        if (peekChar() == '}')
        {
            getChar(); // consume '}'
            return Value(std::move(obj));
        }

        while (true)
        {
            skipWhitespace();
            std::string key = parseString().unwrap().as<Value::StringType>();

            skipWhitespace();
            expectChar(':');

            Value value = parseValue().unwrap();
            obj.emplace(std::move(key), std::move(value));

            skipWhitespace();
            char c;
            VAL_OR_RET(c, getChar());
            if (c == '}')
                break;
            if (c != ',')
                return Result<Value>(-1, FILE_LINE "Expected ',' or '}' in object");
        }

        return Value(std::move(obj));
    }

    Result<Value> parseArray()
    {
        RET_ER(Value, expectChar('['));
        Value::ArrayType arr;

        skipWhitespace();
        if (peekChar() == ']')
        {
            char c;
            VAL_OR_RET(c, getChar()); // consume char
            (void)c;
            return Value(std::move(arr));
        }

        while (true)
        {
            RET_ER(Value, parseValue().and_then([&](auto v)
                                                { arr.push_back(v); return 0;}));

            skipWhitespace();
            char c;
            VAL_OR_RET(c, getChar());
            if (c == ']')
                break;
            if (c != ',')
                return Result<Value>(-1, "Expected ',' or ']' in array");
        }

        return Value(std::move(arr));
    }

    Result<Value> parseString()
    {
        expectChar('"');
        std::string str;

        while (true)
        {
            char c;
            VAL_OR_RET(c, getChar());
            if (c == '"')
                break;
            if (c == '\\')
            {
                VAL_OR_RET(c, getChar());
                switch (c)
                {
                case '"':
                    str += '"';
                    break;
                case '\\':
                    str += '\\';
                    break;
                case '/':
                    str += '/';
                    break;
                case 'b':
                    str += '\b';
                    break;
                case 'f':
                    str += '\f';
                    break;
                case 'n':
                    str += '\n';
                    break;
                case 'r':
                    str += '\r';
                    break;
                case 't':
                    str += '\t';
                    break;
                case 'u':
                {
                    // Simplified Unicode handling (should parse 4 hex digits)
                    str += "\\u";
                    for (int i = 0; i < 4; i++)
                    {
                        VAL_OR_RET(c, getChar());
                        if (!isxdigit(c))
                            return Result<Value>(-1, "Invalid Unicode escape");
                        str += c;
                    }
                    break;
                }
                default:
                    return Result<Value>(-1, "Invalid escape sequence");
                }
            }
            else
            {
                str += c;
            }
        }

        return Value(std::move(str));
    }

    Result<Value> parseNumber()
    {
        std::string numStr;
        char c = peekChar();

        if (c == '-')
        {
            char d;
            VAL_OR_RET(d, getChar());
            numStr += d;
            c = peekChar();
        }

        while (c >= '0' && c <= '9')
        {
            char d;
            VAL_OR_RET(d, getChar());
            numStr += d;
            c = peekChar();
        }

        if (c == '.')
        {
            char d;
            VAL_OR_RET(d, getChar());
            numStr += d;
            c = peekChar();
            while (c >= '0' && c <= '9')
            {
                char d;
                VAL_OR_RET(d, getChar());
                numStr += d;
                c = peekChar();
            }

            // Check for scientific notation
            if (c == 'e' || c == 'E')
            {
                char d;
                VAL_OR_RET(d, getChar());
                numStr += d;
                c = peekChar();
                if (c == '+' || c == '-')
                {
                    char d;
                    VAL_OR_RET(d, getChar());
                    numStr += d;
                    c = peekChar();
                }
                while (c >= '0' && c <= '9')
                {
                    char d;
                    VAL_OR_RET(d, getChar());
                    numStr += d;
                    c = peekChar();
                }
            }

            return Value(std::stod(numStr));
        }

        return Value(std::stoll(numStr));
    }

    Result<Value> parseBoolean()
    {
        if (peekChar() == 't')
        {
            RET_ER(Value, expectString("true"));
            return Value(true);
        }
        else
        {
            RET_ER(Value, expectString("false"));
            return Value(false);
        }
    }

    Result<Value> parseNull()
    {
        expectString("null");
        return Value(nullptr);
    }

    // Helper functions
    char peekChar()
    {
        if (pos >= input.size())
            return '\0';
        return input[pos];
    }

    Result<char> getChar()
    {
        if (pos >= input.size())
            return Result<char>(-1, "Unexpected end of JSON");
        return input[pos++];
    }

    void skipWhitespace()
    {
        while (pos < input.size() && isspace(input[pos]))
        {
            pos++;
        }
    }

    Result<Void> expectChar(char expected)
    {
        char c;
        VAL_OR_RET(c, getChar());
        if (c != expected)
        {
            return Result<Void>(-1, "Expected char not found ");
        }
        return ResOk;
    }

    Result<Void> expectString(const std::string &expected)
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

// Implement the static fromJson method
Result<Value> Value::fromJson(const std::string &jsonStr)
{
    return JsonParser::parse(jsonStr);
}

#include <sstream>
#include <iomanip>

// Serialize to JSON string
std::string Value::toJson(bool pretty, int indent) const
{
    std::ostringstream oss;
    serializeJson(oss, pretty, indent);
    return oss.str();
}

// Static method to parse JSON

void Value::serializeJson(std::ostream &os, bool pretty, int indent) const
{
    if (is<NullType>())
    {
        os << "null";
    }
    else if (is<BoolType>())
    {
        os << (as<BoolType>() ? "true" : "false");
    }
    else if (is<IntType>())
    {
        os << as<IntType>();
    }
    else if (is<FloatType>())
    {
        os << as<FloatType>();
    }
    else if (is<StringType>())
    {
        serializeString(os, as<StringType>());
    }
    else if (is<ArrayType>())
    {
        serializeArray(os, pretty, indent);
    }
    else if (is<ObjectType>())
    {
        serializeObject(os, pretty, indent);
    }
}

void Value::serializeString(std::ostream &os, const std::string &str) const
{
    os << std::quoted(str);
}

void Value::serializeArray(std::ostream &os, bool pretty, int indent) const
{
    os << "[";
    const auto &arr = as<ArrayType>();
    const size_t size = arr.size();

    for (size_t i = 0; i < size; ++i)
    {
        if (pretty && size > 1)
        {
            os << "\n"
               << std::string(indent + 2, ' ');
        }

        arr[i].serializeJson(os, pretty, indent + 2);

        if (i < size - 1)
        {
            os << ",";
            if (pretty && size <= 1)
                os << " ";
        }
    }

    if (pretty && size > 1)
    {
        os << "\n"
           << std::string(indent, ' ');
    }
    os << "]";
}

void Value::serializeObject(std::ostream &os, bool pretty, int indent) const
{
    os << "{";
    const auto &obj = as<ObjectType>();
    const size_t size = obj.size();
    size_t count = 0;

    for (const auto &[key, value] : obj)
    {
        if (pretty && size > 1)
        {
            os << "\n"
               << std::string(indent + 2, ' ');
        }

        serializeString(os, key);
        os << (pretty ? ": " : ":");
        value.serializeJson(os, pretty, indent + 2);

        if (++count < size)
        {
            os << ",";
            if (pretty && size <= 1)
                os << " ";
        }
    }

    if (pretty && size > 1)
    {
        os << "\n"
           << std::string(indent, ' ');
    }
    os << "}";
}

#endif
