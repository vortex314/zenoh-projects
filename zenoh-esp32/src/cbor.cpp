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
    return ResOk;
}
Res CborSerializer::serialize(const uint8_t v)
{
    RET_ERRI(nanocbor_fmt_uint(&_enc, v), "Failed to encode uint8_t");
    return ResOk;
}
Res CborSerializer::serialize(const int8_t v)
{
    RET_ERRI(nanocbor_fmt_int(&_enc, v), "Failed to encode int8_t");
    return ResOk;
}
Res CborSerializer::serialize(const int i)
{
    RET_ERRI(nanocbor_fmt_int(&_enc, i), "Failed to encode int");
    return ResOk;
}
Res CborSerializer::serialize(const bool b)
{
    RET_ERRI(nanocbor_fmt_bool(&_enc, b), "Failed to encode bool");
    return ResOk;
}

Res CborSerializer::serialize(const int32_t i)
{
    RET_ERRI(nanocbor_fmt_int(&_enc, i), "Failed to encode int");
    return ResOk;
}

Res CborSerializer::serialize(const uint32_t i)
{
    RET_ERRI(nanocbor_fmt_uint(&_enc, i), "Failed to encode uint32_t");
    return ResOk;
}

Res CborSerializer::serialize(const int64_t i)
{
    RET_ERRI(nanocbor_fmt_int(&_enc, i), "Failed to encode int64_t");
    return ResOk;
}

Res CborSerializer::serialize(const uint64_t i)
{
    RET_ERRI(nanocbor_fmt_uint(&_enc, i), "Failed to encode uint64_t");
    return ResOk;
}

Res CborSerializer::serialize(const char *s)
{
    RET_ERRI(nanocbor_put_tstr(&_enc, s), "Failed to encode string");
    return ResOk;
}

Res CborSerializer::serialize(const std::string &s)
{
    RET_ERRI(nanocbor_put_tstr(&_enc, s.c_str()), "Failed to encode string");
    return ResOk;
}
Res CborSerializer::serialize(const Bytes b)
{
    RET_ERRI(nanocbor_put_bstr(&_enc, b.data(), b.size()),
             "Failed to encode bytes");
    return ResOk;
}
Res CborSerializer::serialize(const float f)
{
    RET_ERRI(nanocbor_fmt_float(&_enc, f), "Failed to encode float");
    return ResOk;
}
Res CborSerializer::map_begin()
{
    RET_ERRI(nanocbor_fmt_map_indefinite(&_enc), "Failed to encode map");
    return ResOk;
}
Res CborSerializer::map_end()
{
    RET_ERRI(nanocbor_fmt_end_indefinite(&_enc), "Failed to encode map");
    return ResOk;
}
Res CborSerializer::array_begin()
{
    RET_ERRI(nanocbor_fmt_array_indefinite(&_enc), "Failed to encode array");
    _state = ARRAY;
    return ResOk;
}
Res CborSerializer::array_begin(size_t count)
{
    RET_ERRI(nanocbor_fmt_array(&_enc, count), "Failed to encode array");
    _state = ARRAY_FIXED;
    return ResOk;
}
Res CborSerializer::array_end()
{
    if (_state == ARRAY)
        RET_ERRI(nanocbor_fmt_end_indefinite(&_enc), "Failed to encode array");
    return ResOk;
}
Res CborSerializer::serialize_null()
{
    RET_ERRI(nanocbor_fmt_null(&_enc), "Failed to encode null");
    return ResOk;
}

Res CborSerializer::serialize(const Serializable &value) { return value.serialize(*this); }

nanocbor_value_t *CborDeserializer::get_des()
{
    switch (_state)
    {
    case INIT:
        return &_des;
    case MAP:
    case MAP_FIXED:
        return &_map;
    case ARRAY:
    case ARRAY_FIXED:
        return &_array;
    }
    return &_des;
}

CborDeserializer::~CborDeserializer()
{
    // delete bytes only when owner of bytes
}



