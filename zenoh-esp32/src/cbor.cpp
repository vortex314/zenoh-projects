#include "cbor.h"

void CborSerializer::append_func(nanocbor_encoder_t *enc, void *ctx, const uint8_t *data, size_t len)
{
    CborSerializer *ser = (CborSerializer *)ctx;
    for (size_t i = 0; i < len; i++)
    {
        ser->_bytes.push_back(data[i]);
    }
}
// typedef bool (*nanocbor_encoder_fits)(nanocbor_encoder_t *enc, void *ctx, size_t len);

bool CborSerializer::fits_func(nanocbor_encoder_t *enc, void *ctx, size_t len)
{
    return true;
}

CborSerializer::CborSerializer(Bytes &bytes) : _bytes(bytes)
{
    nanocbor_encoder_stream_init(&_enc, this,
                                 CborSerializer::append_func, CborSerializer::fits_func);
}

CborSerializer::~CborSerializer() {}
Res CborSerializer::reset()
{
    _bytes.clear();
    nanocbor_encoder_stream_init(&_enc, this,
                                 CborSerializer::append_func, CborSerializer::fits_func);
    return Res::Ok();
}
Res CborSerializer::serialize(uint8_t v)
{
    RET_ERRI(nanocbor_fmt_uint(&_enc, v), "Failed to encode uint8_t");
    return Res::Ok();
}
Res CborSerializer::serialize(int8_t v)
{
    RET_ERRI(nanocbor_fmt_int(&_enc, v), "Failed to encode int8_t");
    return Res::Ok();
}
Res CborSerializer::serialize(int i)
{
    RET_ERRI(nanocbor_fmt_int(&_enc, i), "Failed to encode int");
    return Res::Ok();
}
Res CborSerializer::serialize(bool b)
{
    RET_ERRI(nanocbor_fmt_bool(&_enc, b), "Failed to encode bool");
    return Res::Ok();
}

Res CborSerializer::serialize(int32_t i)
{
    RET_ERRI(nanocbor_fmt_int(&_enc, i), "Failed to encode int");
    return Res::Ok();
}

Res CborSerializer::serialize(uint32_t i)
{
    RET_ERRI(nanocbor_fmt_uint(&_enc, i), "Failed to encode uint32_t");
    return Res::Ok();
}

Res CborSerializer::serialize(int64_t i)
{
    RET_ERRI(nanocbor_fmt_int(&_enc, i), "Failed to encode int64_t");
    return Res::Ok();
}

Res CborSerializer::serialize(uint64_t i)
{
    RET_ERRI(nanocbor_fmt_uint(&_enc, i), "Failed to encode int64_t");
    return Res::Ok();
}

Res CborSerializer::serialize(std::string &s)
{
    RET_ERRI(nanocbor_put_tstr(&_enc, s.c_str()), "Failed to encode string");
    return Res::Ok();
}
Res CborSerializer::serialize(Bytes b)
{
    RET_ERRI(nanocbor_put_bstr(&_enc, b.data(), b.size()),
             "Failed to encode bytes");
    return Res::Ok();
}
Res CborSerializer::serialize(float f)
{
    RET_ERRI(nanocbor_fmt_float(&_enc, f), "Failed to encode float");
    return Res::Ok();
}
Res CborSerializer::map_begin()
{
    RET_ERRI(nanocbor_fmt_map_indefinite(&_enc), "Failed to encode map");
    return Res::Ok();
}
Res CborSerializer::map_end()
{
    RET_ERRI(nanocbor_fmt_end_indefinite(&_enc), "Failed to encode map");
    return Res::Ok();
}
Res CborSerializer::array_begin()
{
    RET_ERRI(nanocbor_fmt_array_indefinite(&_enc), "Failed to encode array");
    _state = ARRAY;
    return Res::Ok();
}
Res CborSerializer::array_begin(size_t count)
{
    RET_ERRI(nanocbor_fmt_array(&_enc, count), "Failed to encode array");
    _state = ARRAY_FIXED;
    return Res::Ok();
}
Res CborSerializer::array_end()
{
    if (_state == ARRAY)
        RET_ERRI(nanocbor_fmt_end_indefinite(&_enc), "Failed to encode array");
    return Res::Ok();
}
Res CborSerializer::serialize_null()
{
    RET_ERRI(nanocbor_fmt_null(&_enc), "Failed to encode null");
    return Res::Ok();
}

Res CborSerializer::serialize(Serializable &value) { return value.serialize(*this); }

nanocbor_value_t *CborDeserializer::get_des()
{
    switch (_state)
    {
    case INIT:
        return &_des;
    case MAP:
        return &_map;
    case ARRAY:
    case ARRAY_FIXED:
        return &_array;
    }
    return &_des;
}

