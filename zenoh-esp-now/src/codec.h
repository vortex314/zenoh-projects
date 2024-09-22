
#ifndef _CODEC_H_
#define _CODEC_H_
#include <Log.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>

#define XSTR(s) STR(s)
#define STR(s) #s

typedef enum MsgType : uint8_t
{
    Alive = 0,
    Pub,
    Info,
} MsgType;

typedef enum ValueMode : uint8_t
{
    Read = 1,
    Write = 2,
} ValueMode;

typedef enum ValueType : uint8_t
{
    Uint32 = 0,
    Str = 1,
    Float = 2,
    Int32 = 3,
    Bstr = 4,
    Array = 5,
    Map = 6,
    Null = 7,
    Bool = 8,
} ValueType;

typedef uint32_t TopicId;
typedef uint8_t PropId;
typedef uint16_t MsgId;
typedef std::vector<uint8_t> ByteBuffer;
typedef std::vector<uint8_t> ByteString;
typedef bool Void;

void panic(const char *msg);

template <typename T>
class Option
{
private:
    bool _none = false;
    T _value;

public:
    Option() : _none(true) {}
    static Option<T> None()
    {
        Option<T> option(true);
        option._none = true;
        return option;
    }
    static Option<T> Some(T value)
    {
        Option<T> option(false);
        option._value = value;
        return option;
    }
    Option(bool none) : _none(none) {}
    bool is_some() { return !_none; }
    bool is_none() { return _none; }
    T unwrap() { return _value; }
    Option<T> &on_some(std::function<void(T)> ff)
    {
        if (!_none)
        {
            ff(_value);
        }
        return *this;
    }
    Option<T> &on_some(void (*fp)(T))
    {
        if (!_none)
        {
            fp(_value);
        }
        return *this;
    }
    Option<T> &on_none(std::function<void()> ff)
    {
        if (_none)
        {
            ff();
        }
        return *this;
    }
    Option<T>(const Option<T> &other)
    {
        _none = other._none;
        if (!_none)
            _value = other._value;
    }
};

template <typename T>
class Result
{
    bool _ok;
    union
    {
        T _value;
        struct
        {
            int _err;
            const char *_msg;
        };
    };

public:
    Result() : _ok(false), _err(0), _msg("") {};
    Result(int err, const char *msg) : _ok(false), _err(err), _msg(msg) {};
    Result(T value) : _value(value), _ok(true) {};
    ~Result() {}
    int erc() { return _err; }
    T &value() { return _value; }
    bool is_ok() { return _err == 0; }
    bool is_err() { return _err != 0; }
    const char *get_err_msg() { return (_err == 0) ? "No error" : _msg; }
    Result &on_error(std::function<void(const char *)> ff)
    {
        if (_err != 0)
        {
            ff(_msg);
        }
        return *this;
    }
    Result &on_ok(std::function<void(T)> ff)
    {
        if (_err == 0)
        {
            ff(_value);
        }
        else
        {
            ERROR("Error: %d\n", _err);
        }
        return *this;
    }
    Result(const Result<T> &other)
    {
        _ok = other._ok;
        if (_ok)
            _value = other._value;
        else
        {
            _err = other._err;
            _msg = other._msg;
        }
    }
    template <typename U>
    Result<U> map_err()
    {
        if (_err != 0)
            return Result<U>::Err(_err, _msg);
        else
            return Result<U>::Ok(U());
    }
    T unwrap()
    {
        if (!_ok)
            panic("Error: unwrap()");
        return _value;
    }
    static Result<T> Ok(T value) { return Result<T>(value); }
    static Result<T> Err(int err) { return Result<T>(err, ""); }
    static Result<T> Err(int err, const char *msg) { return Result<T>(err, msg); }
};

#define CHECK_MAP(U, _r) ({ auto __r=_r;if (__r.is_err()) return Result<U>::Err(__r.erc(), __r.get_err_msg()) ; __r.value() ; })
#define CHECK(_r) ({ auto __r=_r;if (__r.is_err()) return __r ; __r.value() ; })
#define RET_ERR_MSG(_r, _msg) ({ auto& __r=_r;if (__r.is_err()) return Result<T>::Err(__r.erc(), _msg) ; __r.value() ; })
#define RET_ERR(_r)       \
    {                     \
        auto __r = _r;    \
        if (__r.is_err()) \
            return __r;   \
    }

