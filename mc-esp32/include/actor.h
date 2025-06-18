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

uint64_t current_time();

template <typename T>
class Channel
{
private:
    QueueHandle_t queue;
    size_t _depth;

public:
    Channel(size_t depth) : _depth(depth)
    {
        queue = xQueueCreate(depth, sizeof(T));
        INFO("Channel created [%d][%d]", depth, sizeof(T));
    }

    int getDepth() const { return _depth; }
    int getSize() const { return uxQueueMessagesWaiting(queue); }

    bool send(const T message, TickType_t timeout = portMAX_DELAY)
    {
        return xQueueSend(queue, &message, timeout) == pdTRUE ? true : false;
    }

    bool sendFromIsr(const T message, TickType_t timeout = portMAX_DELAY)
    {
        return xQueueSendFromISR(queue, &message, nullptr) == pdTRUE;
    }

    bool receive(T *message, TickType_t timeout = portMAX_DELAY)
    {
        return xQueueReceive(queue, message, timeout) == pdTRUE;
    }
    size_t size() { return uxQueueMessagesWaiting(queue); }
    ~Channel() { vQueueDelete(queue); }
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

class Actor : public ThreadSupport
{
private:
    std::vector<std::function<void(const Value &)>> _handlers;
    Channel<const Value *> _cmds;
    Timers _timers;
    bool _stop_actor = false;
    TaskHandle_t _task_handle;
    size_t _stack_size;
    int _priority;
    std::string _name;

public:
    virtual void on_cmd(const Value &cmd) = 0;
    virtual void on_timer(int id) = 0;

    virtual void on_start() {};
    virtual void on_stop() {};

    QueueHandle_t queue_handle() override { return _cmds.getQueue(); }
    uint64_t sleep_time() override { return _timers.sleep_time(); }
    void handle_all_cmd() override
    {
        const Value *cmd;
        if (_cmds.receive(&cmd, 0))
        {
 //           INFO("%s <= %s", name(), cmd->toJson().c_str());
            on_cmd(*cmd);
        }
    };
    void handle_expired_timers() override
    {
        for (int id : _timers.get_expired_timers())
        {
            on_timer(id);
            _timers.refresh(id);
        }
    };

    const char *name() { return _name.c_str(); }

    Actor(size_t stack_size, const char *name, int priority, size_t queue_depth) : _cmds(queue_depth), _stack_size(stack_size), _priority(priority), _name(name)
    {
    }

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
            const Value *pcmd;
            INFO("Actor %s waiting for command during %d", name(), _timers.sleep_time());
            if (_cmds.receive(&pcmd, _timers.sleep_time()))
            {
                INFO("Actor %s received command: %s", name(), pcmd->toJson().c_str());
                on_cmd(*pcmd);
                delete pcmd;
            }
            else
            {
                for (int id : _timers.get_expired_timers())
                {
                    on_timer(id);
                    _timers.refresh(id);
                }
            }
        }
        INFO("stopping actor %s", name());
        on_stop();
    }

    void emit(const Value &event)
    {
        for (auto &handler : _handlers)
            handler(event);
    }
    void on_event(std::function<void(const Value &)> handler)
    {
        _handlers.push_back(handler);
    }
    inline bool tell(const Value &msg)
    {
        Value *pmsg = new Value;
        *pmsg = std::move(msg);
        bool ok = _cmds.send(pmsg, pdMS_TO_TICKS(10));
        if (!ok)
        {
            ERROR("Failed to send message to actor %s [%d/%d]", name(), _cmds.getSize(), _cmds.getDepth());
            delete pmsg; // Clean up if send failed
        }
        return ok;
    }
    inline bool tellFromIsr(const Value &msg)
    {
        Value *pmsg = new Value;
        *pmsg = std::move(msg);
        bool ok =  _cmds.sendFromIsr(pmsg);
        if (!ok)
        {
            ERROR("Failed to send message to actor %s [%d/%d]", name(), _cmds.getSize(), _cmds.getDepth());
            delete pmsg; // Clean up if send failed
        }
        return ok;
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
                                                                                                                             _preferred_cpu(preferred_cpu),
                                                                                                                             _queue_set_size(queue_set_size)
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

#endif
