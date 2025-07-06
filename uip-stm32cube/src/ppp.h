#ifndef PPP_H
#define PPP_H

#include <stdint.h>
#include <string.h>
// PPP Protocol Constants
#define PPP_FLAG 0x7E   // Flag sequence
#define PPP_ESCAPE 0x7D // Control escape
#define PPP_TRANS 0x20  // Asynchronous transparency modifier

// PPP Protocol Types
#define PPP_IP 0x0021   // Internet Protocol
#define PPP_LCP 0xC021  // Link Control Protocol
#define PPP_IPCP 0x8021 // IP Control Protocol

// LCP/IPCP Codes
#define PPP_CONFREQ 0x01 // Configuration Request
#define PPP_CONFACK 0x02 // Configuration Ack
#define PPP_TERMREQ 0x05 // Termination Request
#define PPP_TERMACK 0x06 // Termination Ack

// PPP States
enum ppp_state
{
    PPP_DEAD,
    PPP_ESTABLISH,
    PPP_NETWORK,
    PPP_TERMINATE
};

// PPP Frame Structure
struct ppp_frame
{
    uint8_t addr;   // Address field (always 0xFF)
    uint8_t ctrl;   // Control field (always 0x03)
    uint16_t proto; // Protocol field
    uint8_t data[]; // Information field
};



class PPP
{
private:
    uint8_t rx_buffer[1024];
    uint8_t tx_buffer[1024];
    uint16_t rx_index;
    uint16_t tx_index;
    bool escape_flag;
    ppp_state state;
    uint8_t req_id;
    uint32_t lcp_timer;
    uint8_t lcp_retries;

    // CRC-16 calculation for PPP
    uint16_t crc16(const uint8_t *data, uint16_t len);

    // Escape byte for PPP HDLC framing
    void escape_byte(uint8_t byte);

    // Send PPP frame with HDLC framing
    void send_frame(uint16_t proto, const uint8_t *data, uint16_t len);

    // Process LCP packet
    void process_lcp(const uint8_t *data, uint16_t len);
    // Send LCP Configuration Ack
    void send_lcp_ack(uint8_t id);
    // Send LCP Termination Ack
    void send_lcp_term_ack(uint8_t id);

    // Process IPCP packet
    void process_ipcp(const uint8_t *data, uint16_t len);

    // Send IPCP Configuration Ack
    void send_ipcp_ack(uint8_t id);

public:
    // Process received frame
    void process_frame();
public:
    PPP() : rx_index(0), tx_index(0), escape_flag(false), state(PPP_DEAD), req_id(1) {}

    void tick(uint32_t now_ms) ;
    void init();
    // Process received byte from serial port
    void process_byte(uint8_t byte);
    // Send IP packet through PPP
    void send_ip_packet(const uint8_t *packet, uint16_t len);

    // Get current PPP state
    ppp_state get_state() const;

    // Platform-specific functions to implement
    virtual void serial_send(const uint8_t *data, uint16_t len) = 0;
    virtual void uip_input_packet(const uint8_t *packet, uint16_t len) = 0;
};

#endif // PPP_H
