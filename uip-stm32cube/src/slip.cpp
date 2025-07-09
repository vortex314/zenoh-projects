#include <slip.h>

// Process received SLIP data and return complete packets
std::vector<std::vector<uint8_t>> SLIP::process_rx_data(const uint8_t *data, size_t length)
{
    std::vector<std::vector<uint8_t>> packets;
    static std::vector<uint8_t> current_packet;
    static bool escaping = false;

    for (size_t i = 0; i < length; i++)
    {
        uint8_t c = data[i];

        if (escaping)
        {
            escaping = false;
            switch (c)
            {
            case SLIP_ESC_END:
                c = SLIP_END;
                break;
            case SLIP_ESC_ESC:
                c = SLIP_ESC;
                break;
            default:
                // Protocol error, discard packet
                current_packet.clear();
                continue;
            }
            current_packet.push_back(c);
        }
        else if (c == SLIP_ESC)
        {
            escaping = true;
        }
        else if (c == SLIP_END)
        {
            if (!current_packet.empty())
            {
                packets.push_back(current_packet);
                current_packet.clear();
            }
        }
        else
        {
            current_packet.push_back(c);
        }
    }

    return packets;
}

// Encode a packet into SLIP format
std::vector<uint8_t> SLIP::encode_packet(const uint8_t *data, size_t length)
{
    std::vector<uint8_t> slip_data;
    slip_data.push_back(SLIP_END); // Start with END character

    for (size_t i = 0; i < length; i++)
    {
        uint8_t c = data[i];

        switch (c)
        {
        case SLIP_END:
            slip_data.push_back(SLIP_ESC);
            slip_data.push_back(SLIP_ESC_END);
            break;
        case SLIP_ESC:
            slip_data.push_back(SLIP_ESC);
            slip_data.push_back(SLIP_ESC_ESC);
            break;
        default:
            slip_data.push_back(c);
        }
    }

    slip_data.push_back(SLIP_END); // End with END character
    return slip_data;
}
