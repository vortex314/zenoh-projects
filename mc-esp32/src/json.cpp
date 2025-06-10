#include "json.h"



JsonSerializer::JsonSerializer(Bytes &bytes) : _bytes(bytes)
{
    
}

JsonSerializer::~JsonSerializer() {}
Res JsonSerializer::reset()
{
    _bytes.clear();
    
    return ResOk;
}
Res JsonSerializer::serialize(const uint8_t v)
{
    return ResOk;
}
Res JsonSerializer::serialize(const int8_t v)
{
    return ResOk;
}
Res JsonSerializer::serialize(const int i)
{
    return ResOk;
}
Res JsonSerializer::serialize(const bool b)
{
    return ResOk;
}

Res JsonSerializer::serialize(const int32_t i)
{
    return ResOk;
}

Res JsonSerializer::serialize(const uint32_t i)
{
    return ResOk;
}

Res JsonSerializer::serialize(const int64_t i)
{
    return ResOk;
}

Res JsonSerializer::serialize(const uint64_t i)
{
    return ResOk;
}

Res JsonSerializer::serialize(const char *s)
{
    return ResOk;
}

Res JsonSerializer::serialize(const std::string &s)
{
    return ResOk;
}
Res JsonSerializer::serialize(const Bytes b)
{

    return ResOk;
}
Res JsonSerializer::serialize(const float f)
{
    return ResOk;
}
Res JsonSerializer::map_begin()
{
    return ResOk;
}
Res JsonSerializer::map_end()
{
    return ResOk;
}
Res JsonSerializer::array_begin()
{
    _state = ARRAY;
    return ResOk;
}
Res JsonSerializer::array_begin(size_t count)
{
    _state = ARRAY_FIXED;
    return ResOk;
}
Res JsonSerializer::array_end()
{

    return ResOk;
}
Res JsonSerializer::serialize_null()
{
    return ResOk;
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
        return ResOk;
}

Res JsonDeserializer::deserialize(int8_t &i)
{
    return ResOk;
}

Res JsonDeserializer::deserialize(int32_t &i)
{
    return ResOk;
}

Res JsonDeserializer::deserialize(uint64_t &val)
{
    return ResOk;
}

Res JsonDeserializer::deserialize(int64_t &val)
{
    return ResOk;
}

Res JsonDeserializer::deserialize(uint32_t &val)
{
    return ResOk;
}

Res JsonDeserializer::deserialize(std::string &s)
{

    return ResOk;
}
Res JsonDeserializer::deserialize(Bytes &bytes)
{

    return ResOk;
}
Res JsonDeserializer::deserialize(float &f)
{

    return ResOk;
}

Res JsonDeserializer::deserialize(bool &b)
{

    return ResOk;
}

Res JsonDeserializer::skip_next()
{

    return ResOk;
}

Res JsonDeserializer::map_begin()
{

    return ResOk;
}
Res JsonDeserializer::map_begin(size_t &count)
{

    return ResOk;
}
Res JsonDeserializer::map_end()
{

    return ResOk;
}

Res JsonDeserializer::array_begin()
{

    return ResOk;
}
Res JsonDeserializer::array_begin(size_t &count)
{

    return ResOk;
}
Res JsonDeserializer::array_end()
{

    return ResOk;
}
Res JsonDeserializer::peek_type(SerialType &serial_type)
{
    
    return ResOk;
}

Res JsonDeserializer::deserialize_null()
{
    return ResOk;
}
