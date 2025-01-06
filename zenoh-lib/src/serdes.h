#ifndef SERDES_H
#define SERDES_H

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <log.h>

#include <nanocbor/nanocbor.h>
// #include <ArduinoJson.h>

typedef std::vector<uint8_t> Bytes;
typedef bool Void;
#define TEST_RC(TYPE, VAL, MSG)                                                \
  {                                                                            \
    auto rc = (VAL);                                                           \
    if (rc < 0) {                                                              \
      INFO(MSG);                                                             \
      INFO("=%d %s:%d", rc, __FILE__, __LINE__);                           \
      return Result<TYPE>::Err(rc, MSG);                                       \
    }                                                                          \
  }
#define TEST_R(TYPE, VAL, MSG)                                                 \
  {                                                                            \
    auto r = (VAL);                                                            \
    if (r.is_err()) {                                                          \
      INFO(MSG);                                                             \
      INFO("=%d %s:%d", r.rc(), __FILE__, __LINE__);                       \
      return Result<TYPE>::Err(r.rc(), MSG);                                   \
    }                                                                          \
  }
#define RET_ERR(VAL, MSG)                                                      \
  {                                                                            \
    auto r = (VAL);                                                            \
    if (r.is_err()) {                                                          \
      return Res::Err(r.rc(), MSG);                                            \
    }                                                                          \
  }

#define RET_ERRI(VAL, MSG)                                                     \
  {                                                                            \
    int rc = (VAL);                                                            \
    if (rc < 0) {                                                              \
      INFO(MSG);                                                             \
      INFO("=%d %s:%d", rc, __FILE__, __LINE__);                           \
      return Res::Err(rc, MSG);                                                \
    }                                                                          \
  }
//#define Void (void())

template <typename T=void > class Result {
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

class Res {
private:
  uint32_t code;
  std::optional<std::string> desc;

public:
  Res(const Res &other) {
    code = other.code;
    desc = other.desc;
  }

  Res() { code = 0; }

  static Res Ok() {
    Res res;
    res.code = 0;
    return res;
  }
  static Res Err(uint32_t code, std::string desc) {
    Res res;
    res.code = code;
    res.desc = desc;
    return res;
  }

  bool is_ok() { return code == 0; }
  bool is_err() { return code != 0; }
  uint32_t rc() { return code; }
  std::string msg() {
    if (code) {
      return std::string("NO error");
    } else {
      return desc.value();
    }
  }
};

class Serializer;
class Deserializer;

class Serializable {
public:
  virtual Res serialize(Serializer &ser) = 0;
  virtual Res deserialize(Deserializer &des) = 0;
};

class Serializer {
public:
  virtual Res serialize(uint8_t i) = 0;
  virtual Res serialize(int8_t i) = 0;
  virtual Res serialize(int32_t i) = 0;
  virtual Res serialize(uint32_t i) = 0;
  virtual Res serialize(int64_t i) = 0;
  virtual Res serialize(uint64_t i) = 0;
  virtual Res serialize(std::string &s) = 0;
  virtual Res serialize(Bytes b) = 0;
  virtual Res serialize(float f) = 0;
  virtual Res serialize_null() = 0;
  template <typename V> Res serialize(std::optional<V> value) {
    if (value.has_value()) {
      serialize(value.value());
    } else {
      serialize_null();
    }
    return Res::Ok();
  }
  template <typename V> Res serialize(uint32_t idx, std::optional<V> value) {
    if (value.has_value()) {
      serialize(idx);
      serialize(value.value());
    }
    return Res::Ok();
  }
  Res serialize(Serializable &value) { return value.serialize(*this); }

  virtual Res map_begin() = 0;
  virtual Res map_end() = 0;
  virtual Res array_begin() = 0;
  virtual Res array_end() = 0;
  virtual Res get_bytes(Bytes &bytes) = 0;
  virtual Res reset() = 0;
};

typedef enum {
  SER_UINT,
  SER_SINT,
  SER_FLOAT,
  SER_STR,
  SER_BYTES,
  SER_ARRAY,
  SER_MAP,
  SER_END,
} SerialType;

class Deserializer {
public:
  virtual Res fill_buffer(Bytes &b) = 0;
  virtual Res deserialize(std::string &s) = 0;
  virtual Res deserialize(uint8_t &i) = 0;
  virtual Res deserialize(int8_t &i) = 0;
  virtual Res deserialize(uint32_t &i) = 0;
  virtual Res deserialize(int32_t &i) = 0;
  virtual Res deserialize(uint64_t &i) = 0;
  virtual Res deserialize(int64_t &i) = 0;
  virtual Res deserialize(float &f) = 0;
  virtual Res deserialize(Bytes &bytes) = 0;
  virtual Res deserialize(bool &b) = 0;
  virtual Res map_begin() = 0;
  virtual Res map_end() = 0;
  virtual Res array_begin() = 0;
  virtual Res array_end() = 0;
  virtual Res peek_type(SerialType &type) = 0;
  virtual Res deserialize_null() = 0;

  template <typename U> Res deserialize(std::optional<U> &opt) {
    U u;
    RET_ERR(deserialize(u), "Failed to decode option");
    opt = u;
    return Res::Ok();
  }

  template <typename U> Result<U> deserialize_type(U &u) {
    if constexpr (std::is_same<U, int>::value) {
      return deserialize_int(u);
    } else if constexpr (std::is_same<U, uint32_t>::value) {
      return deserialize_uint32(u);
    } else if constexpr (std::is_same<U, std::string>::value) {
      return deserialize_string(u);
    } else if constexpr (std::is_same<U, Bytes>::value) {
      return deserialize_bytes(u);
    } else if constexpr (std::is_same<U, float>::value) {
      return deserialize_float(u);
    } else if constexpr (std::is_same<U, bool>::value) {
      return deserialize_bool(u);
    } else {
      return U::deserialize(*this);
    }
    return U::deserialize(*this);
  }

  Res iterate_map(std::function<Res(Deserializer &, uint32_t)> func) {
    RET_ERR(map_begin(), "Failed to decode map");
    SerialType type;
    while (true) {
      RET_ERR(peek_type(type), "Failed to peek type");
      if (type == SerialType::SER_END)
        break;
      if (type != SerialType::SER_UINT)
        return Res::Err(0, "Expected uint");
      uint32_t key;
      RET_ERR(deserialize(key), "Failed to decode key in map");
      RET_ERR(func(*this, key), "Failed to process map entry");
    }
    RET_ERR(map_end(), "Failed to decode map end ");
    return Res::Ok();
  }
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

#endif