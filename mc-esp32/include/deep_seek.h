#include <vector>
#include <cstdint>
#include <string>
#include <variant>

class GenericValue {
public:
    // Add ByteArray type to supported types
    using ByteArray = std::vector<uint8_t>;
    
    // Update the Value variant to include ByteArray
    using Value = std::variant<
        NullType,
        IntType,
        FloatType,
        BoolType,
        StringType,
        ByteArray,  // New type
        std::shared_ptr<ArrayType>,
        std::shared_ptr<ObjectType>
    >;

    // ... rest of existing class definition ...
};

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace Base64 {
    static const std::string chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static inline bool is_base64(uint8_t c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string encode(const uint8_t* buf, size_t bufLen) {
        std::string ret;
        int i = 0;
        int j = 0;
        uint8_t char_array_3[3];
        uint8_t char_array_4[4];

        while (bufLen--) {
            char_array_3[i++] = *(buf++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; i <4; i++)
                    ret += chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; j < i + 1; j++)
                ret += chars[char_array_4[j]];

            while(i++ < 3)
                ret += '=';
        }

        return ret;
    }

    std::vector<uint8_t> decode(const std::string& encoded_string) {
        size_t in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        uint8_t char_array_4[4], char_array_3[3];
        std::vector<uint8_t> ret;

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i ==4) {
                for (i = 0; i <4; i++)
                    char_array_4[i] = chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }

        if (i) {
            for (j = i; j <4; j++)
                char_array_4[j] = 0;

            for (j = 0; j <4; j++)
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

class GenericValue {
    // ... existing code ...

public:
    // Add constructor for byte arrays
    GenericValue(const ByteArray& data) : value(data) {}
    GenericValue(ByteArray&& data) : value(std::move(data)) {}
    
    // Add type checker
    bool isByteArray() const { return std::holds_alternative<ByteArray>(value); }
    
    // Add accessor
    const ByteArray& asByteArray() const {
        if (!isByteArray()) throw std::runtime_error("Not a byte array");
        return std::get<ByteArray>(value);
    }
    
    ByteArray& asByteArray() {
        if (!isByteArray()) throw std::runtime_error("Not a byte array");
        return std::get<ByteArray>(value);
    }

    // Update the memoryUsage method
    size_t memoryUsage() const {
        size_t total = sizeof(GenericValue);
        
        if (isString()) {
            total += asString().capacity();
        }
        else if (isByteArray()) {
            total += asByteArray().capacity();
        }
        // ... rest of existing memoryUsage implementation ...
    }
};

class GenericValue {
    // ... existing code ...

private:
    void serializeJson(std::ostream& os, bool pretty, int indent) const {
        if (isNull()) {
            os << "null";
        } 
        // ... other type checks ...
        else if (isByteArray()) {
            serializeByteArray(os);
        }
        // ... rest of method ...
    }

    void serializeByteArray(std::ostream& os) const {
        const auto& data = asByteArray();
        std::string base64 = Base64::encode(data.data(), data.size());
        serializeString(os, base64);
    }
};

class GenericValue {
    // ... existing code ...

private:
    void serializeJson(std::ostream& os, bool pretty, int indent) const {
        if (isNull()) {
            os << "null";
        } 
        // ... other type checks ...
        else if (isByteArray()) {
            serializeByteArray(os);
        }
        // ... rest of method ...
    }

    void serializeByteArray(std::ostream& os) const {
        const auto& data = asByteArray();
        std::string base64 = Base64::encode(data.data(), data.size());
        serializeString(os, base64);
    }
};

int main() {
    // Create a binary payload
    GenericValue::ByteArray binaryData = {0x01, 0x02, 0x03, 0x04, 0xFF, 0xFE, 0xFD};
    
    // Store in GenericValue
    GenericValue data;
    data["binary"] = binaryData;
    data["description"] = "Sample binary data";
    
    // Serialize to JSON
    std::string json = data.toJson(true);
    std::cout << "JSON with Base64:\n" << json << "\n";
    
    // Deserialize back
    GenericValue parsed = GenericValue::fromJson(json);
    const auto& decodedData = parsed["binary"].asByteArray();
    
    std::cout << "\nDecoded binary data size: " << decodedData.size() << " bytes\n";
    std::cout << "First byte: 0x" << std::hex << (int)decodedData[0] << "\n";
    
    return 0;
}

#include <vector>
#include <cstdint>
#include <utility>

class GenericValue {
    // ... existing code ...

public:
    // Serialize to CBOR binary data
    std::vector<uint8_t> toCbor() const {
        std::vector<uint8_t> output;
        serializeCbor(output);
        return output;
    }

    // Deserialize from CBOR
    static GenericValue fromCbor(const uint8_t* data, size_t size);
    static GenericValue fromCbor(const std::vector<uint8_t>& data) {
        return fromCbor(data.data(), data.size());
    }

private:
    void serializeCbor(std::vector<uint8_t>& output) const {
        if (isNull()) {
            // CBOR null (0xf6)
            output.push_back(0xf6);
        } else if (isBool()) {
            // CBOR boolean (0xf4 for false, 0xf5 for true)
            output.push_back(asBool() ? 0xf5 : 0xf4);
        } else if (isInt()) {
            serializeCborInt(output, asInt());
        } else if (isFloat()) {
            // CBOR double-precision float (0xfb)
            output.push_back(0xfb);
            double d = asFloat();
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&d);
            // Network byte order (big-endian)
            for (int i = sizeof(double)-1; i >= 0; i--) {
                output.push_back(bytes[i]);
            }
        } else if (isString()) {
            serializeCborString(output, asString());
        } else if (isByteArray()) {
            serializeCborByteArray(output, asByteArray());
        } else if (isArray()) {
            serializeCborArray(output, asArray());
        } else if (isObject()) {
            serializeCborObject(output, asObject());
        }
    }

    void serializeCborInt(std::vector<uint8_t>& output, int64_t value) const {
        if (value >= 0) {
            if (value < 24) {
                output.push_back(static_cast<uint8_t>(value));
            } else if (value <= 0xff) {
                output.push_back(0x18);
                output.push_back(static_cast<uint8_t>(value));
            } else if (value <= 0xffff) {
                output.push_back(0x19);
                pushBigEndian(output, static_cast<uint16_t>(value));
            } else if (value <= 0xffffffff) {
                output.push_back(0x1a);
                pushBigEndian(output, static_cast<uint32_t>(value));
            } else {
                output.push_back(0x1b);
                pushBigEndian(output, value);
            }
        } else {
            const uint64_t posValue = -1 - value;
            if (posValue < 24) {
                output.push_back(static_cast<uint8_t>(0x20 + posValue));
            } else if (posValue <= 0xff) {
                output.push_back(0x38);
                output.push_back(static_cast<uint8_t>(posValue));
            } else if (posValue <= 0xffff) {
                output.push_back(0x39);
                pushBigEndian(output, static_cast<uint16_t>(posValue));
            } else if (posValue <= 0xffffffff) {
                output.push_back(0x3a);
                pushBigEndian(output, static_cast<uint32_t>(posValue));
            } else {
                output.push_back(0x3b);
                pushBigEndian(output, posValue);
            }
        }
    }

    void serializeCborString(std::vector<uint8_t>& output, const std::string& str) const {
        // Major type 3 (text string)
        serializeCborLength(output, 0x60, str.size());
        output.insert(output.end(), str.begin(), str.end());
    }

    void serializeCborByteArray(std::vector<uint8_t>& output, const ByteArray& data) const {
        // Major type 2 (byte string)
        serializeCborLength(output, 0x40, data.size());
        output.insert(output.end(), data.begin(), data.end());
    }

    void serializeCborArray(std::vector<uint8_t>& output, const ArrayType& array) const {
        // Major type 4 (array)
        serializeCborLength(output, 0x80, array.size());
        for (const auto& item : array) {
            item.serializeCbor(output);
        }
    }

    void serializeCborObject(std::vector<uint8_t>& output, const ObjectType& object) const {
        // Major type 5 (map)
        serializeCborLength(output, 0xa0, object.size());
        for (const auto& [key, value] : object) {
            serializeCborString(output, key);
            value.serializeCbor(output);
        }
    }

    void serializeCborLength(std::vector<uint8_t>& output, uint8_t majorType, size_t length) const {
        if (length < 24) {
            output.push_back(majorType + static_cast<uint8_t>(length));
        } else if (length <= 0xff) {
            output.push_back(majorType + 24);
            output.push_back(static_cast<uint8_t>(length));
        } else if (length <= 0xffff) {
            output.push_back(majorType + 25);
            pushBigEndian(output, static_cast<uint16_t>(length));
        } else if (length <= 0xffffffff) {
            output.push_back(majorType + 26);
            pushBigEndian(output, static_cast<uint32_t>(length));
        } else {
            output.push_back(majorType + 27);
            pushBigEndian(output, static_cast<uint64_t>(length));
        }
    }

    template<typename T>
    void pushBigEndian(std::vector<uint8_t>& output, T value) const {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        for (int i = sizeof(T)-1; i >= 0; i--) {
            output.push_back(bytes[i]);
        }
    }
};

#include "generic_value.h"
#include <stdexcept>

class CborParser {
public:
    static GenericValue parse(const uint8_t* data, size_t size) {
        CborParser parser(data, size);
        return parser.parseValue();
    }

private:
    CborParser(const uint8_t* data, size_t size) 
        : data(data), size(size), pos(0) {}

    GenericValue parseValue() {
        if (pos >= size) throw std::runtime_error("Unexpected end of CBOR data");
        
        uint8_t initialByte = data[pos++];
        uint8_t majorType = initialByte >> 5;
        uint8_t minorType = initialByte & 0x1f;
        uint64_t length = parseLength(minorType);

        switch (majorType) {
            case 0: return parseUnsignedInt(length); // unsigned integer
            case 1: return parseNegativeInt(length); // negative integer
            case 2: return parseByteString(length); // byte string
            case 3: return parseTextString(length); // text string
            case 4: return parseArray(length); // array
            case 5: return parseMap(length); // map
            case 6: return parseTaggedValue(length); // tag
            case 7: return parseSimpleValue(minorType, length); // simple value/float
            default: throw std::runtime_error("Invalid CBOR major type");
        }
    }

    uint64_t parseLength(uint8_t minorType) {
        if (minorType < 24) {
            return minorType;
        } else if (minorType == 24) {
            return readUint8();
        } else if (minorType == 25) {
            return readUint16();
        } else if (minorType == 26) {
            return readUint32();
        } else if (minorType == 27) {
            return readUint64();
        } else if (minorType == 31) {
            // Indefinite length (not fully supported here)
            throw std::runtime_error("Indefinite length items not supported");
        } else {
            throw std::runtime_error("Reserved minor type");
        }
    }

    GenericValue parseUnsignedInt(uint64_t value) {
        return GenericValue(static_cast<int64_t>(value));
    }

    GenericValue parseNegativeInt(uint64_t value) {
        return GenericValue(-1 - static_cast<int64_t>(value));
    }

    GenericValue parseByteString(uint64_t length) {
        checkAvailable(length);
        GenericValue::ByteArray byteArray(data + pos, data + pos + length);
        pos += length;
        return GenericValue(std::move(byteArray));
    }

    GenericValue parseTextString(uint64_t length) {
        checkAvailable(length);
        std::string str(reinterpret_cast<const char*>(data + pos), length);
        pos += length;
        return GenericValue(std::move(str));
    }

    GenericValue parseArray(uint64_t length) {
        GenericValue::ArrayType array;
        array.reserve(length);
        for (uint64_t i = 0; i < length; i++) {
            array.push_back(parseValue());
        }
        return GenericValue(std::move(array));
    }

    GenericValue parseMap(uint64_t length) {
        GenericValue::ObjectType object;
        for (uint64_t i = 0; i < length; i++) {
            // Key must be a string in our implementation
            GenericValue key = parseValue();
            if (!key.isString()) {
                throw std::runtime_error("CBOR map key must be a string");
            }
            object.emplace(key.asString(), parseValue());
        }
        return GenericValue(std::move(object));
    }

    GenericValue parseTaggedValue(uint64_t tag) {
        // For simplicity, we just skip tags in this implementation
        // A more complete implementation would handle specific tags
        return parseValue();
    }

    GenericValue parseSimpleValue(uint8_t minorType, uint64_t length) {
        if (minorType == 20) return GenericValue(false);
        if (minorType == 21) return GenericValue(true);
        if (minorType == 22) return GenericValue(nullptr);
        if (minorType == 23) return GenericValue(nullptr); // undefined (treated as null)
        
        if (minorType == 25) {
            // Half-precision float (not directly supported)
            throw std::runtime_error("Half-precision float not supported");
        } else if (minorType == 26) {
            // Single-precision float
            float f = readFloat();
            return GenericValue(static_cast<double>(f));
        } else if (minorType == 27) {
            // Double-precision float
            double d = readDouble();
            return GenericValue(d);
        }
        
        throw std::runtime_error("Unsupported simple value");
    }

    // Helper functions
    void checkAvailable(size_t needed) const {
        if (pos + needed > size) {
            throw std::runtime_error("Unexpected end of CBOR data");
        }
    }

    uint8_t readUint8() {
        checkAvailable(1);
        return data[pos++];
    }

    uint16_t readUint16() {
        checkAvailable(2);
        uint16_t value = (static_cast<uint16_t>(data[pos]) << 8) | data[pos+1];
        pos += 2;
        return value;
    }

    uint32_t readUint32() {
        checkAvailable(4);
        uint32_t value = (static_cast<uint32_t>(data[pos]) << 24) |
                        (static_cast<uint32_t>(data[pos+1]) << 16) |
                        (static_cast<uint32_t>(data[pos+2]) << 8) |
                        data[pos+3];
        pos += 4;
        return value;
    }

    uint64_t readUint64() {
        checkAvailable(8);
        uint64_t value = (static_cast<uint64_t>(data[pos]) << 56) |
                        (static_cast<uint64_t>(data[pos+1]) << 48) |
                        (static_cast<uint64_t>(data[pos+2]) << 40) |
                        (static_cast<uint64_t>(data[pos+3]) << 32) |
                        (static_cast<uint64_t>(data[pos+4]) << 24) |
                        (static_cast<uint64_t>(data[pos+5]) << 16) |
                        (static_cast<uint64_t>(data[pos+6]) << 8) |
                        data[pos+7];
        pos += 8;
        return value;
    }

    float readFloat() {
        uint32_t bits = readUint32();
        return *reinterpret_cast<float*>(&bits);
    }

    double readDouble() {
        uint64_t bits = readUint64();
        return *reinterpret_cast<double*>(&bits);
    }

    const uint8_t* data;
    size_t size;
    size_t pos;
};

// Implement the static fromCbor method
GenericValue GenericValue::fromCbor(const uint8_t* data, size_t size) {
    return CborParser::parse(data, size);
}

int main() {
    // Create a complex object with various types
    GenericValue data;
    data["name"] = "Test Data";
    data["version"] = 2;
    data["active"] = true;
    
    // Add a byte array
    GenericValue::ByteArray binaryData = {0x01, 0x02, 0x03, 0x04, 0xFF};
    data["binary"] = binaryData;
    
    // Add an array
    GenericValue::ArrayType array = {1, 2.5, "three", false};
    data["array"] = array;
    
    // Serialize to CBOR
    std::vector<uint8_t> cborData = data.toCbor();
    std::cout << "CBOR size: " << cborData.size() << " bytes\n";
    
    // Deserialize back
    GenericValue parsed = GenericValue::fromCbor(cborData);
    
    // Verify the binary data
    const auto& decodedBinary = parsed["binary"].asByteArray();
    std::cout << "Decoded binary size: " << decodedBinary.size() << " bytes\n";
    std::cout << "First byte: 0x" << std::hex << (int)decodedBinary[0] << "\n";
    
    // Verify other data
    std::cout << "Name: " << parsed["name"].asString() << "\n";
    std::cout << "Array size: " << parsed["array"].size() << "\n";
    
    return 0;
}