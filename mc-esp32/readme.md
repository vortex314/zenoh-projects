# Principles
- Actor => emit(Event) => Call all event handlers => Handle event .... => tell Actor ==> Put msg on queue
- Actor has 2 callbacks : timer and msg
- Tell is always async on queue