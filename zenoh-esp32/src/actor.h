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

template <typename T>
class Channel
{
    size_t _depth;
public:
    Channel(size_t depth)
    {
        queue = xQueueCreate(depth, sizeof(T));
        _depth = depth;
        INFO("Channel created [%d][%d]  ", depth, sizeof(T));
    }

    bool send(const T message, TickType_t timeout = portMAX_DELAY)
    {
        return xQueueSend(queue, &message, timeout) == pdTRUE;
    }

    bool receive(T *message, TickType_t timeout = portMAX_DELAY)
    {
        return xQueueReceive(queue, message, timeout) == pdTRUE;
    }
    size_t size() { return uxQueueMessagesWaiting(queue); }

    ~Channel() { vQueueDelete(queue); }

    QueueHandle_t getQueue() { return queue; }
    size_t getQueueDepth() { return _depth; }

private:
    QueueHandle_t queue;
};

// Queue Set wrapper
class QueueSet
{
public:
    QueueSet(size_t maxQueues) { set = xQueueCreateSet(maxQueues); }

    void addQueue(QueueHandle_t queue) { xQueueAddToSet(queue, set); }

    QueueHandle_t wait(TickType_t timeout = portMAX_DELAY)
    {
        return xQueueSelectFromSet(set, timeout);
    }

    ~QueueSet() { vQueueDelete(set); }

private:
    QueueSetHandle_t set;
};

struct Timer
{
    bool _auto_reload;
    uint64_t _expires_at;
    uint64_t _period;
    bool _active = false;
    int _id;

public:
    Timer(int id, bool auto_reload, bool active, uint64_t period,
          uint64_t expires_at)
    {
        _auto_reload = auto_reload;
        _active = active;
        _period = period;
        _expires_at = expires_at;
        _id = id;
    }

    static uint64_t now() { return esp_timer_get_time() / 1000; }

    static Timer Repetitive(int id, uint64_t period)
    {
        return Timer(id, true, true, period, now() + period);
    }

    static Timer OneShot(int id, uint64_t delay)
    {
        return Timer(id, false, true, 0, now() + delay);
    }

    void make_one_shot(uint64_t delay)
    {
        _active = true;
        _auto_reload = false;
        _period = 0;
        _expires_at = now() + delay;
    }

    void make_repetitive(uint64_t period)
    {
        _active = true;
        _auto_reload = true;
        _period = period;
        _expires_at = now() + period;
    }

    bool is_expired(uint64_t now) const & { return now >= _expires_at; }

    void update(uint64_t now)
    {
        if (_active)
        {
            if (_auto_reload && _period > 0)
            {
                _expires_at = now + _period;
            }
            else
            {
                if (now > _expires_at)
                {
                    _active = false;
                }
            }
        }
    }
    void start()
    {
        _active = true;
        _expires_at = now() + _period;
    }

    void stop() { _active = false; }

    void reset() { _expires_at = now() + _period; }

    int id() const & { return _id; }
};

class Timers
{
public:
    std::vector<Timer> timers;
    Timers() {}
    void add_timer(Timer timer) { timers.push_back(timer); }
    void remove_timer(int id)
    {
        for (int i = 0; i < timers.size(); i++)
        {
            if (timers[i].id() == id)
            {
                timers.erase(timers.begin() + i);
                break;
            }
        }
    }

    uint64_t get_next_expires_at()
    {
        uint64_t expires_at = UINT64_MAX;
        for (const Timer &timer : timers)
        {
            if (timer._active && timer._expires_at < expires_at)
            {
                expires_at = timer._expires_at;
            }
        }
        return expires_at;
    }

    uint64_t sleep_time()
    {
        uint64_t expires_at = get_next_expires_at();
        uint64_t now = Timer::now();
        if (expires_at <= now)
        {
            return 0;
        }
        return expires_at - now;
    }

    std::vector<int> get_expired_timers()
    {
        uint64_t now = Timer::now();
        std::vector<int> expired_timers;
        for (const Timer &timer : timers)
        {
            if (timer.is_expired(now))
            {
                expired_timers.push_back(timer.id());
            }
        }
        return expired_timers;
    }

