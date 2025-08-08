
#include <actor.h>
#include <queue>

class SpeedMsg : public Msg
{
public:
    static constexpr uint32_t id = FILE_LINE_HASH;          // Unique ID based on file and line number
    inline uint32_t type_id() const override { return id; } // Default implementation, can be overridden
    ~SpeedMsg() = default;
    float speed;
    float direction;
    SpeedMsg(float speed, float direction) : speed(speed), direction(direction) {};
};

class NullActor : public Actor
{
public:
    NullActor(size_t qd) : Actor(qd) {}
    void on_start()
    {
        INFO(" NullActor started")
    }
    void on_message(ActorRef &sender, const Msg &msg)
    {
        handle<Msg>(msg, [](const Msg &msg)
                    { INFO("Message for NullActor "); });
        handle<TimerMsg>(msg, [](const TimerMsg &msg)
                         { INFO("Message for NullActor "); });
    };
};

NullActor null_actor(0);

void tester1()
{
    Queue<MsgContext*> channel(8);
    MsgContext* mc = new MsgContext(null_actor.ref(),new TimerMsg(10));
    channel.send(mc, 100);
    MsgContext* msg_context;
    channel.receive(msg_context, 100);
    handle<TimerMsg>(*msg_context->pmsg, [](const TimerMsg &v) {});
    delete msg_context->pmsg;
    delete msg_context;
}

void panic_here(const char *s)
{
    printf("PANIC : %s \n", s);
    // force coredump
    int *p = 0;
    *p = 0;
}

uint64_t current_time() { return esp_timer_get_time() / 1000; }

Timer::Timer(bool auto_reload, bool active, uint64_t period,
             uint64_t expires_at)
{
    _auto_reload = auto_reload;
    _active = active;
    _period = period;
    _expires_at = expires_at;
}

Timer Timer::Repetitive(uint64_t period)
{
    return Timer(true, true, period, current_time() + period);
}

Timer Timer::OneShot(uint64_t delay)
{
    return Timer(false, true, 0, current_time() + delay);
}

void Timer::make_one_shot(uint64_t delay)
{
    _active = true;
    _auto_reload = false;
    _period = 0;
    _expires_at = current_time() + delay;
}

void Timer::fire(uint64_t delay)
{
    _active = true;
    _auto_reload = false;
    _period = 0;
    _expires_at = current_time() + delay;
}

void Timer::make_repetitive(uint64_t period)
{
    _active = true;
    _auto_reload = true;
    _period = period;
    _expires_at = current_time() + period;
}

bool Timer::is_expired(uint64_t now) const & { return now >= _expires_at; }

