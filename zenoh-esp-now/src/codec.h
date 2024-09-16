
#ifndef _CODEC_H_
#define _CODEC_H_
#include <Log.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>

 #define XSTR(s) STR(s)
 #define STR(s) #s

typedef bool Void;

template <typename T>
class Option {
   private:
    bool _none = false;
    T _value;

   public:
    static Option<T> None() {
        Option<T> option(true);
        option._none = true;
        return option;
    }
    static Option<T> Some(T value) {
        Option<T> option(false);
        option._value = value;
        return option;
    }
    Option(bool none) : _none(none) {}
    bool is_some() { return !_none; }
    bool is_none() { return _none; }
    T unwrap() { return _value; }
};

template <typename T>
class Result {
    T _value;
    int _err;
    const char* _msg;

   public:
    Result() : _value(), _err(0) {}
    bool is_ok() { return _err == 0; }
    bool is_err() { return _err != 0; }
    const char* get_err_msg() { return (_err == 0) ? "No error" : _msg; }
    Result& on_error(std::function<void(const char*)> ff) {
        if (_err != 0) {
            ff(_msg);
        }
        return *this;
    }
    Result& on_ok(std::function<void(T)> ff) {
        if (_err == 0) {
            ff(_value);
        } else {
            ERROR("Error: %d\n", _err);
        }
        return *this;
    }
    T unwrap() {
        if (_err != 0) {
            ERROR("Error: %d\n", _err);
        }
        return _value;
    }
    static Result<T> Ok(T value) {
        Result<T> result;
        result._value = value;
        result._err = 0;
        return result;
    }
    static Result<T> Err(int err) {
        Result<T> result;
        result._err = err;
        result._msg = strerror(err);
        return result;
    }
    static Result<T> Err(int err, const char* msg) {
        Result<T> result;
        result._err = err;
        result._msg = msg;
        return result;
    }
};

#define RET_ERR(x)    \
    if (x.is_err()) { \
        return x;     \
    }

class FrameEncoder {
   private:
    std::vector<uint8_t> _buffer;
    uint32_t _max;
    Result<Void> write_byte(uint8_t byte);

   public:
    FrameEncoder(uint32_t max);
    Result<Void> encode_array();
    Result<Void> encode_map();
    Result<Void> encode_end();
    Result<Void> encode_uint32(uint32_t input_value);
    Result<Void> encode_str(const char* str);
    Result<Void> encode_bstr(std::vector<uint8_t>& buffer);
    Result<Void> encode_float(float value);
    Result<Void> encode_int32(int32_t value);
    Result<Void> encode_null();
    Result<Void> add_crc();
    Result<Void> add_cobs();
    Result<Void> read_buffer(uint8_t* buffer, size_t len);
    Result<Void> read_buffer(std::vector<unsigned char>& buffer);
    Result<Void> clear();
    Result<Void> rewind();
    Result<std::string> to_string();
};

enum CborType {
    CBOR_UINT32 = 0,
    CBOR_STR = 1,
    CBOR_FLOAT = 2,
    CBOR_DOUBLE = 3,
    CBOR_INT32 = 4,
    CBOR_BSTR = 5,
};

class FrameDecoder {
   private:
    std::vector<uint8_t> _buffer;
    uint32_t _index;
    uint32_t _max;
    Result<uint8_t> read_next();
    Result<uint8_t> peek_next();
    Result<CborType> peek_type();

   public:
    FrameDecoder(uint32_t max);
    Result<Void> decode_array();
    Result<Void> decode_map();
    Result<Void> decode_end();
    Result<uint32_t> decode_uint32();
    Result<std::string> decode_str();
    Result<float> decode_float();
    Result<int32_t> decode_int32();
    Result<std::vector<uint8_t>> decode_bstr();
    Result<bool> check_crc();
    Result<Void> decode_cobs();
    Result<bool> add_byte(uint8_t byte);
    Result<bool> fill_buffer(std::vector<uint8_t> buffer);
    Result<bool> fill_buffer(uint8_t* buffer, uint32_t size);
    Result<Void> read_buffer(uint8_t* buffer, size_t len);
    Result<Void> read_buffer(std::vector<unsigned char>& buffer);
    Result<Void> clear();
    Result<Void> rewind();
    Result<std::string> to_string();
};


// FNV-1a hash function for 32-bit hash value
constexpr uint32_t fnv1a_32_1(const char* str, uint32_t hash = 2166136261U) {
    return *str == '\0' ? hash : fnv1a_32_1(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619U);
}

// Helper to compute the hash at compile time for a string literal
template<std::size_t N>
constexpr uint32_t FNV(const char(&str)[N]) {
    return fnv1a_32_1(str);
}

typedef enum MsgType {
    Alive = 0,
    Pub0Req = 1,
    Pub1Req,
    PingReq,
    NameReq,
    DescReq,
    SetReq,
    GetReq,
} MsgType;

class MsgHeader {
    public :
    Option<uint32_t> dst;
    Option<uint32_t> src;
    uint32_t msg_type;
    Option<uint32_t> msg_id;

   public:
    Result<Void> encode(FrameEncoder& encoder) {
        if (dst.is_some()) {
            RET_ERR(encoder.encode_uint32(dst.unwrap()));
        } else {
            RET_ERR(encoder.encode_null());
        }
        if (src.is_some()) {
            RET_ERR(encoder.encode_uint32(src.unwrap()));
        } else {
            RET_ERR(encoder.encode_null());
        }
        RET_ERR(encoder.encode_uint32(msg_type));
        if (msg_id.is_some()) {
            RET_ERR(encoder.encode_uint32(msg_id.unwrap()));
        } else {
            RET_ERR(encoder.encode_null());
        }
        return Result<Void>::Ok(Void());
    }
};

#endif
