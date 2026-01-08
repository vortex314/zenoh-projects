
#include <actor.h>
#include <queue>

ActorRef NULL_ACTOR = ActorRef("null");

void Actor::set_eventbus(EventBus *eventbus)
{
    this->_eventbus = eventbus;
}

void Actor::emit(const Envelope *env)
{
    if (_eventbus)
    {
        _eventbus->push(env);
    }
    else
    {
        ERROR("EventBus not set for actor %s", name());
    }
}

void Actor::emit(const Msg *msg)
{
    if (_eventbus)
    {
        _eventbus->push(new Envelope(ref(),msg));
    }
    else
    {
        ERROR("EventBus not set for actor %s", name());
    }
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

            return Res::Err(-1, "xQueueAddToSet failed");
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
    return Res::Ok(true);
}

Res Thread::add_actor(ThreadSupport &actor)
{
    INFO("Adding actor %s:%X to thread %s", actor.name(), &actor, name());
    _actors.push_back(&actor);

    return Res::Ok(true);
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
        //        actor->handle_all_cmd();
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
                    //                 actor->handle_all_cmd();
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

EventBus::EventBus(size_t size) : Queue<const Envelope *>(size) {};

void EventBus::push(const Envelope *msg)
{
    if (send(msg) == false)
    {
        ERROR("EventBus queue full, dropping message");
        delete msg;
    }
}

void EventBus::loop()
{
    Envelope *pmsg;
    for (Actor *actor : _actors)
    {
        actor->on_start();
    }
    uint32_t timeout = 100; // 100 ms timeout

    while (true)
    {
    //    INFO("EventBus waiting for messages or timeout %d ms", timeout);
        if (receive((const Envelope **)&pmsg, timeout))
        {
            for (const auto &handler : _message_handlers)
            {
                handler(*pmsg);
            }
            for (Actor *actor : _actors)
            {
                actor->on_message(*pmsg);
            }
            delete pmsg;
        }

        timeout = 1000;
        for (Actor *actor : _actors)
        {
            for (int id : actor->timers().get_expired_timers())
            {
                actor->on_message(Envelope(new TimerMsg(id)));
                actor->timers().refresh(id);
            }
            uint64_t sleep_duration = actor->sleep_time();
            DEBUG("Actor %s sleep duration: %llu", actor->name(), sleep_duration);
            if (sleep_duration < timeout)
                timeout = sleep_duration;
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

ActorRef EventBus::find_actor(const char *name)
{
    for (auto actor : _actors)
    {
        if (actor->name() == name)
        {
            return actor->ref();
        }
    }
    return NULL_ACTOR;
}
