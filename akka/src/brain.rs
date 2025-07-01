use actix::{Actor, Context, Handler};
use log::info;
use tokio::runtime::Handle;

use crate::{multicast::McEvent, value::Value};



struct  Property <T>{
    pub value: T,
    pub last_updated: std::time::SystemTime,
}

impl <T> Property<T> {
    pub fn new(value: T) -> Self {
        Property {
            value,
            last_updated: std::time::SystemTime::now(),
        }
    }
}




struct Brain {
    pub drive_name : String,
    pub drive_speed: Property<f64>,
    pub drive_direction: Property<f64>,
    pub steer_direction: Property<f64>,
    pub tilt_angle: Property<f64>,
}


impl Brain {
    pub fn new() -> Self {
        Brain {
            drive_name: "esp2/motor".to_string(),
            drive_speed: Property::new(0.0),
            drive_direction: Property::new(0.0),
            steer_direction: Property::new(0.0),
            tilt_angle: Property::new(0.0),
        }
    }
    fn handle_pub_drive(&self, v: &Value) {
        v["speed"].handle::<f64, _>(|speed| {
            info!("drive speed: {}",  speed);
            self.drive_speed.value = *speed;
            self.drive_speed.last_updated = std::time::SystemTime::now();
        });
        v["direction"].handle::<f64, _>(|direction| {
            info!("received drive direction: {}",  direction);
            self.drive_direction.value = *direction;
            self.drive_direction.last_updated = std::time::SystemTime::now();
        });
    }
}   

impl Actor for Brain {
    type Context = Context<Self>;
}   

impl Handler<McEvent> for Brain {
    type Result = ();

    fn handle(&mut self, msg: McEvent, _: &mut Self::Context) -> Self::Result {
        match msg {
            McEvent::Received(value) => {
                if value["src"] == Value::from(self.drive_name.as_str()) {
                    self.handle_pub_drive(&value["pub"]);
                } else {
                    info!("Received non-motor value: {}", value);
                }
            }
            McEvent::ConnectionLost => {
                info!("Brain connection lost");
            }
            McEvent::ConnectionEstablished => {
                info!("Brain connection established");
            }
        }
    }
}