    void update()
    {
        uint64_t now = Timer::now();
        for (Timer &timer : timers)
        {
            timer.update(now);
        }
    }

    void one_shot(int id, uint64_t delay)
    {
        for (Timer &timer : timers)
        {
            if (timer.id() == id)
            {
                timer.make_one_shot(delay);
                return;
            }
        }
        timers.push_back(Timer::OneShot(id, delay));
    }
    void repetitive(int id, uint64_t period)
    {
        for (Timer &timer : timers)
        {
            if (timer.id() == id)
            {
                timer.make_repetitive(period);
                return;
            }
        }
        timers.push_back(Timer::Repetitive(id, period));
    }
    void stop(int id)
    {
        for (Timer &timer : timers)
        {
            if (timer.id() == id)
            {
                timer.stop();
                return;
            }
        }
    }
};

class ThreadSupport
{
public:
    virtual void on_start() = 0;
    virtual void on_stop() = 0;
    virtual QueueHandle_t queue_handle() = 0;
    virtual uint64_t sleep_time() = 0;
    virtual void handle_cmd() = 0;
    virtual void handle_timers() = 0;
    virtual const char *name() = 0;
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


*/

template <typename EVENT, typename CMD>
class Actor : public ThreadSupport
{
private:
    std::vector<std::function<void(EVENT &)>> _handlers;
    Channel<CMD *> _cmds;
    Timers _timers;
    bool _stop_actor = false;
    TaskHandle_t _task_handle;
    size_t _stack_size;
    int _priority;
    const char *_name = "no_name";

public:
    virtual void on_cmd(CMD &cmd) = 0;
    virtual void on_timer(int id) = 0;

    virtual void on_start() {};
    virtual void on_stop() {};

    QueueHandle_t queue_handle() override { return _cmds.getQueue(); }
    uint64_t sleep_time() override { return _timers.sleep_time(); }
    void handle_cmd() override
    {
        CMD *cmd;
        if  (_cmds.receive(&cmd, 0))
        {
            on_cmd(*cmd);
            delete cmd;
        } else {
            ERROR("Failed to receive command for actor %s", _name);
        }
    };
    void handle_timers() override
    {
        for (int id : _timers.get_expired_timers())
            on_timer(id);
        _timers.update();
    };

    const char *name() { return _name; }

    Actor(size_t stack_size, const char *name, int priority, size_t queue_depth) : _cmds(queue_depth), _stack_size(stack_size), _priority(priority), _name(name)
    {
    }

    ~Actor()
    {
        stop();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
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
            _name, _stack_size, this, 5, &_task_handle);
    }

    void loop()
    {
        INFO("starting actor %s", _name);
        on_start();
        CMD *cmd;
        while (!_stop_actor)
        {
            if (_cmds.receive(&cmd, _timers.sleep_time()))
            {
                on_cmd(*cmd);
                delete cmd;
            }
            else
            {
                for (int id : _timers.get_expired_timers())
                    on_timer(id);
                _timers.update();
            }
        }
        INFO("stopping actor %s", _name);
        on_stop();
    }

    void emit(EVENT event)
    {
        for (auto &handler : _handlers)
            handler(event);
    }
    void on_event(std::function<void(EVENT &)> handler)
    {
        _handlers.push_back(handler);
    }
    bool tell(CMD *msg)
    {
        return _cmds.send(msg);
    }
    void add_timer(Timer timer)
    {
        _timers.add_timer(timer);
    }
    void stop()
    {
        _stop_actor = true;
    }
    void timer_one_shot(int id, uint64_t delay)
    {
        _timers.one_shot(id, delay);
    }
    void timer_repetitive(int id, uint64_t period)
    {
        _timers.repetitive(id, period);
    }
    void timer_stop(int id)
    {
        _timers.stop(id);
    }
};

/*

A thread is created to manage multiple actors. The thread will wait for the actor with the lowest sleep time and handle the command or timer event.

*/

