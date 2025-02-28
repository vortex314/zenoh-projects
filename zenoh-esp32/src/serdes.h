#ifndef SERDES_H
#define SERDES_H
#include <util.h>

class Serializer;
class Deserializer;

class Serializable
{
public:
  virtual Res serialize(Serializer &ser) = 0;
  virtual Res deserialize(Deserializer &des) = 0;
};

class Serializer
{
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
  virtual Res serialize(bool b) = 0;
  virtual Res serialize(int i) = 0;
  virtual Res serialize_null() = 0;
  template <typename V>
  Res serialize(std::optional<V> value)
  {
    if (value)
    {
      serialize(*value);
    }
    else
    {
      serialize_null();
    }
    return Res::Ok();
  }
  template <typename V>
  Res serialize(uint32_t idx, std::optional<V> value)
  {
    if (value)
    {
      serialize(idx);
      serialize(*value);
    }
    return Res::Ok();
  }
  Res serialize(Serializable &value) { return value.serialize(*this); }

  virtual Res map_begin() = 0;
  virtual Res map_end() = 0;
  virtual Res array_begin() = 0;
  virtual Res array_begin(size_t size) = 0;
  virtual Res array_end() = 0;
  virtual Res reset() = 0;
};

typedef enum
{
  SER_UINT = 0,
  SER_SINT,
  SER_FLOAT,
  SER_STR,
  SER_BYTES,
  SER_ARRAY = 5,
  SER_ARRAY_FIXED,
  SER_MAP,
  SER_MAP_FIXED,
  SER_END,
} SerialType;

class Deserializer
{
public:
  // virtual Res fill_buffer(Bytes &b) = 0;
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
  virtual Res skip_next() = 0;
  virtual Res map_begin() = 0;
  virtual Res map_begin(size_t &size) = 0;
  virtual Res map_end() = 0;
  virtual Res array_begin() = 0;
  virtual Res array_begin(size_t &size) = 0;
  virtual Res array_end() = 0;
  virtual Res peek_type(SerialType &type) = 0;
  virtual Res deserialize_null() = 0;

  template <typename U>
  Res deserialize(std::optional<U> &opt)
  {
    U u;
    RET_ERR(deserialize(u), "Failed to decode option");
    opt = u;
    return Res::Ok();
  }

  template <typename U>
  Result<U> deserialize_type(U &u)
  {
    if constexpr (std::is_same<U, int>::value)
    {
      return deserialize_int(u);
    }
    else if constexpr (std::is_same<U, uint32_t>::value)
    {
      return deserialize_uint32(u);
    }
    else if constexpr (std::is_same<U, std::string>::value)
    {
      return deserialize_string(u);
    }
    else if constexpr (std::is_same<U, Bytes>::value)
    {
      return deserialize_bytes(u);
    }
    else if constexpr (std::is_same<U, float>::value)
    {
      return deserialize_float(u);
    }
    else if constexpr (std::is_same<U, bool>::value)
    {
      return deserialize_bool(u);
    }
    else
    {
      return U::deserialize(*this);
    }
    return U::deserialize(*this);
  }

  Res iterate_map(std::function<Res(Deserializer &, uint32_t)> func)
  {
    //    INFO("iterate_map");
    SerialType map_type;
    size_t map_size = 1000;
    size_t count = 0;
    RET_ERR(peek_type(map_type), "Failed to peek type");
    if (map_type == SerialType::SER_MAP_FIXED)
    {
      RET_ERR(map_begin(map_size), "Failed to decode map begin");
  //    INFO("map size %u", map_size);
    }
    else if (map_type == SerialType::SER_MAP)
    {
      RET_ERR(map_begin(), "Failed to decode map begin");
    }
    else
    {
      ERROR("Expected map %d", map_type);
      return Res::Err(0, "Expected map");
    }

    while (true && (count++ < map_size))
    {
      SerialType type;
      RET_ERR(peek_type(type), "Failed to peek type");
      if (type == SerialType::SER_END)
        break;
      if (type != SerialType::SER_UINT)
        return Res::Err(0, "Expected uint");
      uint32_t key;
      RET_ERR(deserialize(key), "Failed to decode key in map");
      RET_ERR(func(*this, key), "Failed to process map entry");
    }

    if (map_type == SerialType::SER_MAP)
    {
      RET_ERR(map_end(), "Failed to decode map end ");
    }
    return Res::Ok();
  }
};

class JsonSerializer : public Serializer
{
private:
  std::string _json;
  typedef enum
  {
    INIT,
    MAP_0,
    MAP_N,
    ARRAY_0,
    ARRAY_N,
  } State;
  State _state;
};

#endif
