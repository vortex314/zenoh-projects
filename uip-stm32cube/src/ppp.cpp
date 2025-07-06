
#include <common.h> // Include common definitions

// PPP FCS polynomial (0x8408 is bit-reversed 0x1021)
#define PPP_FCS_POLY    0x8408
#define PPP_FCS_INIT    0xFFFF
#define PPP_FCS_GOOD    0xF0B8  // Good FCS value when checking
static uint16_t fcs_table[256];
static int table_initialized = 0;

/*
 * Initialize the FCS lookup table
 */
void fcs_init_table(void)
{
    uint16_t crc;
    int i, j;
    
    for (i = 0; i < 256; i++) {
        crc = i;
        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ PPP_FCS_POLY;
            } else {
                crc >>= 1;
            }
        }
        fcs_table[i] = crc;
    }
    table_initialized = 1;
}

volatile bool ppp_frame_ready = false; // Flag to indicate a PPP frame is ready

// CRC-16 calculation for PPP
uint16_t ppp_fcs_calculate(const uint8_t *data, uint16_t len)
{
    uint16_t fcs = PPP_FCS_INIT;
    uint16_t i;
    
    if (!table_initialized) {
        fcs_init_table();
    }
    
    for (i = 0; i < len; i++) {
        fcs = (fcs >> 8) ^ fcs_table[(fcs ^ data[i]) & 0xFF];
    }
    
    return fcs ^ 0xFFFF;  // Final XOR
}

// Escape byte for PPP HDLC framing
void PPP::escape_byte(uint8_t byte)
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

u8 buffer[1024]; // Buffer for received data

// Send PPP frame with HDLC framing
void PPP::send_frame(uint16_t proto, const uint8_t *data, uint16_t len)
{
    buffer[0] = 0xFF; // Address field (always 0xFF)
    buffer[1] = 0x03; // Control field (always 0x03)
    // Protocol field (big-endian)
    buffer[2] = (uint8_t)(proto >> 8);
    buffer[3] = (uint8_t)(proto & 0xFF);
    // Copy data to buffer
    memcpy(buffer + 4, data, len);
    // Calculate CRC-16
    if (!table_initialized) {
        fcs_init_table();
    }
    ppp_fcs_calculate(buffer, len + 4); // Include address, control, and protocol in CRC calculation
    uint16_t crc = ppp_fcs_calculate(buffer, len + 4);
    // Check for maximum data length

    if (len > 1020) // Check for maximum data length
    {
        len = 1020; // Limit to maximum length
    }
    if (len < 4) // Minimum length for PPP frame
    {
        return; // Invalid length, do not send
    }
  
    tx_index = 0;

    // Start flag
    tx_buffer[tx_index++] = PPP_FLAG;

    // Escape header
    for (int i = 0; i < len+4; i++) {
        escape_byte(buffer[i]);
    }
    
    // Escape CRC (little-endian)
    escape_byte(crc & 0xFF);
    escape_byte(crc >> 8);

    // End flag
    tx_buffer[tx_index++] = PPP_FLAG;

    // Send to serial port (implement this based on your platform)
    serial_send(tx_buffer, tx_index);
}

// Process LCP packet
void PPP::process_lcp(const uint8_t *data, uint16_t len)
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
void PPP::send_lcp_ack(uint8_t id)
{
    uint8_t lcp_ack[] = {PPP_CONFACK, id, 0x00, 0x04};
    send_frame(PPP_LCP, lcp_ack, sizeof(lcp_ack));
}

// Send LCP Termination Ack
void PPP::send_lcp_term_ack(uint8_t id)
{
    uint8_t lcp_term[] = {PPP_TERMACK, id, 0x00, 0x04};
    send_frame(PPP_LCP, lcp_term, sizeof(lcp_term));
}

// Process IPCP packet
void PPP::process_ipcp(const uint8_t *data, uint16_t len)
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
void PPP::send_ipcp_ack(uint8_t id)
{
    uint8_t ipcp_ack[] = {PPP_CONFACK, id, 0x00, 0x04};
    send_frame(PPP_IPCP, ipcp_ack, sizeof(ipcp_ack));
}

// Process received frame
void PPP::process_frame()
{
    if (rx_index < 6)
        return; // Minimum frame size

    // CRC check
    uint16_t received_crc = rx_buffer[rx_index - 2] | (rx_buffer[rx_index - 1] << 8);
    uint16_t calc_crc = ppp_fcs_calculate(rx_buffer, rx_index - 2);
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

// PPP::PPP() : rx_index(0), tx_index(0), escape_flag(false), state(PPP_DEAD), req_id(1) {}
void PPP::tick(uint32_t now_ms)
{
    // Example: retransmit LCP request every 3 seconds, max 5 tries
    if (state == PPP_ESTABLISH && now_ms - lcp_timer > 3000 && lcp_retries < 5000)
    {
        uint8_t lcp_req[] = {PPP_CONFREQ, req_id++, 0x00, 0x04};
        send_frame(PPP_LCP, lcp_req, sizeof(lcp_req));
        lcp_timer = now_ms;
        lcp_retries++;
    }
    // Add more timer-based logic as needed
}
// Initialize PPP connection
void PPP::init()
{
    state = PPP_ESTABLISH;
    req_id = 1;

    // Send LCP Configuration Request
    uint8_t lcp_req[] = {PPP_CONFREQ, req_id++, 0x00, 0x04};
    send_frame(PPP_LCP, lcp_req, sizeof(lcp_req));
}

// Process received byte from serial port
void PPP::process_byte(uint8_t byte)
{
    if (byte == PPP_FLAG)
    {
        if (rx_index > 0)
        {
            // End of frame
            ppp_frame_ready = true; // Signal main loop to process frame
            process_frame();
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
void PPP::send_ip_packet(const uint8_t *packet, uint16_t len)
{
    if (state == PPP_NETWORK)
    {
        send_frame(PPP_IP, packet, len);
    }
}

// Get current PPP state
ppp_state PPP::get_state() const
{
    return state;
}
