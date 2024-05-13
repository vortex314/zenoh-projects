use super::{msg::ProxyMessage, ProxyMessage::*};
use alloc::string::String;
use crate::protocol::msg::ProxyMessage;

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
            ProxyMessage::Subscribe { topic ,  qos } => {
                self.on_subscribe(ProxyMessage::Subscribe { topic,qos  }).await;
            },
            _ => {
                // Ignore
            },
        }
    }

    async fn on_connect(&mut self, connect : ProxyMessage::Connect) {
        self.protocol_id = connect.protocol_id;
        self.duration = connect.duration;
        self.client_id = connect.client_id;
        self.send_client(ProxyMessage::WillTopicReq).await;

        self.send_client(ProxyMessage::ConnAck { return_code: 0 }).await;
        self.state = ServerState::Connected;

    }


    async fn on_will_topic(&mut self, will_topic : ProxyMessage::WillTopic) {
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

    async fn on_disconnect(&mut self) {
        // Disconnect
    }

    async fn on_connect(&mut self) {
        // Connect
    }





}

