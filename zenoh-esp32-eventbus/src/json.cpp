#include "json.h"



JsonSerializer::JsonSerializer(Bytes &bytes) : _bytes(bytes)
{
    
}

JsonSerializer::~JsonSerializer() {}
Res JsonSerializer::reset()
{
    _bytes.clear();
    
    return Res::Ok(true);
}
Res JsonSerializer::serialize(const uint8_t v)
{
    return Res::Ok(true);
}
Res JsonSerializer::serialize(const int8_t v)
{
    return Res::Ok(true);
}
Res JsonSerializer::serialize(const int i)
{
    return Res::Ok(true);
}
Res JsonSerializer::serialize(const bool b)
{
    return Res::Ok(true);
}

Res JsonSerializer::serialize(const int32_t i)
{
    return Res::Ok(true);
}

Res JsonSerializer::serialize(const uint32_t i)
{
    return Res::Ok(true);
}

Res JsonSerializer::serialize(const int64_t i)
{
    return Res::Ok(true);
}

Res JsonSerializer::serialize(const uint64_t i)
{
    return Res::Ok(true);
}

Res JsonSerializer::serialize(const char *s)
{
    return Res::Ok(true);
}

Res JsonSerializer::serialize(const std::string &s)
{
    return Res::Ok(true);
}
Res JsonSerializer::serialize(const Bytes b)
{

    return Res::Ok(true);
}
Res JsonSerializer::serialize(const float f)
{
    return Res::Ok(true);
}
Res JsonSerializer::map_begin()
{
    return Res::Ok(true);
}
Res JsonSerializer::map_end()
{
    return Res::Ok(true);
}
Res JsonSerializer::array_begin()
{
    _state = ARRAY;
    return Res::Ok(true);
}
Res JsonSerializer::array_begin(size_t count)
{
    _state = ARRAY_FIXED;
    return Res::Ok(true);
}
Res JsonSerializer::array_end()
{

    return Res::Ok(true);
}
Res JsonSerializer::serialize_null()
{
    return Res::Ok(true);
}

Res JsonSerializer::serialize(const Serializable &value) { return value.serialize(*this); }



JsonDeserializer::~JsonDeserializer()
{
    // delete bytes only when owner of bytes
}





JsonDeserializer::JsonDeserializer(const uint8_t *bytes, size_t size)     {
 //   nanoJson_decoder_init(&_des, _bytes, _size);
}

Res JsonDeserializer::deserialize(uint8_t &i)
{
        return Res::Ok(true);
}

Res JsonDeserializer::deserialize(int8_t &i)
{
    return Res::Ok(true);
}

Res JsonDeserializer::deserialize(int32_t &i)
{
    return Res::Ok(true);
}

Res JsonDeserializer::deserialize(uint64_t &val)
{
    return Res::Ok(true);
}

Res JsonDeserializer::deserialize(int64_t &val)
{
    return Res::Ok(true);
}

Res JsonDeserializer::deserialize(uint32_t &val)
{
    return Res::Ok(true);
}

Res JsonDeserializer::deserialize(std::string &s)
{

    return Res::Ok(true);
}
Res JsonDeserializer::deserialize(Bytes &bytes)
{

    return Res::Ok(true);
}
Res JsonDeserializer::deserialize(float &f)
{

    return Res::Ok(true);
}

Res JsonDeserializer::deserialize(bool &b)
{

    return Res::Ok(true);
}

Res JsonDeserializer::skip_next()
{

    return Res::Ok(true);
}

Res JsonDeserializer::map_begin()
{

    return Res::Ok(true);
}
Res JsonDeserializer::map_begin(size_t &count)
{

    return Res::Ok(true);
}
Res JsonDeserializer::map_end()
{

    return Res::Ok(true);
}

Res JsonDeserializer::array_begin()
{

    return Res::Ok(true);
}
Res JsonDeserializer::array_begin(size_t &count)
{

    return Res::Ok(true);
}
Res JsonDeserializer::array_end()
{

    return Res::Ok(true);
}
Res JsonDeserializer::peek_type(SerialType &serial_type)
{
    
    return Res::Ok(true);
}

Res JsonDeserializer::deserialize_null()
{
    return Res::Ok(true);
}
