#ifndef CBOR_H
#define CBOR_H

#include <nanocbor/nanocbor.h>
#include <optional>
#include <serdes.h>
#include <string.h>
#include <string>
#include <vector>

// #include <ArduinoJson.h>

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

  Res serialize(uint64_t i) {
    RET_ERRI(nanocbor_fmt_uint(&_enc, i), "Failed to encode int64_t");
    return Res::Ok();
  }

  Res serialize(std::string &s) {
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

  Res serialize(Serializable &value) { return value.serialize(*this); }
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

  Res deserialize(uint8_t &i) {
    RET_ERRI(nanocbor_get_uint8(get_des(), &i), "Failed to decode uint8_t");
    return Res::Ok();
  }

  Res deserialize(int8_t &i) {
    RET_ERRI(nanocbor_get_int8(get_des(), &i), "Failed to decode int8_t");
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

  Res deserialize(uint64_t &val) {
    RET_ERRI(nanocbor_get_uint64(get_des(), &val), "Failed to decode uint64_t");
    return Res::Ok();
  }

  Res deserialize(int64_t &val) {
    RET_ERRI(nanocbor_get_int64(get_des(), &val), "Failed to decode int64_t");
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