#include <codec.h>

void panic(const char *msg)
{
    ERROR("Panic: %s\n", msg);
    while (1)
    {
    }
}
// CRC-16 function (CRC-CCITT)
uint16_t crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// COBS encoding function
std::vector<uint8_t> cobs_encode(const std::vector<uint8_t> &input)
{
    std::vector<uint8_t> output(input.size() + 2);
    size_t read_index = 0, write_index = 1, code_index = 0;
    uint8_t code = 1;

    while (read_index < input.size())
    {
        if (input[read_index] == 0)
        {
            output[code_index] = code;
            code_index = write_index++;
            code = 1;
        }
        else
        {
            output[write_index++] = input[read_index];
            code++;
            if (code == 0xFF)
            {
                output[code_index] = code;
                code_index = write_index++;
                code = 1;
            }
        }
        read_index++;
    }
    output[code_index] = code;
    output[write_index++] = 0; // COBS terminator

    output.resize(write_index);
    return output;
}

// COBS decoding function
std::vector<uint8_t> cobs_decode(const std::vector<uint8_t> &input)
{
    std::vector<uint8_t> output(input.size());
    size_t read_index = 0, write_index = 0;
    uint8_t code = 0, i = 0;

    while (read_index < input.size())
    {
        code = input[read_index];
        if (read_index + code > input.size() && code != 1)
        {
            output.clear();
            return output;
            // throw std::runtime_error("COBS decode error");
        }
        read_index++;
        for (i = 1; i < code; i++)
        {
            output[write_index++] = input[read_index++];
        }
        if (code != 0xFF && read_index < input.size())
        {
            output[write_index++] = 0;
        }
    }

    output.resize(write_index);
    return output;
}

// FrameEncoder Class

FrameEncoder::FrameEncoder(uint32_t max) : _buffer(), _max(max)
{
    _buffer.reserve(max);
}

Result<Void> FrameEncoder::write_byte(uint8_t byte)
{
    // INFO("write_byte : 0x%X at offset %d", byte, _buffer.size());
    if (_buffer.size() + 1 > _max)
    {
        return Result<Void>::Err(ENOSPC);
    }
    _buffer.push_back(byte);
    return Result<Void>::Ok(Void());
}

template <typename T>
Result<Void> FrameEncoder::encode(Option<T> value)
{
    if (value.is_none())
    {
        return encode_null();
    }
    return encode(value.unwrap());
}

Result<Void> FrameEncoder::encode(int32_t value)
{
    if (_buffer.size() + 4 > _max)
    {
        return Result<Void>::Err(ENOSPC);
    }
    // First, calculate the positive encoding of the absolute value minus 1
    if (value >= 0)
    {
        return encode(value);
    }
    uint32_t encoded_value = -(value + 1); // CBOR encodes -1 as 0, -2 as 1, etc.
                                           //  INFO("Encoded value : %llu", encoded_value);

    if (encoded_value <= 23)
    {
        // Values from -1 to -24 are encoded as a single byte (0x20 to 0x37)
        RET_ERR(write_byte((0x20 | encoded_value)));
    }
    else if (encoded_value <= 0xFF)
    {
        // Values from -25 to -256 are encoded with 0x38 followed by 1 byte
        RET_ERR(write_byte(0x38)); // CBOR additional info for 1-byte negative integer
        RET_ERR(write_byte(encoded_value));
    }
    else if (encoded_value <= 0xFFFF)
    {
        // Values from -257 to -65536 are encoded with 0x39 followed by 2 bytes (big-endian)
        RET_ERR(write_byte(0x39));                        // CBOR additional info for 2-byte negative integer
        RET_ERR(write_byte((encoded_value >> 8) & 0xFF)); // High byte
        RET_ERR(write_byte(encoded_value & 0xFF));        // Low byte
    }
    else if (encoded_value <= 0xFFFFFFFF)
    {
        // Values from -65537 to -4294967296 are encoded with 0x3A followed by 4 bytes (big-endian)
        RET_ERR(write_byte(0x3A));                         // CBOR additional info for 4-byte negative integer
        RET_ERR(write_byte((encoded_value >> 24) & 0xFF)); // Highest byte
        RET_ERR(write_byte((encoded_value >> 16) & 0xFF));
        RET_ERR(write_byte((encoded_value >> 8) & 0xFF));
        RET_ERR(write_byte(encoded_value & 0xFF)); // Lowest byte
    } /*
     else
     {
         // Values smaller than -4294967296 are encoded with 0x3B followed by 8 bytes (big-endian)
         RET_ERR(write_byte(0x3B));                         // CBOR additional info for 8-byte negative integer
         RET_ERR(write_byte((encoded_value >> 56) & 0xFF)); // Highest byte
         RET_ERR(write_byte((encoded_value >> 48) & 0xFF));
         RET_ERR(write_byte((encoded_value >> 40) & 0xFF));
         RET_ERR(write_byte((encoded_value >> 32) & 0xFF));
         RET_ERR(write_byte((encoded_value >> 24) & 0xFF));
         RET_ERR(write_byte((encoded_value >> 16) & 0xFF));
         RET_ERR(write_byte((encoded_value >> 8) & 0xFF));
         RET_ERR(write_byte(encoded_value & 0xFF)); // Lowest byte
     }*/

    return Result<Void>::Ok({});
}

