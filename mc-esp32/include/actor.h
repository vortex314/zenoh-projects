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
#pragma once

uint64_t current_time();

class Msg
{
public:
    virtual uint32_t type_id() const = 0;
    virtual ~Msg() = default;
};

#define MSG(MSG_TYPE, ...)                                   \
  class MSG_TYPE : public Msg                                \
  {                                                          \
  public:                                                    \
    static constexpr uint32_t id = H(STRINGIZE(MSG_TYPE));   \
    inline uint32_t type_id() const override { return id; }; \
    ~MSG_TYPE() = default;                                   \
    __VA_ARGS__;                                             \
  }

MSG(TimerMsg,int timer_id);

template <typename T>
void handle(const Msg &message, std::function<void(const T &)> f)
{
    if (message.type_id() == T::id)
    {
        T &msg = (T &)message;
        f(msg);
    }
}


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

    bool receive(T &qt, TickType_t timeout = portMAX_DELAY)
    {
        if (xQueueReceive(queue, &qt, timeout) == pdTRUE)
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
    virtual void handle_all_cmd() = 0;
    virtual void handle_expired_timers() = 0;
    virtual const char *name() = 0;
    virtual ~ThreadSupport() = default;
};

/*

Actor usage as independent threads

Create actor - Run actor


Actor actor = Actor(4000, "actor", 5, 10);
actor.on_event([](Event &event) {
    INFO("Event received");
});
actor.start();

The start method creates a thread dedicated to the actor. The actor will run in the thread until the stop method is called.
The base actor only passes pointers through the queue to the actor. The actor is responsible for deleting the message.

*/
typedef struct MsgContext
{
    std::shared_ptr<Queue<MsgContext *>> sender_queue;
    Msg *pmsg;

    MsgContext(std::shared_ptr<Queue<MsgContext *>> sender_queue, Msg *pmsg) : sender_queue(sender_queue), pmsg(pmsg) {};
} MsgContext;

class ActorRef
{
    std::shared_ptr<Queue<MsgContext *>> queue;

public:
    bool tell(ActorRef& me, Msg *ms)
    {
        auto v = new MsgContext(me.queue, ms);
        return queue->send(v);
    }
    ActorRef(std::shared_ptr<Queue<MsgContext *>> queue) : queue(queue) {}
};

MSG(AddListener,ActorRef listener);


class Actor : public ThreadSupport
{
private:
    std::shared_ptr<Queue<MsgContext *>> _queue;
    ActorRef _self;
    std::vector<ActorRef> _listeners;

    Timers _timers;
    bool _stop_actor = false;
    TaskHandle_t _task_handle;
    size_t _stack_size;
    int _priority;
    std::string _name;

public:
    virtual void on_start() { INFO("actor %s default started.", _name.c_str()); }
    virtual void on_stop() { INFO("actor %s default stopped.", _name.c_str()); }
    virtual void on_message(ActorRef &sender, const Msg &message)
    {
        WARN(" No message handler for actor %s ", _name.c_str());
    }

    Actor(size_t stack_size, const char *name, int priority, size_t queue_depth)
        : _queue(std::make_shared<Queue<MsgContext *>>(queue_depth)),
          _self(ActorRef(_queue)),
          _stack_size(stack_size),
          _priority(priority),
          _name(name)
    {
    }

    ActorRef ref()
    {
        return _self;
    }

    QueueHandle_t queue_handle() override { return _queue->getQueue(); }
    uint64_t sleep_time() override { return _timers.sleep_time(); }
    void handle_all_cmd() override
    {
        MsgContext *msg_context;
        if (_queue->receive(msg_context))
        {
            auto ref = ActorRef(msg_context->sender_queue);
            handle<AddListener>(*msg_context->pmsg,[&](const AddListener& msg ){
                _listeners.push_back(msg.listener);
            });
            on_message(ref, *msg_context->pmsg);
            delete msg_context->pmsg;
            delete msg_context; // Clean up the command after processing
        }
    };
    void handle_expired_timers() override
    {
        for (int id : _timers.get_expired_timers())
        {
            TimerMsg timer_msg;
            timer_msg.timer_id =id;
            on_message(_self, timer_msg);
            _timers.refresh(id);
        }
    };

    void emit(Msg& msg){
        for ( ActorRef listener : _listeners ){
            listener.tell(_self,&msg); //TODO will lead to double free
        }
    }

    const char *name() { return _name.c_str(); }

    Actor(size_t queue_depth) : Actor(1024, FILE_LINE_STR, 1, queue_depth) {}

    ~Actor()
    {
        INFO("Destroying actor %s", name());
        stop();
        vTaskDelay(1000);
        // xTaskDelete(_task_handle);
    }

    void start()
    {
        xTaskCreate(
            [](void *arg)
            {
                auto self = static_cast<Actor *>(arg);
                self->loop();
            },
            name(), _stack_size, this, 5, &_task_handle);
    }

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
                    timer_msg.timer_id=id;
                    on_message(_self, timer_msg);
                    _timers.refresh(id);
                }
            }
        }
        INFO("stopping actor %s", name());
        on_stop();
    }

    void stop()
    {
        _stop_actor = true;
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

typedef MsgContext *QueueType;

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
    void handle_all_cmd();
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

MSG(StopActorMsg);



#endif
