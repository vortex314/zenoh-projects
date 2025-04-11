
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
    ser.map_begin();
    ser.serialize(H("rpm_measured"), rpm_measured);
    ser.serialize(H("rpm_target"), rpm_target);
    ser.serialize(H("current"), current);
    ser.serialize(H("Kp"), Kp);
    ser.serialize(H("Ki"), Ki);
    ser.serialize(H("Kd"), Kd);
    ser.map_end();
    return Res::Ok();
}

Res MotorMsg::deserialize(Deserializer &des)
{
    des.map_begin();
    des.iterate_map([&](Deserializer &d, uint32_t key) -> Res
                    {
//    INFO("key %d", key);
        switch (key)
        {
        case H("rpm_target"):
            return d.deserialize(rpm_target);
        case H("Kp"):   
           return d.deserialize(Kp);
        case H("Ki"):   
           return d.deserialize(Ki);
        case H("Kd"):   
           return d.deserialize(Kd);
        default:
            INFO("unknown key %d",key);
            return d.skip_next();
        } });
    des.map_end();
    return Res::Ok();
}
