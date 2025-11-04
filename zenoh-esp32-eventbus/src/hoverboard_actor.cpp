
#include <hoverboard_actor.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <esp_flash.h>

#define UART_PORT UART_NUM_2
#define UART_TX_PIN GPIO_NUM_17
#define UART_RX_PIN GPIO_NUM_16
#define UART_BAUD_RATE 115200
#define UART_BUF_SIZE 1024
#define COBS_SEPARATOR 0x00
const uint16_t START_FRAME = 0xABCD;

void uart_event_task(void *pvParameters);

class HbCmd
{
public:
    int speed;
    int steer;
    HbCmd(int s, int st) : speed(s), steer(st) {};

    uint16_t crc()
    {
        uint16_t crc = 0;
        crc = crc ^ (START_FRAME);
        crc = crc ^ static_cast<uint16_t>(steer);
        crc = crc ^ static_cast<uint16_t>(speed);
        return crc;
    }

    void add_crc(std::vector<uint8_t> &vec)
    {
        uint16_t crc_value = crc();
        vec.push_back(static_cast<uint8_t>(crc_value & 0xFF));
        vec.push_back(static_cast<uint8_t>((crc_value >> 8) & 0xFF));
    }

    Bytes encode()
    {
        std::vector<uint8_t> v;
        v.push_back(static_cast<uint8_t>(START_FRAME & 0xFF));
        v.push_back(static_cast<uint8_t>((START_FRAME >> 8) & 0xFF));
        v.push_back(static_cast<uint8_t>(steer & 0xFF));
        v.push_back(static_cast<uint8_t>((steer >> 8) & 0xFF));
        v.push_back(static_cast<uint8_t>(speed & 0xFF));
        v.push_back(static_cast<uint8_t>((speed >> 8) & 0xFF));
        add_crc(v);
        return Bytes(v.begin(), v.end());
    }
};

HoverboardActor::HoverboardActor(const char *name) : Actor(name)
{
    _timer_publish = timer_repetitive(5000);
    _timer_hb_alive = timer_repetitive(100);
    (void)init_uart().or_else([&](Error e)
                              {
        ERROR("Failed to initialize UART: %s", e.msg);
        return Result<bool>::Ok(false); });
}

#define ESP_ERROR_RET(x)                                                              \
    do                                                                                \
    {                                                                                 \
        esp_err_t rc = (x);                                                           \
        if (rc != 0)                                                                  \
        {                                                                             \
            return Result<bool>::Err(rc, "ESP error @" __FILE__ STRINGIZE(__LINE__)); \
        }                                                                             \
    } while (0)

Result<bool> HoverboardActor::init_uart()
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_RET(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_RET(uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_RET(uart_driver_install(UART_PORT, UART_BUF_SIZE * 2, UART_BUF_SIZE, 10, &uart_queue, 0));

    INFO("UART%d initialized at %d baud", UART_PORT, UART_BAUD_RATE);
    auto rc = xTaskCreate(uart_event_task, "uart_event_task", 4096, this, 12, &uart_task_handle);
    if (rc != pdPASS)
    {
        return Result<bool>::Err(-1, "Failed to create UART event task");
    }
    return Result<bool>::Ok(true);
}

void HoverboardActor::on_start()
{
    INFO("Starting HoverboardActor");
    emit(new ZenohSubscribe("src/time_server/clock/utc"));
}

/**
 * @brief Decode a COBS-encoded byte sequence.
 *
 * @param input The COBS-encoded input bytes.
 * @return The decoded bytes or an error.
 */

Result<Bytes> HoverboardActor::cobs_decode(const Bytes &input)
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
            return Result<Bytes>::Err(-1, "COBS decode error");
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
    return Result<Bytes>::Ok(output);
}

uint16_t crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            }
            else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
/*
 * @brief Check CRC of the input data.
 *
 * @param input The input bytes with CRC at the end.
 * @return The data without CRC if valid, or an error.
 */
Result<Bytes> HoverboardActor::check_crc(const Bytes &input)
{
    if (input.size() < 4)
    {
        return Result<Bytes>::Err(-1, "Data too short for CRC check");
    }
    uint16_t received_crc = static_cast<uint16_t>(input[input.size() - 1]) |
                            (static_cast<uint16_t>(input[input.size() - 2]) << 8);
    uint16_t calculated_crc = crc16(input.data(), input.size() - 2);
    if (received_crc != calculated_crc)
    {
        return Result<Bytes>::Err(-2, "CRC mismatch");
    }
    return Result<Bytes>::Ok(Bytes(input.begin(), input.end() - 2));
}

