#include "CircBuf.h"
// attentions : this is a single producer, single consumer circular buffer,
// totally NOT thread-safe
// start[writePos] => location of next write
// start[readPos] => location of next read
// positions are increased after operation
// readPos==writePos => empty
// writePos-readPos== limit ==> full

CircBuf::CircBuf(uint32_t size)
{
    start = new uint8_t[size];
    clear();
    limit = size;
}

void CircBuf::clear()
{
    readPos = 0;
    writePos = 0;
}

CircBuf::~CircBuf() { delete[] start; }

int CircBuf::write(uint8_t *b, uint32_t len)
{
    if (space() >= len)
    {
        for (uint32_t i = 0; i < len; i++)
        {
            start[writePos++ % limit] = b[i];
        }
        return 0;
    }
    return -ENOSPC;
}

int CircBuf::write(uint8_t b)
{
    if (space() > 1)
    {
        start[writePos++ % limit] = b;
        return 0;
    }
    return -ENOSPC;
}

int CircBuf::read()
{
    if (hasData())
        return start[readPos++ % limit];
    PANIC("CircBuf underflow");
    return -ENODATA;
}

uint32_t CircBuf::size() { return writePos - readPos; }

uint32_t CircBuf::space() { return limit - size(); }

bool CircBuf::hasSpace(uint32_t sz = 1) { return space() >= sz; }

bool CircBuf::hasData() { return size() > 0; }