template <typename T>
Res decode_number(T &i, nanocbor_value_t *des)
{
    int type = nanocbor_get_type(des);
    switch (type)
    {
    case NANOCBOR_TYPE_UINT:
    {
        uint64_t v;
        RET_ERRI(nanocbor_get_uint64(des, &v), "decode fail");
        i = v;
        break;
    }
    case NANOCBOR_TYPE_NINT:
    {
        int64_t v;
        RET_ERRI(nanocbor_get_int64(des, &v), "decode fail");
        i = v;
        break;
    }
    case NANOCBOR_TYPE_FLOAT:
    {
        double v;
        RET_ERRI(nanocbor_get_double(des, &v), "decode fail");
        i = v;
        break;
    }
    default:
    {
        RET_ERRI(-1, "decode fail");
        break;
    }
    }
    return ResOk;
}

CborDeserializer::CborDeserializer(const uint8_t *bytes, size_t size):_bytes(bytes),_size(size)     {
    nanocbor_decoder_init(&_des, _bytes, _size);
}

Res CborDeserializer::deserialize(uint8_t &i)
{
    return decode_number(i, get_des());
}

Res CborDeserializer::deserialize(int8_t &i)
{
    return decode_number(i, get_des());
}

Res CborDeserializer::deserialize(int32_t &i)
{
    return decode_number(i, get_des());
}

Res CborDeserializer::deserialize(uint64_t &val)
{
    return decode_number(val, get_des());
}

Res CborDeserializer::deserialize(int64_t &val)
{
    return decode_number(val, get_des());
}

Res CborDeserializer::deserialize(uint32_t &val)
{
    return decode_number(val, get_des());
}

Res CborDeserializer::deserialize(std::string &s)
{
    const uint8_t *val;
    size_t len;
    RET_ERRI(nanocbor_get_tstr(get_des(), &val, &len),
             "Failed to decode string");
    s = std::string(val, val + len);
    return ResOk;
}
Res CborDeserializer::deserialize(Bytes &bytes)
{
    const uint8_t *val;
    size_t len;
    RET_ERRI(nanocbor_get_bstr(get_des(), &val, &len),
             "Failed to decode bytes");
    bytes = Bytes(val, val + len);
    return ResOk;
}
Res CborDeserializer::deserialize(float &f)
{
    float val;
    RET_ERRI(nanocbor_get_float(get_des(), &val), "Failed to decode float");
    f = val;
    return ResOk;
}

Res CborDeserializer::deserialize(bool &b)
{
    bool val;
    RET_ERRI(nanocbor_get_bool(get_des(), &val), "Failed to decode bool");
    b = val;
    return ResOk;
}

Res CborDeserializer::skip_next()
{
    CHECK(nanocbor_skip(get_des()));
    return ResOk;
}

Res CborDeserializer::map_begin()
{
    CHECK(nanocbor_enter_map(get_des(), &_map));
    _state = MAP;
    return ResOk;
}
Res CborDeserializer::map_begin(size_t &count)
{
    CHECK(nanocbor_enter_map(get_des(), &_map));
    _state = MAP_FIXED;
    count = ((size_t)(&_map)->remaining) / 2;
    return ResOk;
}
Res CborDeserializer::map_end()
{
    nanocbor_leave_container(get_des(), &_map);
    _state = INIT;
    return ResOk;
}

Res CborDeserializer::array_begin()
{
    CHECK(nanocbor_enter_array(get_des(), &_array));
    _state = ARRAY;
    return ResOk;
}
Res CborDeserializer::array_begin(size_t &count)
{
    CHECK(nanocbor_enter_array(get_des(), &_array));
    _state = ARRAY_FIXED;
    return ResOk;
}
Res CborDeserializer::array_end()
{
    nanocbor_leave_container(get_des(), &_array);
    _state = INIT;
    return ResOk;
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
            return Res(-1, "Unknown type");
        }
    }
    }
    return ResOk;
}

Res CborDeserializer::deserialize_null()
{
    RET_ERRI(nanocbor_get_null(get_des()), "Failed to decode null");
    return ResOk;
}
