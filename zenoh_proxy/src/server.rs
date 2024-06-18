use super::{msg::ProxyMessage, ProxyMessage::*};
use alloc::string::String;

enum ServerState {
    Disconnected,
    WaitWillTopic,
    WaitWillMessage,
    Connected,
    Will,
}

enum ClientState {
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
                self.on_connect(protocol_id,duration,client_id).await;
            },
            ProxyMessage::WillTopic { topic } => {
            },
            ProxyMessage::WillMsg { message } => {
            },
            ProxyMessage::Publish { topic_id, message } => {
            },
            ProxyMessage::Register { topic_id, topic_name } => {
            },
            ProxyMessage::Subscribe { topic ,  qos } => {
            },
            ProxyMessage::Disconnect => {
                self.on_disconnect().await;
            },
            _ => {
                // Ignore
            },
        }
    }

    async fn on_connect(&mut self, protocol_id: u8, duration: u16, client_id: String) {
        self.protocol_id = protocol_id;
        self.duration = duration;
        self.client_id = client_id;
        self.send_client(ProxyMessage::WillTopicReq).await;

        self.send_client(ProxyMessage::ConnAck { return_code: 0 }).await;
        self.state = ServerState::Connected;

    }


    async fn on_will_topic(&mut self, will_topic : String) {
        self.will_topic = Some(will_topic.topic);
        self.send_client(ProxyMessage::WillMsgReq).await;
        self.state = ServerState::WaitWillMessage;
    } 

    async fn on_will_message(&mut self, will_message : ProxyMessage::WillMsg) {
        self.will_message = Some(will_message.message);
        self.state = ServerState::Will;
    }

    async fn on_publish(&mut self, publish : ProxyMessage::Publish) {
        // Publish message to topic
    }

    async fn on_register(&mut self, register : ProxyMessage::Register) {
        // Register topic
    }

    async fn on_subscribe(&mut self, subscribe : ProxyMessage::Subscribe) {
        // Subscribe to topic
    }

    async fn on_disconnect(&mut self) {
        // Disconnect
    }

    async fn on_ping(&mut self) {
        // Ping
    }

    async fn on_unsubscribe(&mut self) {
        // Unsubscribe
    }

    async fn on_log(&mut self) {
        // Log
    }

    async fn on_will_topic_req(&mut self) {
        // Will Topic Request
    }

    async fn on_will_msg_req(&mut self) {
        // Will Message Request
    }

    async fn on_will_topic_upd(&mut self) {
        // Will Topic Update
    }

    async fn on_will_msg_upd(&mut self) {
        // Will Message Update
    }

    async fn on_conn_ack(&mut self) {
        // Connection Acknowledgement
    }

    async fn on_sub_ack(&mut self) {
        // Subscription Acknowledgement
    }

    async fn on_pub_ack(&mut self) {
        // Publish Acknowledgement
    }

    async fn on_unsub_ack(&mut self) {
        // Unsubscribe Acknowledgement
    }

    async fn on_ping_resp(&mut self) {
        // Ping Response
    }







}

