#ifndef ACTOR_H
#define ACTOR_H
#include <freertos/FreeRTOS.h>
#include <freertos/mpu_wrappers.h>
#include <freertos/portmacro.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <stddef.h>
#include <vector>
#include <functional>
#include <esp_timer.h>
#include <log.h>
#include <util.h>
#include <serdes.h>
#include <option.h>
#include <value.h>
#include <result.h>
#include <memory>
#include <stdint.h>
#pragma once

uint64_t current_time();

class ActorRef
{
    const char *_actor_name;

public:
    ActorRef() = delete;
    constexpr ActorRef(const char *name) : _actor_name(name) {};
    ActorRef(const ActorRef &other) : _actor_name(other._actor_name) {};
    bool operator==(ActorRef &other) { return _actor_name == other._actor_name; }; // matches same id
    void operator=(ActorRef other) { _actor_name = other._actor_name; }
    bool match_name(ActorRef &other) { return strcmp(_actor_name, other._actor_name) == 0; };
    inline const char *name() const { return _actor_name; };
};

ActorRef NULL_ACTOR = ActorRef("null");

class Msg
{
public:
    ActorRef src = ActorRef("null");
    ActorRef dst = ActorRef("null");
    virtual const char *type_id() const = 0;
    virtual ~Msg() = default;
    template <typename T>
    void handle(std::function<void(const T &)> f) const
    {
        if (type_id() == T::id)
        {
            const T &msg = static_cast<const T &>(*this);
            f(msg);
        }
    }
};

#define MSG(MSG_TYPE, ...)                                          \
    class MSG_TYPE : public Msg                                     \
    {                                                               \
    public:                                                         \
        static constexpr const char *id = STRINGIZE(MSG_TYPE);      \
        inline const char *type_id() const override { return id; }; \
        ~MSG_TYPE() = default;                                      \
        __VA_ARGS__;                                                \
    }

MSG(TimerMsg, int timer_id);

/*template <typename T>
void handle(const Msg &message, std::function<void(const T &)> f)
{
    if (message.type_id() == T::id)
    {
        T &msg = (T &)message;
        f(msg);
    }
}*/

template <class T>
class Queue
{
private:
    QueueHandle_t queue;
    size_t _depth;

public:
    Queue(size_t depth) : _depth(depth)
    {
        queue = xQueueCreate(depth, sizeof(T));
        INFO("Channel created [%d][%d]", depth, sizeof(T));
    }

    int getDepth() const { return _depth; }
    int getSize() const { return uxQueueMessagesWaiting(queue); }

    bool send(T qt, TickType_t timeout = portMAX_DELAY)
    {
        return xQueueSend(queue, &qt, timeout) == pdTRUE ? true : false;
    }

    bool sendFromIsr(T qt, TickType_t timeout = portMAX_DELAY)
    {
        return xQueueSendFromISR(queue, &qt, nullptr) == pdTRUE;
    }

    bool receive(T *qt, TickType_t timeout = portMAX_DELAY)
    {
        if (xQueueReceive(queue, qt, timeout) == pdTRUE)
        {
            return true;
        };
        return false;
    }
    size_t size() { return uxQueueMessagesWaiting(queue); }
    ~Queue() { vQueueDelete(queue); }
    QueueHandle_t getQueue() { return queue; }
    size_t getQueueDepth() { return _depth; }
};

// Queue Set wrapper
class QueueSet
{
private:
    QueueSetHandle_t set;

public:
    QueueSet(size_t maxQueues) { set = xQueueCreateSet(maxQueues); }

    inline void addQueue(QueueHandle_t queue)
    {
        if (xQueueAddToSet(queue, set) != pdTRUE)
        {
            ERROR("Failed to add queue to set");
        }
    }

    inline QueueHandle_t wait(TickType_t timeout = portMAX_DELAY)
    {
        return xQueueSelectFromSet(set, timeout);
    }

    inline ~QueueSet() { vQueueDelete(set); }
};

struct Timer
{
    bool _auto_reload;
    uint64_t _expires_at;
    uint64_t _period;
    bool _active = false;

public:
    Timer(bool auto_reload, bool active, uint64_t period,
          uint64_t expires_at);
    static Timer Repetitive(uint64_t period);
    static Timer OneShot(uint64_t delay);
    void make_one_shot(uint64_t delay);
    void make_repetitive(uint64_t period);
    bool is_expired(uint64_t now) const &;
    void refresh(uint64_t now);
    void start();
    inline void stop() { _active = false; }
    void reset();
    void fire(uint64_t delay);
};

void refresh_expired_timers(std::vector<Timer> &timers);

class Timers
{
    std::vector<Timer> _timers;

public:
    Timers() {}
    uint64_t get_next_expires_at();
    uint64_t sleep_time();
    std::vector<int> get_expired_timers();
    void refresh_expired_timers();
    void refresh(int id);
    int create_one_shot(uint64_t delay);
    int create_repetitive(uint64_t period);
    int start();
    void fire(int id, uint64_t delay);
    void stop(int id);
};

class ThreadSupport
{
public:
    virtual void on_start() = 0;
    virtual void on_stop() = 0;
    virtual QueueHandle_t queue_handle() = 0;
    Option<QueueHandle_t> additional_queue() { return Option<QueueHandle_t>::None(); }
    virtual uint64_t sleep_time() = 0;
    virtual void handle_expired_timers() = 0;
    virtual const char *name() = 0;
    virtual ~ThreadSupport() = default;
};


