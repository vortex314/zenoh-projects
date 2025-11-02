#ifndef _HOVERBOARD_ACTOR_H_
#define _HOVERBOARD_ACTOR_H_
#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <serdes.h>
#include <vector>
#include <option.h>
#include <ArduinoJson.h>
#include <limero.cpp>
#include <zenoh_actor.h>

DEFINE_MSG(UartRxd,
    Bytes payload;
    UartRxd(const Bytes &payload) : payload(payload){};
);

DEFINE_MSG(UartTxd,
    Bytes payload;
    UartTxd(const Bytes &payload) : payload(payload){};
);

class HoverboardActor : public Actor
{
private:
    int _timer_publish = -1;
    int _prop_counter = 0;

public:
    QueueHandle_t uart_queue = NULL;
    TaskHandle_t uart_task_handle = NULL;
    static const int UART_PORT = UART_NUM_1;
    static const int UART_BUF_SIZE = 1024;

public:
    HoverboardActor(const char *name);
    ~HoverboardActor();
    void on_message(const Envelope &msg);
    void on_timer(int timer_id);
    void on_start();
    void publish_info();
    void init_uart();
    void write_uart(const Bytes &);
    void read_uart(Bytes &);
    void process_uart();
};

#endif
