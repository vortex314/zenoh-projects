#ifndef SERDES_H
#define SERDES_H

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <nanocbor/nanocbor.h>
// #include <ArduinoJson.h>

typedef std::vector<uint8_t> Bytes;
typedef bool Void;
#define TEST_RC(TYPE, VAL, MSG)                                                \
  {                                                                            \
    auto rc = (VAL);                                                           \
    if (rc < 0) {                                                              \
      printf(MSG);                                                             \
      return Result<TYPE>::Err(rc, MSG);                                       \
    }                                                                          \
  }
#define TEST_R(TYPE, VAL, MSG)                                                 \
  {                                                                            \
    auto rc = (VAL);                                                           \
    if (rc.is_err()) {                                                         \
      printf(MSG);                                                             \
      return Result<TYPE>::Err(rc.rc(), MSG);                                  \
    }                                                                          \
  }
//#define Void (void())

template <typename T> class Result {
private:
  std::optional<uint32_t> code;
  std::optional<std::string> desc;
  std::optional<T> _value;

public:
  Result(const Result<T> &other) {
    code = other.code;
    desc = other.desc;
    _value = other._value;
  }

  Result() {}

  static Result<T> Ok(T value) {
    Result<T> res;
    res._value = value;
    return res;
  }
  static Result<T> Err(uint32_t code, std::string desc) {
    Result<T> res;
    res.code = code;
    res.desc = desc;
    return res;
  }

  T value() { return _value.value(); }
  bool is_ok() { return _value.has_value(); }
  bool is_err() { return !_value.has_value(); }
  uint32_t rc() {
    if (code)
      return code.value();
    else
      return 0;
  }
  std::string msg() {
    if (desc) {
      return std::string("NO error");
    } else {
      return desc.value();
    }
  }
  template <typename U> Result<U> map(std::function<Result<U>(T)> f) {
    if (_value.has_value) {
      return Result<U>::Ok(f(_value.value()));
    } else {
      return Result<U>::Err(code.value(), desc.value());
    }
  }
};

class Serializer {
public:
  virtual Result<Void> serialize(int i) = 0;
  virtual Result<Void> serialize(uint32_t i) = 0;
  virtual Result<Void> serialize(std::string s) = 0;
  virtual Result<Void> serialize(Bytes b) = 0;
  virtual Result<Void> serialize(float f) = 0;
  template <typename V>
  Result<Void> serialize(int idx, std::optional<V> value) {
    if (value.has_value()) {
      serialize(idx);
      serialize(value.value());
    }
    return Result<Void>::Ok(true);
  }
  virtual Result<Void> map_begin() = 0;
  virtual Result<Void> map_end() = 0;
  virtual Result<Void> array_begin() = 0;
  virtual Result<Void> array_end() = 0;
  virtual Result<Bytes> get_bytes() = 0;
};

class Deserializer {
public:
  virtual void buffer(Bytes b) = 0;
  virtual Result<int> deserialize_int() = 0;
  virtual Result<uint32_t> deserialize_uint32() = 0;
  virtual Result<std::string> deserialize_string() = 0;
  virtual Result<Bytes> deserialize_bytes() = 0;
  virtual Result<float> deserialize_float() = 0;
  virtual Result<Void> map_begin() = 0;
  virtual Result<Void> map_end() = 0;
  virtual Result<Void> array_begin() = 0;
  virtual Result<Void> array_end() = 0;
};

template <typename T> class Serializable {
public:
  virtual Result<Void> serialize(Serializer &ser) = 0;
  virtual Result<T> deserialize(Deserializer &des) = 0;
};

class JsonSerializer : public Serializer {
private:
  std::string _json;
  typedef enum {
    INIT,
    MAP_0,
    MAP_N,
    ARRAY_0,
    ARRAY_N,
  } State;
  State _state;
};

class CborSerializer : public Serializer {
private:
  nanocbor_encoder_t _enc;
  uint8_t *_bytes;
  typedef enum {
    INIT,
    MAP_0,
    MAP_N,
    ARRAY_0,
    ARRAY_N,
  } State;
  State _state;

public:
  CborSerializer(size_t size) {
    _bytes = new uint8_t[size];
    nanocbor_encoder_init(&_enc, _bytes, size);
  }
  ~CborSerializer() { delete _bytes; }
  Result<Void> serialize(int i) {
    TEST_RC(Void, nanocbor_fmt_int(&_enc, i), "Failed to encode int");
    return Result<Void>::Ok(true);
  }
  Result<Void> serialize(uint32_t i) {
    TEST_RC(Void, nanocbor_fmt_uint(&_enc, i), "Failed to encode uint32_t");
    return Result<Void>::Ok(true);
  }
  Result<Void> serialize(std::string s) {
    TEST_RC(Void, nanocbor_put_tstr(&_enc, s.c_str()),
            "Failed to encode string");
    return Result<Void>::Ok(true);
  }
  Result<Void> serialize(Bytes b) {
    TEST_RC(Void, nanocbor_put_bstr(&_enc, b.data(), b.size()),
            "Failed to encode bytes");
    return Result<Void>::Ok(true);
  }
  Result<Void> serialize(float f) {
    TEST_RC(Void, nanocbor_fmt_float(&_enc, f), "Failed to encode float");
    return Result<Void>::Ok(true);
  }
  Result<Void> map_begin() {
    TEST_RC(Void, nanocbor_fmt_map_indefinite(&_enc), "Failed to encode map");
    return Result<Void>::Ok(true);
  }
  Result<Void> map_end() {
    TEST_RC(Void, nanocbor_fmt_end_indefinite(&_enc), "Failed to encode map");
    return Result<Void>::Ok(true);
  }
  Result<Void> array_begin() {
    TEST_RC(Void, nanocbor_fmt_array_indefinite(&_enc),
            "Failed to encode array");
    return Result<Void>::Ok(true);
  }
  Result<Void> array_end() {
    TEST_RC(Void, nanocbor_fmt_end_indefinite(&_enc), "Failed to encode array");
    return Result<Void>::Ok(true);
  }
  Result<Bytes> get_bytes() {
    size_t length = nanocbor_encoded_len(&_enc);
    std::vector<uint8_t> bytes(_bytes, _bytes + length);
    return Result<Bytes>::Ok(bytes);
  }
};
#endif