
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

void uart_event_task(void *pvParameters);

const uint16_t START_FRAME = 0xABCD;

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
    init_uart();
}

void HoverboardActor::init_uart()
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
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_BUF_SIZE * 2, UART_BUF_SIZE, 10, &uart_queue, 0));

    INFO("UART%d initialized at %d baud", UART_PORT, UART_BAUD_RATE);
    auto rc = xTaskCreate(uart_event_task, "uart_event_task", 4096, this, 12, &uart_task_handle);
    if (rc != pdPASS)
    {
        ERROR("Failed to create UART event task");
    }
}

void HoverboardActor::on_start()
{
    INFO("Starting HoverboardActor");
    emit(new ZenohSubscribe("src/time_server/clock/utc"));
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
                              { INFO("Received HoverboardCmd: speed=%d direction=%d", hb_cmd.speed, hb_cmd.steer); });
    msg.handle<TimerMsg>([&](const TimerMsg &msg)
                         { on_timer(msg.timer_id); });
    msg.handle<UartRxd>([&](const UartRxd &uart_rxd)
                        {
                            INFO("Processing UART RX data (%d bytes)", uart_rxd.payload.size());
                            // Process the received UART data here
                        });
}
void HoverboardActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        publish_info();
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

void HoverboardActor::publish_info()
{
    HoverboardInfo *hb_info = new HoverboardInfo();
    hb_info->temp = 37;
    emit(hb_info);
    _prop_counter++;
}

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
