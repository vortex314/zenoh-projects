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
    MAP,

    ARRAY,
    ARRAY_FIXED,
  } State;
  State _state;
  static void append_func(nanocbor_encoder_t *enc, void *ctx, const uint8_t *data, size_t len);
  static bool fits_func(nanocbor_encoder_t *enc, void *ctx, size_t len);

public:
  CborSerializer(Bytes &bytes);
  ~CborSerializer();
  Res reset();
  Res serialize(const uint8_t v);
  Res serialize(const int8_t v);
  Res serialize(const int i);
  Res serialize(const bool b);
  Res serialize(const int32_t i);
  Res serialize(const uint32_t i);
  Res serialize(const int64_t i);
  Res serialize(const uint64_t i);
  Res serialize(const std::string &s);
  Res serialize(const char *s);
  Res serialize(const Bytes b);
  Res serialize(const float f);

  Res map_begin();
  Res map_end();
  Res array_begin();
  Res array_begin(size_t count);
  Res array_end();
  Res serialize_null();

  Res serialize(const Serializable &value);
};

#define MAX_TSTR_LENGTH 256
#define MAX_BSTR_LENGTH 256

class CborDeserializer : public Deserializer
{

private:
  /* data */
  const uint8_t *_bytes;
  size_t _size;
  size_t _capacity;
  nanocbor_value_t _des;
  nanocbor_value_t _map;
  nanocbor_value_t _array;

  typedef enum
  {
    INIT,
    MAP,
    MAP_FIXED,
    ARRAY,
    ARRAY_FIXED,
  } State;
  State _state;
  nanocbor_value_t *get_des();

public:
  //  CborDeserializer(size_t size);
  CborDeserializer(const uint8_t *bytes, size_t size)
  {
    _bytes = bytes;
    _size = size;
    _capacity = size;
    nanocbor_decoder_init(&_des, _bytes, _size);
  };
  ~CborDeserializer();

  // Res fill_buffer(Bytes &b);
  Res deserialize(uint8_t &i);
  Res deserialize(int8_t &i);

  Res deserialize(int32_t &i);
  Res deserialize(uint64_t &val);
  Res deserialize(int64_t &val);
  Res deserialize(uint32_t &val);
  Res deserialize(std::string &s);
  Res deserialize(Bytes &bytes);
  Res deserialize(float &f);
  // Res deserialize(double& d);
  Res deserialize(bool &b);
  Res skip_next();

  Res map_begin();
  Res map_begin(size_t &count);
  Res map_end();
  Res array_begin();
  Res array_begin(size_t &count);
  Res array_end();
  Res peek_type(SerialType &serial_type);
  Res deserialize_null();
  template <typename U>
  Res deserialize_option(Option<U> &opt)
  {
    if (nanocbor_get_null(get_des()) == 0)
    {
      opt = nullptr;
      return Res::Ok();
    }
    U u;
    RET_ERR(deserialize(u), "Failed to decode option");
    opt = u;
    return Res::Ok();
  }
};

template <typename T>
Option<T> cbor_deserialize(const Bytes &bytes)
{
  // INFO("Deserializing %d bytes", bytes.size());
  CborDeserializer des(bytes.data(), bytes.size());
  T obj;
  if (obj.deserialize(des).is_ok())
    return obj;
  ERROR("Failed to deserialize object ");
  return nullptr;
}

#endif