#ifndef PPP_H
#define PPP_H

#include <common.h> // Include common definitions

volatile bool ppp_frame_ready = false; // Flag to indicate a PPP frame is ready

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

    // CRC-16 calculation for PPP
    uint16_t crc16(const uint8_t *data, uint16_t len)
    {
        uint16_t crc = 0xFFFF;
        for (uint16_t i = 0; i < len; i++)
        {
            crc ^= data[i];
            for (int j = 0; j < 8; j++)
            {
                if (crc & 1)
                {
                    crc = (crc >> 1) ^ 0x8408;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
        return ~crc;
    }

    // Escape byte for PPP HDLC framing
    void escape_byte(uint8_t byte)
    {
        if (byte == PPP_FLAG || byte == PPP_ESCAPE || byte < 0x20)
        {
            tx_buffer[tx_index++] = PPP_ESCAPE;
            tx_buffer[tx_index++] = byte ^ PPP_TRANS;
        }
        else
        {
            tx_buffer[tx_index++] = byte;
        }
    }

public:
    // Send PPP frame with HDLC framing
    void send_frame(uint16_t proto, const uint8_t *data, uint16_t len)
    {
        tx_index = 0;

        // Start flag
        tx_buffer[tx_index++] = PPP_FLAG;

        // Address and Control
        escape_byte(0xFF);
        escape_byte(0x03);

        // Protocol
        escape_byte(proto >> 8);
        escape_byte(proto & 0xFF);

        // Data
        for (uint16_t i = 0; i < len; i++)
        {
            escape_byte(data[i]);
        }

        // CRC
        uint16_t crc = crc16(tx_buffer + 1, tx_index - 1);
        escape_byte(crc & 0xFF);
        escape_byte(crc >> 8);

        // End flag
        tx_buffer[tx_index++] = PPP_FLAG;

        // Send to serial port (implement this based on your platform)
        serial_send(tx_buffer, tx_index);
    }

    // Process LCP packet
    void process_lcp(const uint8_t *data, uint16_t len)
    {
        if (len < 4)
            return;

        uint8_t code = data[0];
        uint8_t id = data[1];

        switch (code)
        {
        case PPP_CONFREQ:
            // Send Configuration Ack
            send_lcp_ack(id);
            if (state == PPP_ESTABLISH)
            {
                state = PPP_NETWORK;
            }
            break;

        case PPP_CONFACK:
            if (state == PPP_ESTABLISH)
            {
                state = PPP_NETWORK;
            }
            break;

        case PPP_TERMREQ:
            send_lcp_term_ack(id);
            state = PPP_DEAD;
            break;
        }
    }

    // Send LCP Configuration Ack
    void send_lcp_ack(uint8_t id)
    {
        uint8_t lcp_ack[] = {PPP_CONFACK, id, 0x00, 0x04};
        send_frame(PPP_LCP, lcp_ack, sizeof(lcp_ack));
    }

    // Send LCP Termination Ack
    void send_lcp_term_ack(uint8_t id)
    {
        uint8_t lcp_term[] = {PPP_TERMACK, id, 0x00, 0x04};
        send_frame(PPP_LCP, lcp_term, sizeof(lcp_term));
    }

    // Process IPCP packet
    void process_ipcp(const uint8_t *data, uint16_t len)
    {
        if (len < 4)
            return;

        uint8_t code = data[0];
        uint8_t id = data[1];

        switch (code)
        {
        case PPP_CONFREQ:
            // Send Configuration Ack (simplified)
            send_ipcp_ack(id);
            break;
        }
    }

    // Send IPCP Configuration Ack
    void send_ipcp_ack(uint8_t id)
    {
        uint8_t ipcp_ack[] = {PPP_CONFACK, id, 0x00, 0x04};
        send_frame(PPP_IPCP, ipcp_ack, sizeof(ipcp_ack));
    }

public:
    // Process received frame
    void process_frame()
    {
        if (rx_index < 6)
            return; // Minimum frame size

        // CRC check
        uint16_t received_crc = rx_buffer[rx_index - 2] | (rx_buffer[rx_index - 1] << 8);
        uint16_t calc_crc = crc16(rx_buffer, rx_index - 2);
        if (calc_crc != received_crc)
        {
            // CRC error, discard frame
            return;
        }
        // Extract protocol
        uint16_t proto = (rx_buffer[2] << 8) | rx_buffer[3];
        uint8_t *data = rx_buffer + 4;
        uint16_t data_len = rx_index - 6; // Exclude addr, ctrl, proto, and CRC

        switch (proto)
        {
        case PPP_LCP:
            process_lcp(data, data_len);
            break;

        case PPP_IPCP:
            process_ipcp(data, data_len);
            break;

        case PPP_IP:
            if (state == PPP_NETWORK)
            {
                // Forward IP packet to uIP
                uip_input_packet(data, data_len);
            }
            break;
        }
    }

public:
    PPP() : rx_index(0), tx_index(0), escape_flag(false), state(PPP_DEAD), req_id(1) {}

    // Initialize PPP connection
    void init()
    {
        state = PPP_ESTABLISH;
        req_id = 1;

        // Send LCP Configuration Request
        uint8_t lcp_req[] = {PPP_CONFREQ, req_id++, 0x00, 0x04};
        send_frame(PPP_LCP, lcp_req, sizeof(lcp_req));
    }

    // Process received byte from serial port
    void process_byte(uint8_t byte)
    {
        if (byte == PPP_FLAG)
        {
            if (rx_index > 0)
            {
                // End of frame
                ppp_frame_ready = true; // Signal main loop to process frame
            }
            rx_index = 0;
            escape_flag = false;
            return;
        }

        if (byte == PPP_ESCAPE)
        {
            escape_flag = true;
            return;
        }

        if (escape_flag)
        {
            byte ^= PPP_TRANS;
            escape_flag = false;
        }

        if (rx_index < sizeof(rx_buffer))
        {
            rx_buffer[rx_index++] = byte;
        }
    }

    // Send IP packet through PPP
    void send_ip_packet(const uint8_t *packet, uint16_t len)
    {
        if (state == PPP_NETWORK)
        {
            send_frame(PPP_IP, packet, len);
        }
    }

    // Get current PPP state
    ppp_state get_state() const
    {
        return state;
    }

    // Platform-specific functions to implement
    virtual void serial_send(const uint8_t *data, uint16_t len) = 0;
    virtual void uip_input_packet(const uint8_t *packet, uint16_t len) = 0;
};

#endif // PPP_H

// Usage example:
/*
PPP_Serial ppp;

void setup() {
    ppp.init();
}

void loop() {
    // Process incoming serial data
    if (serial_available()) {
        uint8_t byte = serial_read();
        ppp.process_byte(byte);
    }

    // Send IP packet when needed
    if (ppp.get_state() == PPP_NETWORK && have_ip_packet()) {
        ppp.send_ip_packet(ip_packet, ip_packet_len);
    }
}
*/