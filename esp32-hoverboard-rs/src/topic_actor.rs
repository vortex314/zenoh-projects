use limero::*;

trait MsgInterface<CMD,EVENT> {
    fn handler(&mut self) -> Box<dyn Handler<CMD>>;
    fn add_listener(&mut self, listener :  Box<dyn Handler<EVENT>>) ;
}
struct TopicActor {
    topic_cmds : CmdQueue<TopicCmd>,
    event_handlers : EventHandlers<TopicEvent>
}

impl MsgInterface<TopicCmd,TopicEvent> for TopicActor {
    fn handler(&mut self) -> Box<dyn Handler<TopicCmd>> {
        self.topic_cmds.handler()
    }

    fn add_listener(&mut self, listener :  Box<dyn Handler<TopicEvent>>) {
        self.event_handlers.add_listener(listener);
    }

}