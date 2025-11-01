#ifndef __JSON_H__
#define __JSON_H__

#include <ArduinoJson.h>
#include <optional>
#include <serdes.h>
#include <string.h>
#include <string>
#include <vector>
#include <util.h>

class JsonSerializer : public Serializer
{
private:
  JsonDocument _doc;
  Bytes &_bytes;
  typedef enum
  {
    INIT,
    MAP,
    ARRAY,
    ARRAY_FIXED,
  } State;
  State _state;

public:
  JsonSerializer(Bytes &bytes);
  ~JsonSerializer();
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

  /*Res serialize(const char *s, const uint8_t v);
  Res serialize(const char *s, const int8_t v);
  Res serialize(const char *s, const int i);
  Res serialize(const char *s, const bool b);
  Res serialize(const char *s, const int32_t i);
  Res serialize(const char *s, const uint32_t i);
  Res serialize(const char *s, const int64_t i);
  Res serialize(const char *s, const uint64_t i);
  Res serialize(const char *s, const std::string &str);
  Res serialize(const char *s, const char *str);
  Res serialize(const char *s, const Bytes b);
  Res serialize(const char *s, const float f);*/

  Res map_begin();
  Res map_end();
  Res array_begin();
  Res array_begin(size_t count);
  Res array_end();
  Res serialize_null();

  template <typename V>
  Res serialize(char const *idx, V value)
  {
    _doc[idx] = value;
    return Res::Ok(true);
  }

  Res serialize(const char *key, uint8_t v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, int8_t v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, uint16_t v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, int16_t v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, uint32_t v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, int32_t v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, uint64_t v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, int64_t v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, const std::string& v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

   Res serialize(const char *key, const char* v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

     Res serialize(const char *key, int v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, float v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, bool v)
  {
    _doc[key] = v;
    return Res::Ok(true);
  }

  Res serialize(const char *key, Bytes v)
  {
    //TODO base64 bytes
    _doc[key] = v.size();
    return Res::Ok(true);
  }

  template <typename V>
  Res serialize(char const *idx, Option<V> opt)
  {
    if (opt)
    {
      _doc[idx] = *opt;
    }
    return Res::Ok(true);
  }

  Res serialize(const Serializable &value);
};

#define MAX_TSTR_LENGTH 256
#define MAX_BSTR_LENGTH 256

class JsonDeserializer : public Deserializer
{

private:
  /* data */

  typedef enum
  {
    INIT,
    MAP,
    MAP_FIXED,
    ARRAY,
    ARRAY_FIXED,
  } State;
  State _state;

public:
  //  JsonDeserializer(size_t size);
  JsonDeserializer(const uint8_t *bytes, size_t size);
  ~JsonDeserializer();

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
};

template <typename T>
Option<T> json_deserialize(const Bytes &bytes)
{
  // INFO("Deserializing %d bytes", bytes.size());
  JsonDeserializer des(bytes.data(), bytes.size());
  T obj;
  auto r = obj.deserialize(des);
  if (r.is_err())
  {
    ERROR("Failed to deserialize %s", r.msg());
    return Option<T>(nullptr);
  }
  return Option<T>(obj);
}

#endif