/*CborDeserializer::CborDeserializer(size_t size)
{
    _bytes = new uint8_t[size];
    _size = 0;
    _capacity = size;
    _state = INIT;
}*/
CborDeserializer::~CborDeserializer()
{
    // delete bytes only when owner of bytes
}
/*
Res CborDeserializer::fill_buffer(Bytes &b)
{
    if (b.size() > _capacity)
    {
        return Res::Err(ENOSPC, "Buffer too small");
    }
    memcpy(_bytes, b.data(), b.size());
    _size = b.size();
    _state = INIT;
    nanocbor_decoder_init(get_des(), _bytes, _size);
    return Res::Ok();
}*/

Res CborDeserializer::deserialize(uint8_t &i)
{
    RET_ERRI(nanocbor_get_uint8(get_des(), &i), "Failed to decode uint8_t");
    return Res::Ok();
}

Res CborDeserializer::deserialize(int8_t &i)
{
    RET_ERRI(nanocbor_get_int8(get_des(), &i), "Failed to decode int8_t");
    return Res::Ok();
}

Res CborDeserializer::deserialize(int32_t &i)
{
    int64_t val;
    RET_ERRI(nanocbor_get_int64(get_des(), &val), "Failed to decode int");
    if (val > INT32_MAX || val < INT32_MIN)
    {
        return Res::Err(ERANGE, "Value out of range");
    }
    i = val;
    return Res::Ok();
}

Res CborDeserializer::deserialize(uint64_t &val)
{
    RET_ERRI(nanocbor_get_uint64(get_des(), &val), "Failed to decode uint64_t");
    return Res::Ok();
}

Res CborDeserializer::deserialize(int64_t &val)
{
    RET_ERRI(nanocbor_get_int64(get_des(), &val), "Failed to decode int64_t");
    return Res::Ok();
}

Res CborDeserializer::deserialize(uint32_t &val)
{
    RET_ERRI(nanocbor_get_uint32(get_des(), &val), "Failed to decode uint32_t");
    return Res::Ok();
}

Res CborDeserializer::deserialize(std::string &s)
{
    const uint8_t *val;
    size_t len;
    RET_ERRI(nanocbor_get_tstr(get_des(), &val, &len),
             "Failed to decode string");
    s = std::string(val, val + len);
    return Res::Ok();
}
Res CborDeserializer::deserialize(Bytes &bytes)
{
    const uint8_t *val;
    size_t len;
    RET_ERRI(nanocbor_get_bstr(get_des(), &val, &len),
             "Failed to decode bytes");
    bytes = Bytes(val, val + len);
    return Res::Ok();
}
Res CborDeserializer::deserialize(float &f)
{
    float val;
    RET_ERRI(nanocbor_get_float(get_des(), &val), "Failed to decode float");
    return Res::Ok();
}

Res CborDeserializer::deserialize(bool &b)
{
    bool val;
    RET_ERRI(nanocbor_get_bool(get_des(), &val), "Failed to decode bool");
    b = val;
    return Res::Ok();
}

Res CborDeserializer::skip_next() {
    CHECK(nanocbor_skip(get_des()));
    return Res::Ok();
}

Res CborDeserializer::map_begin()
{
    CHECK(nanocbor_enter_map(get_des(), &_map));
    _state = MAP;
    return Res::Ok();
}
Res CborDeserializer::map_end()
{
    nanocbor_leave_container(get_des(), &_map);
    _state = INIT;
    return Res::Ok();
}
Res CborDeserializer::array_begin()
{
    CHECK(nanocbor_enter_array(get_des(), &_array));
    _state = ARRAY;
    return Res::Ok();
}
Res CborDeserializer::array_begin(size_t &count)
{
    CHECK(nanocbor_enter_array(get_des(), &_array));
    _state = ARRAY_FIXED;
    return Res::Ok();
}
Res CborDeserializer::array_end()
{
    nanocbor_leave_container(get_des(), &_array);
    _state = INIT;
    return Res::Ok();
}
Res CborDeserializer::peek_type(SerialType &serial_type)
{
    int type = nanocbor_get_type(get_des());
    switch (type)
    {
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
        serial_type = SerialType::SER_ARRAY_FIXED;
        break;
    case NANOCBOR_TYPE_MAP:
        serial_type = SerialType::SER_MAP_FIXED;
        break;
    case NANOCBOR_ERR_END:
        serial_type = SerialType::SER_END;
        break;
    default:
    {
        uint8_t b = *(uint8_t *)(get_des()->cur);
        INFO(" found %x", b);
        if (b == 0x9F)
        {
            serial_type = SerialType::SER_ARRAY;
        }
        else if (b == 0xBF)
        {
            serial_type = SerialType::SER_MAP;
        }
        else
        {
            return Res::Err(-1, "Unknown type");
        }
    }
    }
    return Res::Ok();
}

Res CborDeserializer::deserialize_null()
{
    RET_ERRI(nanocbor_get_null(get_des()), "Failed to decode null");
    return Res::Ok();
}
