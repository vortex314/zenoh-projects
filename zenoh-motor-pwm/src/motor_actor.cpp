
#include <motor_actor.h>



MotorActor::MotorActor() : MotorActor("sys", 4096, 5, 5) {}

MotorActor::MotorActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<MotorEvent, MotorCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(100);

}

void MotorActor::on_cmd(MotorCmd &cmd)
{
    INFO("Received Motor command");
    if (cmd.msg)
    {
        
    }
}

void MotorActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        INFO("Timer 1 : Publishing Motor properties");
        
    }
    else
    {
        INFO("Unknown timer id: %d", id);
    }
}

void MotorActor::on_start(void)
{
    
}

MotorActor::~MotorActor()
{
    INFO("Stopping Motor actor");
}

Res MotorMsg::serialize(Serializer &ser)
{
    ser.reset();
    ser.array_begin();
    ser.array_end();
    return Res::Ok();
}

Res MotorMsg::deserialize(Deserializer &des)
{
    des.array_begin();
    des.array_end();
    return Res::Ok();
}
