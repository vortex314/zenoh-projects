# Template for Zenoh Actors on ESP32
- LedActor - on board LED 
- WifiActor - establishes connection and connects to strongest SSID
- OtaActor - takes care of downloading new images in flash 
- ZenohActor - does pub / sub as client via WiFi

# Actor framework in FreeRTOS
- As lightway as possible, inherits from base class Actor 
- Actors should be re-usable in total different programs, re-usable components.
- _Actors are not aware about each other, know only their own types_
- Actors are glued together at the main level by registering closure handlers that send messages to other actors
- The glue code should be stateless or re-entrant  if re-used among event handlers ( they can be triggered by multiple threads )
- Actor is a separate thread ( version 1 ) 
- Actor has a single queue for messages to act upon
- Actor is triggered by different events : on_start, on_cmd ( received a message ), on_timer ( timer expired )
- Actor can invoke registered function handlers for events , which MUST not be blocking.
- Events are passed by reference .
- Messages are a collection of optional fields in a struct to enable polymorphism
- The Message queue is passing pointers and the receiving side MUST 'delete' the memory 
- Actor should not be aware about their own PubSub address , as they can run in multiple instances

## One thread for multiple actors
- a single thread across multiple actors with FreeRTOS queue sets 

## Example Wifi Actor wakes up Zenoh Actor
```c++
wifi_actor.on_event([&](WifiEvent wifi_event){ 
    if (event.signal == WifiSignal::WIFI_CONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Connect});
    }
}
```
## Example OTA actor with different entry points 
```c++
// received a command via tell and send a reply 
void OtaActor::on_cmd(OtaCmd &cmd)
{
    if (cmd.msg && cmd.msg.value().image)
    {
        if (flash(cmd.msg.value().image.value().data(), cmd.msg.value().image.value().size()).is_err())
        {
            OtaMsg msg;
            msg.rc = -1;
            msg.message = "Failed to flash image";
            emit(OtaEvent{.serdes = PublishSerdes(msg)});
        }
    }
}
// waiting for message , one of the timers expired
void OtaActor::on_timer(int timer_id)
{
    INFO("timer_id %d not handled", timer_id);
}
// execute some code at startup 
void OtaActor::on_start()
{
}
```
## tuning to get this programmed
CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE 2304 -> 3120
CONFIG_ESP_MAIN_TASK_STACK_SIZE increase