class Thread
{
    std::vector<ThreadSupport *> _actors;
    QueueSetHandle_t _queue_set;
    std::string _name;
    size_t _stack_size;
    TaskHandle_t _task_handle;
    bool _stop_thread = false;

public:
    Thread(const char *name, size_t stack_size, size_t queue_size) : _name(name), _stack_size(stack_size)
    {
        _queue_set = xQueueCreateSet(queue_size);
    }

    Res start()
    {
        CHECK(xTaskCreate(
            [](void *arg)
            {
                auto self = static_cast<Thread *>(arg);
                self->loop();
            },
            _name.c_str(), _stack_size, this, 5, &_task_handle));
        return Res::Ok();
    }

    Res add_actor(ThreadSupport &actor)
    {
        _actors.push_back(&actor);
        auto r = xQueueAddToSet(actor.queue_handle(), _queue_set);
        if (r != pdPASS)
        {
            return Res::Err(0, "Failed to add actor to queue set");
        }
        return Res::Ok();
    }

    void loop()
    {
        uint32_t loop_count = 0;
        INFO("starting actor %s", _name);
        for (int i = 0; i < _actors.size(); i++)
        {
            auto &actor = *_actors[i];
            actor.on_start();
        };

        while (!_stop_thread)
        {
            // find lowest sleep time
            uint64_t sleep_time = UINT64_MAX;
            for (int i = 0; i < _actors.size(); i++)
            {
                auto &actor = *_actors[i];

                loop_count = 0;
                int st = 0;
                while (st == 0)
                {
                    actor.handle_timers();
                    st = actor.sleep_time();
                    loop_count++;
                    if (loop_count > 10)
                    {
                        ERROR("loop count exceeded for timer handling %s", actor.name());
                        break;
                    }
                }
                if (st < sleep_time)
                {
                    sleep_time = st;
                }
            };
            QueueSetMemberHandle_t queue = xQueueSelectFromSet(_queue_set, sleep_time);
            for (int i = 0; i < _actors.size(); i++)
            {
                auto &actor = *_actors[i];
                if (queue != NULL && queue == actor.queue_handle())
                {
                    actor.handle_cmd();
                }
                actor.handle_timers();
            };
        }
        INFO("stopping actor %s", _name);
        for (int i = 0; i < _actors.size(); i++)
        {
            auto &actor = *_actors[i];
            actor.on_stop();
        };
    }
};

// when receiving a message, the actor will call the on_cmd method with envelope
struct PublishBytes
{
    std::string topic;
    Bytes payload;
};
struct PublishSerdes
{
    Serializable &payload;
};

class PropertyCommon : public Serializable
{
public:
    std::string name;
    std::string description;
    PropertyCommon(std::string name = "", std::string description = "") : name(name), description(description) {}
    Res serialize_info(Serializer &ser)
    {
        ser.serialize(name);
        ser.serialize(description);
        return Res::Ok();
    }
    virtual Res serialize(Serializer &ser) = 0;
    virtual Res deserialize(Deserializer &des) = 0;
};

template <typename T>
struct Property : public PropertyCommon
{
    T value;
    std::optional<std::function<T()>> getter;
    std::optional<std::function<void(T)>> setter;
    Property(std::string name = "", std::string description = "") : PropertyCommon(name, description) {}
    Res serialize(Serializer &ser) override
    {
        if (getter)
        {
            T t = getter.value()();
            return ser.serialize(t);
        }
        return ser.serialize(value);
    }
    Res deserialize(Deserializer &des) override
    {
        if (setter)
        {
            T value;
            des.deserialize(value);
            setter.value()(value);
            return Res::Ok();
        }
        des.deserialize(value);
        return Res::Ok();
    }
    Res get_value(T &v)
    {
        if (getter)
        {
            v = getter.value()();
            return Res::Ok();
        }
        v = value;
        return Res::Ok();
    }
    Res set_value(T value)
    {
        if (setter)
        {
            setter.value()(value);
            return Res::Ok();
        }
        return Res::Err(0, "No setter");
    }
};

#endif
