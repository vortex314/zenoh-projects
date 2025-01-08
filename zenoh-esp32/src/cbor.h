#ifndef CBOR_H
#define CBOR_H

#include <nanocbor/nanocbor.h>
#include <optional>
#include <serdes.h>
#include <string.h>
#include <string>
#include <vector>
#include <util.h>

class CborSerializer : public Serializer
{
private:
  nanocbor_encoder_t _enc;
  Bytes &_bytes;
  typedef enum
  {
    INIT,
    MAP_0,
    MAP_N,
    ARRAY_0,
    ARRAY_N,
  } State;
  State _state;
  static void append_func(nanocbor_encoder_t *enc, void *ctx, const uint8_t *data, size_t len);
  static bool fits_func(nanocbor_encoder_t *enc, void *ctx, size_t len);

public:
  CborSerializer(Bytes &bytes);
  ~CborSerializer();
  Res reset();
  Res serialize(uint8_t v);
  Res serialize(int8_t v);
  Res serialize(int i);
  Res serialize(bool b);
  Res serialize(int32_t i);
  Res serialize(uint32_t i);
  Res serialize(int64_t i);
  Res serialize(uint64_t i);
  Res serialize(std::string &s);
  Res serialize(Bytes b);
  Res serialize(float f);

  Res map_begin();
  Res map_end();
  Res array_begin();
  Res array_end();
  Res serialize_null();

  Res serialize(Serializable &value);
};

#define MAX_TSTR_LENGTH 256
#define MAX_BSTR_LENGTH 256

class CborDeserializer : public Deserializer
{

private:
  /* data */
  uint8_t *_bytes;
  size_t _size;
  size_t _capacity;
  nanocbor_value_t _des;
  nanocbor_value_t _map;
  nanocbor_value_t _array;
  typedef enum
  {
    INIT,
    MAP,
    ARRAY,
  } State;
  State _state;
  nanocbor_value_t *get_des();

public:
  CborDeserializer(size_t size);
  CborDeserializer(uint8_t *bytes, size_t size)
  {
    _bytes = bytes;
    _size = size;
    _capacity = size;
    nanocbor_decoder_init(&_des, _bytes, _size);
  };
  ~CborDeserializer();

  Res fill_buffer(Bytes &b);
  Res deserialize(uint8_t &i);
  Res deserialize(int8_t &i);

  Res deserialize(int32_t &i);
  Res deserialize(uint64_t &val);
  Res deserialize(int64_t &val);
  Res deserialize(uint32_t &val);
  Res deserialize(std::string &s);
  Res deserialize(Bytes &bytes);
  Res deserialize(float &f);
  Res deserialize(bool &b);
  Res map_begin();
  Res map_end();
  Res array_begin();
  Res array_end();
  Res peek_type(SerialType &serial_type);
  Res deserialize_null();
  template <typename U>
  Res deserialize_option(std::optional<U> &opt)
  {
    if (nanocbor_get_null(get_des()) == 0)
    {
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