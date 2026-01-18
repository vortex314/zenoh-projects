#ifndef _HOVERBOARD_ACTOR_H_
#define _HOVERBOARD_ACTOR_H_
#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <serdes.h>
#include <vector>
#include <option.h>
#include <ArduinoJson.h>
#include <msg.h>
#include <zenoh_actor.h>

DEFINE_MSG(UartRxd,
    Bytes payload;
    UartRxd(const Bytes &payload) : payload(payload){};
);


class HoverboardActor : public Actor
{
private:
    int _timer_publish = -1;
    int _timer_hb_alive = -1;
    int _prop_counter = 0;
    uint16_t _speed=0;
    uint16_t _steer=0;

public:
    QueueHandle_t uart_queue = NULL;
    TaskHandle_t uart_task_handle = NULL;
    static const int UART_PORT = UART_NUM_1;
    static const int UART_BUF_SIZE = 1024;
    Bytes uart_read_buffer;

public:
    HoverboardActor(const char *name);
    ~HoverboardActor();
    void on_message(const Envelope &msg);
    void on_timer(int timer_id);
    void on_start();
    Result<bool> init_uart();
    void write_uart(const Bytes &);
    void handle_uart_bytes(const Bytes &);
    static Result<Bytes> cobs_decode(const Bytes &input);
    static Result<Bytes> check_crc(const Bytes &input);
    static Result<HoverboardEventRaw*> parse_info_msg(const Bytes &input);
};

#endif