Result<Void> FrameEncoder::encode(uint32_t value)
{
    if (value <= 23)
    {
        // Values from -1 to -24 are encoded as a single byte (0x20 to 0x37)
        RET_ERR(write_byte(value));
    }
    else if (value <= 0xFF)
    {
        RET_ERR(write_byte(0x18)); // CBOR additional info for 1-byte negative integer
        RET_ERR(write_byte(value));
    }
    else if (value <= 0xFFFF)
    {
        RET_ERR(write_byte(0x19));                // CBOR additional info for 2-byte negative integer
        RET_ERR(write_byte((value >> 8) & 0xFF)); // High byte
        RET_ERR(write_byte(value & 0xFF));        // Low byte
    }
    else if (value <= 0xFFFFFFFF)
    {
        RET_ERR(write_byte(0x1A));                 // CBOR additional info for 4-byte negative integer
        RET_ERR(write_byte((value >> 24) & 0xFF)); // Highest byte
        RET_ERR(write_byte((value >> 16) & 0xFF));
        RET_ERR(write_byte((value >> 8) & 0xFF));
        RET_ERR(write_byte(value & 0xFF)); // Lowest byte
    } /*
     else
     {
         RET_ERR(write_byte(0x1B));                         // CBOR additional info for 8-byte negative integer
         RET_ERR(write_byte((value >> 56) & 0xFF)); // Highest byte
         RET_ERR(write_byte((value >> 48) & 0xFF));
         RET_ERR(write_byte((value >> 40) & 0xFF));
         RET_ERR(write_byte((value >> 32) & 0xFF));
         RET_ERR(write_byte((value >> 24) & 0xFF));
         RET_ERR(write_byte((value >> 16) & 0xFF));
         RET_ERR(write_byte((value >> 8) & 0xFF));
         RET_ERR(write_byte(value & 0xFF)); // Lowest byte
     }*/

    return Result<Void>::Ok({});
}