Result<HoverboardInfo *> HoverboardActor::parse_info_msg(const Bytes &input)
{
    // print hex buffer for debugging
    INFO("Parsing HoverboardInfo message (%d bytes):", input.size());
    for (size_t i = 0; i < input.size(); i++)
    {
        printf("%02X ", input[i]);
    }
    printf("\n");
    HoverboardInfo *info = HoverboardInfo::cbor_deserialize(input);
    if (info)
    {
        return Result<HoverboardInfo *>::Ok(info);
    }
    else
    {
        return Result<HoverboardInfo *>::Err(-2, "Failed to deserialize HoverboardInfo");
    }
}

HoverboardActor::~HoverboardActor()
{
    INFO("Destroying HoverboardActor");

    uart_driver_delete(UART_PORT);
    if (uart_task_handle != NULL)
    {
        vTaskDelete(uart_task_handle);
    }
}

void HoverboardActor::on_message(const Envelope &env)
{
    const Msg &msg = *env.msg;
    msg.handle<HoverboardCmd>([&](auto hb_cmd)
                              { write_uart(HbCmd(hb_cmd.speed ? *hb_cmd.speed : 0, hb_cmd.steer ? *hb_cmd.steer : 0).encode()); });
    msg.handle<TimerMsg>([&](const TimerMsg &msg)
                         { on_timer(msg.timer_id); });
    msg.handle<UartRxd>([&](const UartRxd &uart_rxd)
                        { handle_uart_bytes(uart_rxd.payload); });
}


void HoverboardActor::handle_uart_bytes(const Bytes &data)
{
    INFO("Processing UART RX data (%d bytes)", data.size());
    for (auto b : data)
    {
        if (b == COBS_SEPARATOR)
        {
            INFO("COBS frame received (%d bytes)", uart_read_buffer.size());
            (void)cobs_decode(uart_read_buffer)
                .and_then(check_crc)
                .and_then(parse_info_msg)
                .and_then([&](HoverboardInfo *info)
                          {
                            emit(info);
                            return Result<bool>::Ok(true); })
                .or_else([&](Error e)
                         {
                        WARN("Failed to process UART data: %s", e.msg);
                        return Result<bool>::Ok(false); });
            uart_read_buffer.clear();
        } else 
        {
            uart_read_buffer.push_back(b);
        }
    }
}

void HoverboardActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        // Publish hoverboard info request
    }
    else if (id == _timer_hb_alive)
    {
        write_uart(HbCmd(200, 0).encode());
    }
    else
    {
        INFO("Unknown timer id: %d", id);
    }
}


// task to handle UART events and make it events on the bus
void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t data[UART_BUF_SIZE];
    HoverboardActor *actor = static_cast<HoverboardActor *>(pvParameters);

    while (true)
    {
        INFO("UART event task waiting for data...");
        // Wait for UART event
        if (xQueueReceive(actor->uart_queue, (void *)&event, portMAX_DELAY))
        {
            INFO("UART event received: %d", event.type);
            switch (event.type)
            {
            case UART_DATA:
            {
                // Read the received data
                int len = uart_read_bytes(UART_PORT, data, event.size, pdMS_TO_TICKS(100));
                if (len > 0)
                {
                    data[len] = '\0'; // Null-terminate
                    INFO("Received: [%d]", len);
                    Bytes payload(data, data + len);
                    actor->emit(new UartRxd(payload));
                }
                break;
            }

            case UART_FIFO_OVF:
            {
                WARN("HW FIFO overflow");
                uart_flush_input(UART_PORT);
                xQueueReset(actor->uart_queue);
                break;
            }

            case UART_BUFFER_FULL:
            {
                WARN("Ring buffer full");
                uart_flush_input(UART_PORT);
                xQueueReset(actor->uart_queue);
                break;
            }

            case UART_PARITY_ERR:
            {
                WARN("Parity error");
                break;
            }

            case UART_FRAME_ERR:
            {
                WARN("Frame error");
                break;
            }

            case UART_BREAK:
            {
                WARN("UART break");
                break;
            }

            default:
            {
                WARN("UART event type: %d", event.type);
                break;
            }
            }
        }
    }
    vTaskDelete(NULL);
}

void HoverboardActor::write_uart(const Bytes &data)
{
    uart_write_bytes(UART_PORT, (const char *)data.data(), data.size());
}
