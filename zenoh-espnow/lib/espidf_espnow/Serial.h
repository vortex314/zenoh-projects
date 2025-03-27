#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <errno.h>
#include <stdint.h>

#include <util.h>
#include <sys.h>
#include <CircBuf.h>


#define COBS_SEPARATOR 0x00
#define FRAME_MAX 256

class Serial
{
public:
    virtual int begin(uint32_t baudrate) = 0;
    virtual int end() = 0;
    virtual int flush() = 0;
    virtual int write(uint8_t *, size_t) = 0;
    virtual int available() = 0;
    virtual uint8_t read() = 0;
};

class HardwareSerial : public Serial
{
private:
    uint32_t _usart;
    uint32_t _txdOverflow = 0;
    CircBuf _rxBuffer;
    CircBuf _txBuffer;

public:
    HardwareSerial(uint32_t usart) : _usart(usart) , _rxBuffer(512), _txBuffer(512)  {}
    int begin(uint32_t baudrate);
    int end();
    int flush();
    int write(uint8_t *, size_t);
    int available();
    uint8_t read();

    void isr_txd_done();
    void isr_rxd();

    uint32_t _rxdOverflow = 0;

};


std::string bytes_to_hex(uint8_t *bytes, size_t length);

#endif /* _SERIAL_H_ */