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
      printf("=%d %s:%d\n", rc, __FILE__, __LINE__);                           \
      return Result<TYPE>::Err(rc, MSG);                                       \
    }                                                                          \
  }
#define TEST_R(TYPE, VAL, MSG)                                                 \
  {                                                                            \
    auto r = (VAL);                                                            \
    if (r.is_err()) {                                                          \
      printf(MSG);                                                             \
      printf("=%d %s:%d\n", r.rc(), __FILE__, __LINE__);                       \
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
      printf(MSG);                                                             \
      printf("=%d %s:%d\n", rc, __FILE__, __LINE__);                           \
      return Res::Err(rc, MSG);                                                \
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

class Serializer {
public:
  virtual Res serialize(int i) = 0;
  virtual Res serialize(uint32_t i) = 0;
  virtual Res serialize(int64_t i) = 0;
  virtual Res serialize(std::string s) = 0;
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
  template <typename V> Res serialize(int idx, std::optional<V> value) {
    if (value.has_value()) {
      serialize(idx);
      serialize(value.value());
    }
    return Res::Ok();
  }
  virtual Res map_begin() = 0;
  virtual Res map_end() = 0;
  virtual Res array_begin() = 0;
  virtual Res array_end() = 0;
  virtual Res  get_bytes(Bytes& bytes) = 0;
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
  virtual Res deserialize(uint32_t &i) = 0;
  virtual Res deserialize(int32_t &i) = 0;
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

  Res iterate_map(std::function<Res(Deserializer &, uint32_t)> func);
};

template <typename T> class Serializable {
public:
  virtual Res serialize(Serializer &ser) = 0;
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
  size_t _size;
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
    _size = size;
    nanocbor_encoder_init(&_enc, _bytes, size);
  }
  ~CborSerializer() { delete _bytes; }
  Res reset() {
    nanocbor_encoder_init(&_enc, _bytes, _size);
    return Res::Ok();
  }
  Res serialize(int i) {
    RET_ERRI(nanocbor_fmt_int(&_enc, i), "Failed to encode int");
    return Res::Ok();
  }
  Res serialize(uint32_t i) {
    RET_ERRI(nanocbor_fmt_uint(&_enc, i), "Failed to encode uint32_t");
    return Res::Ok();
  }

  Res serialize(int64_t i) {
    RET_ERRI(nanocbor_fmt_int(&_enc, i), "Failed to encode int64_t");
    return Res::Ok();
  }

  Res serialize(std::string s) {
    RET_ERRI(nanocbor_put_tstr(&_enc, s.c_str()), "Failed to encode string");
    return Res::Ok();
  }
  Res serialize(Bytes b) {
    RET_ERRI(nanocbor_put_bstr(&_enc, b.data(), b.size()),
             "Failed to encode bytes");
    return Res::Ok();
  }
  Res serialize(float f) {
    RET_ERRI(nanocbor_fmt_float(&_enc, f), "Failed to encode float");
    return Res::Ok();
  }
  Res map_begin() {
    RET_ERRI(nanocbor_fmt_map_indefinite(&_enc), "Failed to encode map");
    return Res::Ok();
  }
  Res map_end() {
    RET_ERRI(nanocbor_fmt_end_indefinite(&_enc), "Failed to encode map");
    return Res::Ok();
  }
  Res array_begin() {
    RET_ERRI(nanocbor_fmt_array_indefinite(&_enc), "Failed to encode array");
    return Res::Ok();
  }
  Res array_end() {
    RET_ERRI(nanocbor_fmt_end_indefinite(&_enc), "Failed to encode array");
    return Res::Ok();
  }
  Res serialize_null() {
    RET_ERRI(nanocbor_fmt_null(&_enc), "Failed to encode null");
    return Res::Ok();
  }
  Res get_bytes(Bytes &bytes) {
    size_t length = nanocbor_encoded_len(&_enc);
    bytes.assign(_bytes, _bytes + length);
    return Res::Ok();
  }
};

#define MAX_TSTR_LENGTH 256
#define MAX_BSTR_LENGTH 256

class CborDeserializer : public Deserializer {

private:
  /* data */
  nanocbor_value_t _des;
  nanocbor_value_t _map;
  nanocbor_value_t _array;
  uint8_t *_bytes;
  size_t _size;
  size_t _capacity;
  typedef enum {
    INIT,
    MAP,
    ARRAY,
  } State;
  State _state;
  nanocbor_value_t *get_des() {
    switch (_state) {
    case INIT:
      return &_des;
    case MAP:
      return &_map;
    case ARRAY:
      return &_array;
    }
    return &_des;
  }

public:
  CborDeserializer(size_t size) {
    _bytes = new uint8_t[size];
    _size = 0;
    _capacity = size;
    _state = INIT;
  }
  ~CborDeserializer() { delete _bytes; }

  Res fill_buffer(Bytes &b) {
    if (b.size() > _capacity) {
      return Res::Err(ENOSPC, "Buffer too small");
    }
    memcpy(_bytes, b.data(), b.size());
    _size = b.size();
    _state = INIT;
    nanocbor_decoder_init(get_des(), _bytes, _size);
    return Res::Ok();
  }
  Res deserialize(int32_t &i) {
    int64_t val;
    RET_ERRI(nanocbor_get_int64(get_des(), &val), "Failed to decode int");
    if (val > INT32_MAX || val < INT32_MIN) {
      return Res::Err(ERANGE, "Value out of range");
    }
    i = val;
    return Res::Ok();
  }

  Res deserialize(uint32_t &val) {
    RET_ERRI(nanocbor_get_uint32(get_des(), &val), "Failed to decode uint32_t");
    return Res::Ok();
  }

  Res deserialize(std::string &s) {
    const uint8_t *val;
    size_t len;
    RET_ERRI(nanocbor_get_tstr(get_des(), &val, &len),
             "Failed to decode string");
    s = std::string(val, val + len);
    return Res::Ok();
  }
  Res deserialize(Bytes &bytes) {
    const uint8_t *val;
    size_t len;
    RET_ERRI(nanocbor_get_bstr(get_des(), &val, &len),
             "Failed to decode bytes");
    bytes = Bytes(val, val + len);
    return Res::Ok();
  }
  Res deserialize(float &f) {
    float val;
    RET_ERRI(nanocbor_get_float(get_des(), &val), "Failed to decode float");
    return Res::Ok();
  }

  Res deserialize(bool &b) {
    bool val;
    RET_ERRI(nanocbor_get_bool(get_des(), &val), "Failed to decode bool");
    b = val;
    return Res::Ok();
  }

  Res map_begin() {
    RET_ERRI(nanocbor_enter_map(get_des(), &_map), "Failed to decode map");
    _state = MAP;
    return Res::Ok();
  }
  Res map_end() {
    nanocbor_leave_container(get_des(), &_map);
    _state = INIT;
    return Res::Ok();
  }
  Res array_begin() {
    RET_ERRI(nanocbor_enter_array(get_des(), &_array),
             "Failed to decode array");
    _state = ARRAY;
    return Res::Ok();
  }
  Res array_end() {
    nanocbor_leave_container(get_des(), &_array);
    _state = INIT;
    return Res::Ok();
  }
  Res peek_type(SerialType &serial_type) {
    nanocbor_value_t val;
    int type = nanocbor_get_type(get_des());
    switch (type) {
    case NANOCBOR_TYPE_UINT:
      serial_type = SerialType::SER_UINT;
      break;
    case NANOCBOR_TYPE_NINT:
      serial_type = SerialType::SER_SINT;
      break;
    case NANOCBOR_TYPE_FLOAT:
      serial_type = SerialType::SER_FLOAT;
      break;
    case NANOCBOR_TYPE_TSTR:
      serial_type = SerialType::SER_STR;
      break;
    case NANOCBOR_TYPE_BSTR:
      serial_type = SerialType::SER_BYTES;
      break;
    case NANOCBOR_TYPE_ARR:
      serial_type = SerialType::SER_ARRAY;
      break;
    case NANOCBOR_TYPE_MAP:
      serial_type = SerialType::SER_MAP;
      break;
    case NANOCBOR_ERR_END:
      serial_type = SerialType::SER_END;
      break;
    default:
      return Res::Err(0, "Unknown type");
    }
    return Res::Ok();
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

  Res deserialize_null() {
    RET_ERRI(nanocbor_get_null(get_des()), "Failed to decode null");
    return Res::Ok();
  }
  template <typename U> Res deserialize_option(std::optional<U> &opt) {
    if (nanocbor_get_null(get_des()) == 0) {
      opt = std::nullopt;
      return Res::Ok();
    }
    U u;
    RET_ERR(deserialize(u), "Failed to decode option");
    opt = u;
    return Res::Ok();
  }
};

#endif