void Timer::refresh(uint64_t now)
{
    //    INFO("Refreshing timer, now %lld, expires at %lld period %lld active %d auto_reload %d  ", now, _expires_at,_period,_active, _auto_reload);
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
void Timer::start()
{
    _active = true;
    _expires_at = current_time() + _period;
}

void Timer::reset() { _expires_at = current_time() + _period; }

uint64_t Timers::get_next_expires_at()
{
    uint64_t expires_at = UINT64_MAX;
    for (const Timer &timer : _timers)
    {
        if (timer._active && timer._expires_at < expires_at)
        {
            expires_at = timer._expires_at;
        }
    }
    return expires_at;
}

uint64_t Timers::sleep_time()
{
    uint64_t expires_at = get_next_expires_at();
    uint64_t now = current_time();
    if (expires_at <= now)
    {
        return 0;
    }
    return expires_at - now;
}

std::vector<int> Timers::get_expired_timers()
{
    uint64_t now = current_time();
    std::vector<int> expired_timers;
    for (int idx = 0; idx < _timers.size(); idx++)
    {
        //       INFO("Checking timer %d, expires at %ld, now %ld", idx, _timers[idx]._expires_at, now);
        if (_timers[idx].is_expired(now))
        {
            expired_timers.push_back(idx);
        }
    }
    return expired_timers;
}

void Timers::refresh(int id)
{
    _timers[id].refresh(current_time());
}

void Timers::refresh_expired_timers()
{
    uint64_t now = current_time();
    for (Timer &timer : _timers)
    {
        timer.refresh(now);
    }
}
int Timers::create_one_shot(uint64_t delay)
{
    _timers.push_back(Timer::OneShot(delay));
    return _timers.size() - 1;
}
int Timers::create_repetitive(uint64_t period)
{
    _timers.push_back(Timer::Repetitive(period));
    return _timers.size() - 1;
}
void Timers::stop(int id)
{
    _timers[id].stop();
}

void Timers::fire(int id, uint64_t delay)
{
    _timers[id].fire(delay);
}

/*

A thread is created to manage multiple actors. The thread will wait for the actor with the lowest sleep time and handle the command or timer event.

*/

Res Thread::start()
{
    BaseType_t xCoreID = tskNO_AFFINITY;
    if (_preferred_cpu == Cpu::CPU1)
    {
        xCoreID = 1;
    }
    if (_preferred_cpu == Cpu::CPU0)
    {
        xCoreID = 0;
    }
    if (_preferred_cpu == Cpu::CPU_ANY)
    {
        xCoreID = tskNO_AFFINITY;
    }
    INFO(" starting Thread %s on core %d ", name(), xCoreID);
    _queue_set = xQueueCreateSet(_queue_set_size);
    for (ThreadSupport *actor : _actors)
    {
        auto r = xQueueAddToSet(actor->queue_handle(), _queue_set);
        if (r != pdPASS)
        {
            panic_here("Failed to add actor to queue set");

            return Res(-1, "xQueueAddToSet failed");
        }
    }

    CHECK(0 == xTaskCreatePinnedToCore([](void *arg)
                                       {
                       auto self = static_cast<Thread *>(arg);
                       self->run(); },
                                       name(), _stack_size, this, _priority, &_task_handle, xCoreID));
    /*CHECK(0 == xTaskCreate(
                   [](void *arg)
                   {
                       auto self = static_cast<Thread *>(arg);
                       self->run();
                   },
                   name(), _stack_size, this, _priority, &_task_handle));*/
    return ResOk;
}

Res Thread::add_actor(ThreadSupport &actor)
{
    INFO("Adding actor %s:%X to thread %s", actor.name(), &actor, name());
    _actors.push_back(&actor);

    return ResOk;
}

/*void Thread::handle_all_cmd()
{
    for (auto actor : _actors)
    {
        actor->handle_all_cmd();
    }
}*/
/*
void Thread::handle_expired_timers()
{
    for (auto actor : _actors)
    {
        actor->handle_expired_timers();
    }
}*/

void Thread::step()
{
    uint64_t min_sleep_msec = UINT64_MAX;
    for (auto actor : _actors)
    {
        if (min_sleep_msec > actor->sleep_time())
        {
            min_sleep_msec = actor->sleep_time();
        }
    }
    /* QueueSetMemberHandle_t queue = */ xQueueSelectFromSet(_queue_set, pdMS_TO_TICKS(min_sleep_msec));
    for (auto actor : _actors)
    {
        actor->handle_all_cmd();
        actor->handle_expired_timers();
    }
}

void Thread::run()
{
    INFO("starting Thread %s", name());
    for (auto actor : _actors)
        actor->on_start();
    while (!_stop_thread)
    {
        uint64_t min_sleep_msec = UINT64_MAX;
        for (auto actor : _actors)
        {
            if (min_sleep_msec > actor->sleep_time())
            {
                min_sleep_msec = actor->sleep_time();
            }
        }
        // Wait for either a message or timer expiration
        //        INFO("Thread %s waiting for %ld msec", name(), min_sleep_msec);
        QueueSetMemberHandle_t queue = xQueueSelectFromSet(_queue_set, pdMS_TO_TICKS(min_sleep_msec));
        // Handle commands for the actor that received a message

        if (queue != NULL)
        {
            for (auto actor : _actors)
            {
                if (actor->queue_handle() == queue)
                {
                    //                   INFO("Thread %s handling command for actor %s", name(), actor->name());
                    actor->handle_all_cmd();
                    break; // Found the actor, no need to continue loop
                }
            }
        }
        // Always check timers after queue check or timeout
        for (auto actor : _actors)
        {
            //            INFO("Thread %s handling expired timers for actor %s", name(), actor->name());
            actor->handle_expired_timers();
        }
    }
    INFO("stopping actor %s", name());
    for (auto actor : _actors)
        actor->on_stop();
}