class Actor;

class EventBus : public Queue<Msg *>
{
    std::vector<Actor *> _actors;
    size_t _stack_size = 1024;
    TaskHandle_t _task_handle;

public:
    EventBus(size_t size);
    void push(Msg *msg);
    void register_actor(Actor *);
    void loop();
    void start();
};




class Actor
{
private:
    ActorRef _self;
    Timers _timers;
    EventBus *eventbus = nullptr;

public:
    virtual void on_start() { INFO("actor %s default started.", _self.name()); }
    virtual void on_stop() { INFO("actor %s default stopped.", _self.name()); }
    virtual void on_message(const Msg &message)
    {
        WARN(" No message handler for actor %s ", _self.name());
    }
    void emit(const Msg *msg);
    void set_eventbus(EventBus *eventbus);

    Actor(const char *name) : _self(name) {};

    ActorRef ref()
    {
        return _self;
    }

    uint64_t sleep_time() { return _timers.sleep_time(); }

    void handle_expired_timers()
    {
        for (int id : _timers.get_expired_timers())
        {
            TimerMsg timer_msg;
            timer_msg.timer_id = id;
            timer_msg.src = NULL_ACTOR;
            on_message(timer_msg);
            _timers.refresh(id);
        }
    };

    void emit(Msg &msg);

    const char *name() { return _self.name(); }

    ~Actor()
    {
        INFO("Destroying actor %s", name());
    }

    void loop() {};
    /*
        void loop()
        {
            INFO("starting actor %s", name());
            on_start();
            while (!_stop_actor)
            {
                MsgContext *msg_context;
                INFO("Actor %s waiting for command during %d", name(), _timers.sleep_time());
                if (_queue->receive(msg_context, _timers.sleep_time()))
                {
                    auto ref = ActorRef(msg_context->sender_queue);
                    on_message(ref, *msg_context->pmsg);
                    delete msg_context; // Clean up the command after processing
                }
                else
                {
                    for (int id : _timers.get_expired_timers())
                    {
                        TimerMsg timer_msg;
                        timer_msg.timer_id = id;
                        on_message(_self, timer_msg);
                        _timers.refresh(id);
                    }
                }
            }
            INFO("stopping actor %s", name());
            on_stop();
        }
    */
    void stop()
    {
    }
    int timer_one_shot(uint64_t delay)
    {
        return _timers.create_one_shot(delay);
    }
    int timer_repetitive(uint64_t period)
    {
        return _timers.create_repetitive(period);
    }
    void timer_stop(int id)
    {
        _timers.stop(id);
    }
    void timer_start(int id, uint64_t delay)
    {
        _timers.refresh(id);
    }
    void refresh(int id)
    {
        _timers.refresh(id);
    }
    void timer_fire(int id, uint64_t delay)
    {
        _timers.fire(id, delay);
    }
};

/*

A thread is created to manage multiple actors. The thread will wait for the actor with the lowest sleep time and handle the command or timer event.

*/

typedef enum Cpu
{
    CPU0,
    CPU1,
    CPU_ANY
} Cpu;

class Thread
{
    std::vector<ThreadSupport *> _actors;
    QueueSetHandle_t _queue_set;
    std::string _name;
    size_t _stack_size;
    TaskHandle_t _task_handle;
    bool _stop_thread = false;
    int _priority;
    Cpu _preferred_cpu;
    size_t _queue_set_size;

public:
    Thread(const char *name, size_t stack_size, size_t queue_set_size, int priority = 5, Cpu preferred_cpu = Cpu::CPU_ANY) : _name(name),
                                                                                                                             _stack_size(stack_size),
                                                                                                                             _priority(priority),
                                                                                                                             _preferred_cpu(preferred_cpu), _queue_set_size(queue_set_size)
    {
    }
    const char *name() { return _name.c_str(); }
    Res start();
    Res add_actor(ThreadSupport &actor);
    void run();
    void step();
    void handle_expired_timers();
};

typedef struct PropInfo
{
    const char *name;
    const char *type;
    const char *desc;
    const char *mode;
    Option<float> min;
    Option<float> max;
} PropInfo;

extern EventBus eventbus;


EventBus::EventBus(size_t size) : Queue<Msg *>(size) {};

void EventBus::push(Msg *msg)
{
    send(msg);
}

void EventBus::loop()
{
    Msg *pmsg;
    for (Actor *actor : _actors)
    {
        actor->on_start();
    }
    while (true)
    {
        if (receive(&pmsg, 100))
        {
            for (Actor *actor : _actors)
            {
                actor->on_message(*pmsg);
            }
            delete pmsg;
        }
    }
}

void EventBus::start()
{
    xTaskCreate(
        [](void *arg)
        {
            auto self = static_cast<EventBus *>(arg);
            self->loop();
        },
        "EventBus", _stack_size, this, 5, &_task_handle);
}

void EventBus::register_actor(Actor *actor)
{
    _actors.push_back(actor);
    actor->set_eventbus(this);
}


MSG(StopActorMsg);
MSG(PublishMsg, std::string topic; Value value; PublishMsg(const ActorRef &s, const std::string &t, const Value &v) : topic(t), value(v){src=s;});
MSG(SubscribeMsg, std::string topic);

#endif
