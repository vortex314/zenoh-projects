#include "Serial.h"


std::string bytes_to_hex(uint8_t *bytes, size_t length)
{
    std::string s;
    for (size_t i = 0; i < length; i++)
    {
        char buf[4];
        snprintf(buf, 4, "%02X ", bytes[i]);
        s += buf;
    }
    return s;
}
//=====================================================================================================
//=====================================================================================================


int HardwareSerial::begin(uint32_t baudrate)
{
   
    return 0;
}

int HardwareSerial::write(uint8_t *data, size_t length)
{
    
}

void HardwareSerial::isr_txd_done()
{
   
}

// split into PPP frames
void HardwareSerial::isr_rxd() // ISR !
{
  
}

int HardwareSerial::available()
{
    return _rxBuffer.size();
}

uint8_t HardwareSerial::read()
{
    return _rxBuffer.read();
}

int HardwareSerial::end()
{

    return 0;
}

int HardwareSerial::flush()
{
    return 0;
}
//=====================================================================================================
//=====================================================================================================
