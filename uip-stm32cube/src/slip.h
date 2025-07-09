#include <cstdint>
#include <vector>

    static constexpr uint8_t SLIP_END      = 0xC0;
    static constexpr uint8_t SLIP_ESC      = 0xDB;
    static constexpr uint8_t SLIP_ESC_END  = 0xDC;
    static constexpr uint8_t SLIP_ESC_ESC  = 0xDD;

class SLIP {
public:
    // SLIP special character codes


    static void serial_write(const uint8_t* data, size_t length); // Platform-specific serial write function
    static void uip_input(const uint8_t* packet, size_t length); // Platform-specific uIP input function

    // Process received SLIP data and return complete packets
    static std::vector<std::vector<uint8_t>> process_rx_data(const uint8_t* data, size_t length);

    // Encode a packet into SLIP format
    static std::vector<uint8_t> encode_packet(const uint8_t* data, size_t length) ;


};