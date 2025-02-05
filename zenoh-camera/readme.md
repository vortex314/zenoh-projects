# Building PS4_actor
- follow : https://bluepad32.readthedocs.io/en/latest/plat_esp32/
- install btstack in esp-idf
[Architecture](doc/)


# Actor framework in FreeRTOS
- As lightway as possible, inherits from base class Actor 
- Actor is a separate thread
- Actor has a single queue for messages to act upon
- Actor can invoke registered function handlers for events , which MUST not be blocking.
- Events are passed by reference 
- Messages are a collection of optional fields in a sruct
- The Message queue is passing pointers and the receiving side MUST 'delete' the memory 
- _Actors are not aware about each other, know only their own types_
- Actors are glued together at the main level by registering closure handlers that send messages to other actors

## Future ??
- a single thread across multiple actors with FreeRTOS queue sets 
-- multiple queues consolidation
-- multiple timers consolidation


## Example Wifi Actor wakes up Zenoh Actor
```
wifi_actor.handlers.push_back([&](WifiEvent wifi_event){ 
    if (event.signal == WifiSignal::WIFI_CONNECTED) {
      zenoh_actor.cmds.send(new ZenohCmd{.action = ZenohAction::Connect});
    }
}
```
## tuning to get this programmed
CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE 2304 -> 3120
CONFIG_ESP_MAIN_TASK_STACK_SIZE increase