Result<Void> FrameEncoder::encode(const char *str)
{
    size_t len = strlen(str);

    if (_buffer.size() + len + 1 > _max)
    {
        return Result<Void>::Err(ENOSPC);
    }
    RET_ERR(write_byte(0x60 | len)); // Major type 3 (text string)
    _buffer.insert(_buffer.end(), str, str + len);
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::encode(std::vector<uint8_t> &buffer)
{
    size_t len = buffer.size();
    if (_buffer.size() + len + 1 > _max)
    {
        return Result<Void>::Err(ENOSPC);
    }
    RET_ERR(write_byte(0x40 | len)); // Major type 2 (byte string)
    _buffer.insert(_buffer.end(), buffer.begin(), buffer.end());
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::encode(float value)
{
    RET_ERR(write_byte(0xFA)); // Major type 7, additional 26 (32-bit float)
    uint32_t float_bits;
    memcpy(&float_bits, &value, sizeof(float)); // Copy the float bits into an unsigned 32-bit integer
    // Store the 4 bytes of the float in big-endian order
    RET_ERR(write_byte((float_bits >> 24) & 0xFF));
    RET_ERR(write_byte((float_bits >> 16) & 0xFF));
    RET_ERR(write_byte((float_bits >> 8) & 0xFF));
    RET_ERR(write_byte(float_bits & 0xFF));
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::encode_array()
{
    RET_ERR(write_byte(0x9F)); // Array of indefinite length
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::encode_map()
{
    RET_ERR(write_byte(0xBF)); // Map of indefinite length
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::encode_end()
{
    RET_ERR(write_byte(0xFF)); // CBOR "break" byte
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::encode_null()
{
    RET_ERR(write_byte(0xF6)); // CBOR null value
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::add_crc()
{
    uint16_t crc = crc16(_buffer.data(), _buffer.size());
    RET_ERR(write_byte(crc >> 8));
    RET_ERR(write_byte(crc & 0xFF));
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::add_cobs()
{
    _buffer = cobs_encode(_buffer);
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::read_buffer(uint8_t *buf, size_t len)
{
    if (len < _buffer.size())
    {
        return Result<Void>::Err(ENOSPC);
    }
    std::memcpy(buf, _buffer.data(), _buffer.size());
    return Result<Void>::Ok(Void());
}

Result<Void> FrameEncoder::read_buffer(std::vector<unsigned char> &buf)
{
    buf = _buffer;
    return Result<Void>::Ok(Void());
}

std::vector<uint8_t> FrameEncoder::get_buffer()
{
    return _buffer;
}

Result<Void> FrameEncoder::clear()
{
    _buffer.clear();
    return Result<Void>::Ok(Void());
}

// FrameDecoder Class

FrameDecoder::FrameDecoder(uint32_t max) : _buffer()
{
    _index = 0;
    _max = max;
}

Result<uint8_t> FrameDecoder::read_next()
{
    if (_index >= _buffer.size())
    {
        return Result<uint8_t>::Err(EINVAL, "At end of buffer read");
    }
    uint8_t value = _buffer[_index++];
    return Result<uint8_t>::Ok(value);
}

Result<uint8_t> FrameDecoder::peek_next()
{
    if (_index >= _buffer.size())
    {
        return Result<uint8_t>::Err(EINVAL, "At end of buffer peek.");
    }
    uint8_t value = _buffer[_index];
    return Result<uint8_t>::Ok(value);
}

Result<Void> FrameDecoder::fill_buffer(std::vector<uint8_t> &buffer)
{
    if (buffer.size() > _max)
    {
        return Result<Void>::Err(ENOSPC, "Buffer overflow");
    }
    _buffer.clear();
    _buffer.insert(_buffer.end(), buffer.begin(), buffer.end());
    _index = 0;
    return Result<Void>::Ok(Void());
}

Result<bool> FrameDecoder::fill_buffer(uint8_t *buffer, uint32_t size)
{
    if (size > _max)
    {
        return Result<bool>::Err(ENOSPC, "Buffer overflow");
    }
    _buffer.clear();
    _buffer.insert(_buffer.end(), buffer, buffer + size);
    _index = 0;
    return Result<bool>::Ok(Void());
}

Result<Void> FrameDecoder::decode_cobs()
{
    _buffer = cobs_decode(_buffer);
    _index = 0;
    if (_buffer.size() == 0)
    {
        return Result<Void>::Err(EINVAL, "COBS decode error");
    }
    return Result<Void>::Ok(Void());
}

Result<bool> FrameDecoder::add_byte(uint8_t byte)
{
    if (byte == 0)
    {
        return Result<bool>::Ok(true);
    }
    _buffer.push_back(byte);
    if (_buffer.size() > _max)
    {
        return Result<bool>::Err(ENOSPC, "Buffer overflow");
    }
    return Result<bool>::Ok(false);
}

Result<CborType> FrameDecoder::peek_type()
{
    auto r = peek_next();
    if (r.is_err())
    {
        return Result<CborType>::Err(EINVAL, "Buffer peek_next failed");
    }
    uint8_t header = r.unwrap();
    if ((header & 0xE0) == 0x00)
    {
        return Result<CborType>::Ok(CBOR_UINT32);
    }
    else if ((header & 0xE0) == 0x60)
    {
        return Result<CborType>::Ok(CBOR_STR);
    }
    else if ((header & 0xE0) == 0x40)
    {
        return Result<CborType>::Ok(CBOR_BSTR);
    }
    else if (header == 0xFA)
    {
        return Result<CborType>::Ok(CBOR_FLOAT);
    }
    else if (header == 0xFB)
    {
        return Result<CborType>::Ok(CBOR_DOUBLE);
    }
    else if ((header & 0xE0) == 0x20)
    {
        return Result<CborType>::Ok(CBOR_INT32);
    }
    else if (header == 0x9F)
    {
        return Result<CborType>::Ok(CBOR_ARRAY);
    }
    else if (header == 0xBF)
    {
        return Result<CborType>::Ok(CBOR_MAP);
    }
    else if (header == 0xFF)
    {
        return Result<CborType>::Ok(CBOR_END);
    }
    else if (header == 0xF6)
    {
        return Result<CborType>::Ok(CBOR_NULL);
    }
    else if (header == 0xF5 || header == 0xF4)
    {
        return Result<CborType>::Ok(CBOR_BOOL);
    }
    return Result<CborType>::Err(EINVAL, "Unknown CBOR type");
}

Result<Void> FrameDecoder::decode_array()
{
    auto r = peek_next();
    if (r.is_err())
    {
        return Result<Void>::Err(EINVAL, "Buffer is empty");
    }
    uint8_t header = r.unwrap();
    if (header != 0x9F)
    {
        return Result<Void>::Err(EINVAL, "Not a CBOR array");
    }
    read_next();
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::decode_map()
{
    auto r = peek_next();
    if (r.is_err())
    {
        return Result<Void>::Err(EINVAL, "Buffer is empty");
    }
    uint8_t header = r.unwrap();
    if (header != 0xBF)
    {
        return Result<Void>::Err(EINVAL, "Not a CBOR map");
    }
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::decode_end()
{
    auto r = peek_next();
    if (r.is_err())
    {
        return Result<Void>::Err(EINVAL, "Buffer is empty");
    }
    uint8_t header = r.unwrap();
    if (header != 0xFF)
    {
        return Result<Void>::Err(EINVAL, "Not a CBOR end");
    }
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::decode(std::string &str)
{
    auto cbor_type = CHECK_MAP(Void, peek_type());

    if (cbor_type != CBOR_STR)
    {
        return Result<Void>::Err(EINVAL, "Not a CBOR string");
    }
    size_t len = read_next().unwrap() & 0x1F;
    str = "";
    for (size_t i = 0; i < len; i++)
    {
        auto r = CHECK_MAP(Void, read_next());
        str += r;
    }
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::decode(ByteString &bstr)
{
    auto cbor_type = CHECK_MAP(Void, peek_type());
    if (cbor_type != CBOR_BSTR)
    {
        return Result<Void>::Err(EINVAL, "Not a CBOR string");
    }
    size_t len = read_next().unwrap() & 0x1F;
    bstr.clear();
    for (size_t i = 0; i < len; i++)
    {
        auto r = CHECK_MAP(Void, read_next());
        bstr.push_back(r);
    }
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::decode(int32_t &value)
{
    auto cbor_type = CHECK_MAP(Void, peek_type());
    if (cbor_type != CBOR_INT32)
    {
        return Result<Void>::Err(EINVAL, "Not a CBOR int32");
    }
    auto r2 = read_next();
    if (r2.is_err())
    {
        return Result<Void>::Err(EINVAL, "Buffer decode_int32 read_next fails");
    }
    uint8_t header = r2.unwrap();
    value = 0;
    if ((header & 0x20) == 0x20)
    {
        value = -(header & 0x1F) - 1;
    }
    else if (header == 0x38)
    {
        value = -(read_next().unwrap() + 1);
    }
    else if (header == 0x39)
    {
        value = -(read_next().unwrap() << 8) | read_next().unwrap();
    }
    else if (header == 0x3A)
    {
        value = -(read_next().unwrap() << 24) | (read_next().unwrap() << 16) | (read_next().unwrap() << 8) | read_next().unwrap();
    }
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::decode(uint32_t &value)
{
    auto cbor_type = CHECK_MAP(Void, peek_type());
    if (cbor_type != CBOR_UINT32)
    {
        return Result<Void>::Err(EINVAL, "Not a CBOR uint32");
    }
    auto r2 = read_next();
    if (r2.is_err())
    {
        return Result<Void>::Err(EINVAL, "Read_next fails in decode_uint32");
    }
    INFO("Read next byte : 0x%X", r2.unwrap());
    uint8_t header = r2.unwrap();
    value = 0;
    if ((header & 0x1F) <= 23)
    {
        value = header;
    }
    else if (header == 0x18)
    {
        value = read_next().unwrap();
    }
    else if (header == 0x19)
    {
        value = (read_next().unwrap() << 8) | read_next().unwrap();
    }
    else if (header == 0x1A)
    {
        value = (read_next().unwrap() << 24) | (read_next().unwrap() << 16) | (read_next().unwrap() << 8) | read_next().unwrap();
    }
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::decode(uint16_t &value)
{
    uint32_t v;
    auto r = CHECK(decode(v));
    value = v;
    return Result<Void>::Ok(Void());
}

template <typename T>
Result<Void> FrameDecoder::decode_opt(Option<T> &value)
{
    auto r = CHECK_MAP(Void, peek_type());
    if (r == CBOR_NULL)
    {
        value = Option<T>::None();
    }
    else
    {
        T v;
        auto r2 = CHECK(decode(v));
        value = Option<T>::Some(v);
    }
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::decode(float &value)
{
    auto cbor_type = CHECK_MAP(Void, peek_type());
    if (cbor_type != CBOR_FLOAT)
    {
        return Result<Void>::Err(EINVAL, "Not a CBOR float");
    }
    read_next(); // consume type byte
    value;
    uint32_t float_bits = 0;
    for (size_t i = 0; i < 4; i++)
    {
        auto r = read_next();
        if (r.is_err())
        {
            return Result<Void>::Err(EINVAL, "Buffer is empty");
        }
        float_bits = (float_bits << 8) | r.unwrap();
    }
    std::memcpy(&value, &float_bits, sizeof(float));
    return Result<Void>::Ok(Void());
}

Result<bool> FrameDecoder::check_crc()
{
    if (_buffer.size() < 2)
    {
        return Result<bool>::Err(ENOSPC, "Buffer too small for CRC");
    }
    uint16_t crc_received = (_buffer[_buffer.size() - 2] << 8) | _buffer[_buffer.size() - 1];
    uint16_t crc_calculated = crc16(_buffer.data(), _buffer.size() - 2);
    if (crc_received != crc_calculated)
    {
        return Result<bool>::Err(EFAULT, "CRC check failed");
    }
    return Result<bool>::Ok(true);
}

Result<Void> FrameDecoder::clear()
{
    _buffer.clear();
    _index = 0;
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::read_buffer(uint8_t *buf, size_t len)
{
    if (len < _buffer.size())
    {
        return Result<Void>::Err(ENOSPC);
    }
    std::memcpy(buf, _buffer.data(), _buffer.size());
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::read_buffer(std::vector<unsigned char> &buf)
{
    buf = _buffer;
    return Result<Void>::Ok(Void());
}

Result<Void> FrameDecoder::rewind()
{
    _index = 0;
    return Result<Void>::Ok(Void());
}

Result<Void> MsgHeader::encode(FrameEncoder &encoder)
{
    RET_ERR(encoder.encode_array());
    RET_ERR(encoder.encode(dst));
    RET_ERR(encoder.encode(src));
    RET_ERR(encoder.encode(msg_type));
    RET_ERR(encoder.encode(msg_id));
    return Result<Void>();
}
Result<MsgHeader> MsgHeader::decode(FrameDecoder &decoder)
{
    MsgHeader header;
    INFO("Decoding MsgHeader");
    CHECK_MAP(MsgHeader, decoder.decode_array());
    INFO("Decoded array");
    CHECK_MAP(MsgHeader, decoder.decode_opt<TopicId>(header.src));
    INFO("Decoded source");
    CHECK_MAP(MsgHeader, decoder.decode_opt<TopicId>(header.dst));
    INFO("Decoded destination");
    uint32_t type;
    CHECK_MAP(MsgHeader, decoder.decode(type));
    INFO("Decoded type");
    header.msg_type = (MsgType)type;
    CHECK_MAP(MsgHeader, decoder.decode_opt<MsgId>(header.msg_id));
    INFO("Decoded msg_id");
    return Result<MsgHeader>(header);
}

Result<Void> DescMsg::encode(FrameEncoder &encoder)
{
    DescMsg msg = DescMsg();
    RET_ERR(encoder.encode_array());
    RET_ERR(encoder.encode(prop_id));
    RET_ERR(encoder.encode(name.c_str()));
    RET_ERR(encoder.encode(desc));
    RET_ERR(encoder.encode(value_type));
    RET_ERR(encoder.encode(value_mode));
    return Result<Void>();
}
Result<DescMsg> DescMsg::decode(FrameDecoder &decoder)
{
    DescMsg msg;
    CHECK_MAP(DescMsg, decoder.decode_array());
    CHECK_MAP(DescMsg, decoder.decode_opt<uint8_t>(msg.prop_id));
    CHECK_MAP(DescMsg, decoder.decode(msg.name));
    CHECK_MAP(DescMsg, decoder.decode_opt<std::string>(msg.desc));
    CHECK_MAP(DescMsg, decoder.decode_opt<ValueType>(msg.value_type));
    CHECK_MAP(DescMsg, decoder.decode_opt<ValueMode>(msg.value_mode));
    return Result<DescMsg>(msg);
}