class FrameEncoder
{
private:
    std::vector<uint8_t> _buffer;
    uint32_t _max;
    Result<Void> write_byte(uint8_t byte);

public:
    FrameEncoder(uint32_t max = 256);
    Result<Void> encode_array();
    Result<Void> encode_map();
    Result<Void> encode_end();
    Result<Void> encode(uint16_t value);
    Result<Void> encode(uint32_t input_value);
    Result<Void> encode(const char *str);
    Result<Void> encode(std::string str);
    Result<Void> encode(std::vector<uint8_t> &buffer);
    Result<Void> encode(float value);
    Result<Void> encode(int32_t value);
    template <typename T>
    Result<Void> encode(Option<T> value);
    Result<Void> encode_null();
    Result<Void> add_crc();
    Result<Void> add_cobs();
    Result<Void> read_buffer(uint8_t *buffer, size_t len);
    Result<Void> read_buffer(std::vector<unsigned char> &buffer);
    std::vector<uint8_t> get_buffer();
    Result<Void> clear();
    Result<Void> rewind();
    Result<std::string> to_string();
};

typedef enum
{
    CBOR_UINT32 = 0,
    CBOR_STR = 1,
    CBOR_FLOAT = 2,
    CBOR_DOUBLE = 3,
    CBOR_INT32 = 4,
    CBOR_BSTR = 5,
    CBOR_ARRAY = 6,
    CBOR_MAP = 7,
    CBOR_NULL = 8,
    CBOR_BOOL = 9,
    CBOR_END = 10,
} CborType;

class FrameDecoder
{
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
    Result<Void> decode(uint8_t &value);
    Result<Void> decode(uint16_t &value);
    Result<Void> decode(uint32_t &value);
    Result<Void> decode(int32_t &value);
    Result<Void> decode(std::string &value);
    Result<Void> decode(float &value);
    Result<Void> decode(ByteString &value); // ByteArray is std::vector<uint8_t>
    Result<Void> check_crc();
    Result<Void> decode_cobs();
    Result<Void> add_byte(uint8_t byte);
    Result<Void> fill_buffer(std::vector<uint8_t> &buffer);
    Result<Void> fill_buffer(uint8_t *buffer, uint32_t size);
    Result<Void> read_buffer(uint8_t *buffer, size_t len);
    Result<Void> read_buffer(std::vector<unsigned char> &buffer);
    Result<std::vector<uint8_t>> get_buffer();
    Result<Void> clear();
    Result<Void> rewind();
    Result<std::string> to_string();
    Result<Void> decode_opt(Option<unsigned int> &value);

    template <typename T>
    Result<Void> decode_opt(Option<T> &value);
    template <typename T>
    Result<Void> decode(T &value);
};

// FNV-1a hash function for 32-bit hash value
constexpr uint32_t fnv1a_32_1(const char *str, uint32_t hash = 2166136261U)
{
    return *str == '\0' ? hash : fnv1a_32_1(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619U);
}

// Helper to compute the hash at compile time for a string literal
template <std::size_t N>
constexpr uint32_t FNV(const char (&str)[N])
{
    return fnv1a_32_1(str);
}

class MsgHeader
{
public:
    Option<TopicId> dst;
    Option<TopicId> src;
    MsgType msg_type;
    Option<MsgId> msg_id;

public:
    MsgHeader() {}
    Result<Void> encode(FrameEncoder &encoder);
    static Result<MsgHeader> decode(FrameDecoder &decoder);
};

class DescMsg
{
public:
    Option<PropId> prop_id;
    std::string name;
    Option<std::string> desc;
    Option<ValueType> value_type;
    Option<ValueMode> value_mode;

public:
    DescMsg() {}
    Result<Void> encode(FrameEncoder &encoder);
    static Result<DescMsg> decode(FrameDecoder &decoder);
};

#endif
