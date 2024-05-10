use super::ProxyMessage;




enum ServerState {
    Disconnected,
    WaitWillTopic,
    WaitWillMessage,
    Connected,
    Will,
}

struct ServerSession {
    state: ServerState,
    client_id: String,
    protocol_id: u8,
    duration: u16,
    will_topic: Option<String>,
    will_message: Option<String>,



}

impl ServerSession {

    async fn send_client(&mut self, message: ProxyMessage) {
        // Send message to client
    }

    async fn on_message(&mut self, message: ProxyMessage) {
        match message {
            ProxyMessage::Connect { protocol_id, duration, client_id } => {
                self.on_connect(ProxyMessage::Connect { protocol_id, duration, client_id }).await;
            },
            ProxyMessage::WillTopic { topic } => {
                self.on_will_topic(ProxyMessage::WillTopic { topic }).await;
            },
            ProxyMessage::WillMsg { message } => {
                self.on_will_message(ProxyMessage::WillMsg { message }).await;
            },
            ProxyMessage::Publish { topic_id, message } => {
                self.on_publish(ProxyMessage::Publish { topic_id, message }).await;
            },
            ProxyMessage::Register { topic_id, topic_name } => {
                self.on_register(ProxyMessage::Register { topic_id, topic_name }).await;
            },
            ProxyMessage::Subscribe { topic  } => {
                self.on_subscribe(ProxyMessage::Subscribe { topic }).await;
            },
            _ => {
                // Ignore
            },
        }
    }

    async fn on_connect(&mut self, connect : ProxyMessage::Connect) {
        self.protocol_id = protocol_id;
        self.duration = duration;
        self.client_id = client_id;
        self.send_client(ProxyMessage::WillTopicReq).await;

        send_client(ProxyMessage::ConnAck { return_code: 0 }).await;
        self.state = ServerState::Connected;

    }


    loop 



}

