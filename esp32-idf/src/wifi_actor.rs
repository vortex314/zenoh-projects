use std::time::Duration;

use esp_idf_hal::modem::Modem;
use esp_idf_svc::eventloop::EspSystemEventLoop;
use esp_idf_svc::nvs::EspDefaultNvsPartition;
use esp_idf_svc::timer::EspAsyncTimer;
use esp_idf_svc::timer::EspTimerService;
use esp_idf_svc::wifi::AuthMethod;
use esp_idf_svc::wifi::BlockingWifi;
use esp_idf_svc::wifi::ClientConfiguration;
use esp_idf_svc::wifi::Configuration;
use esp_idf_svc::wifi::EspWifi;

use esp_idf_hal::timer::TimerConfig;

use anyhow::Result;
use log::info;
use log::warn;
use minicbor::decode::info;

use crate::async_wait_millis;
use crate::limero::Actor;
use crate::CmdQueue;
use crate::EventHandlers;
use crate::Handler;
use crate::Timers;

#[derive(Clone, Debug)]
pub enum WifiCmd {
    Scan,
    Connect,
    Disconnect,
}

#[derive(Clone, Debug)]
pub enum WifiActorEvent {
    Connected,
    Disconnected,
}

pub struct WifiActor {
    cmds: CmdQueue<WifiCmd>,
    events: EventHandlers<WifiActorEvent>,
    timers: Timers,
    wifi: BlockingWifi<EspWifi<'static>>,
}

impl WifiActor {
    pub fn new(
        sysloop: EspSystemEventLoop,
        modem: Modem,
        wifi_ssid: &str,
        wifi_pswd: &str,
    ) -> Result<Self> {
        let nvs = EspDefaultNvsPartition::take()?;
        let mut wifi = BlockingWifi::wrap(
            EspWifi::new(modem, sysloop.clone(), Some(nvs)).unwrap(),
            sysloop,
        )?;

        let mut ssid = heapless::String::<32>::new();
        let mut password = heapless::String::<64>::new();
        ssid.push_str(wifi_ssid).unwrap();
        password.push_str(wifi_pswd).unwrap();

        wifi.set_configuration(&Configuration::Client(ClientConfiguration {
            ssid,
            bssid: None,
            auth_method: AuthMethod::None,
            password,
            channel: None,
            ..Default::default()
        }))?;

        Ok(WifiActor {
            cmds: CmdQueue::new(10),
            events: EventHandlers::new(),
            timers: Timers::new(),
            wifi,
        })
    }

    async fn handle_cmd(&mut self, cmd: WifiCmd) {
        match cmd {
            WifiCmd::Scan => {
                info!("Scanning...");
            }
            WifiCmd::Connect => {
                info!("Connecting...");
            }
            WifiCmd::Disconnect => {
                info!("Disconnecting...");
            }
        }
    }
}

impl Actor<WifiCmd, WifiActorEvent> for WifiActor {
    async fn run(&mut self) {
        info!("WifiActor::run");
        loop {
            async_wait_millis(1000).await;

            // Start Wifi
            if self.wifi.start().is_err() {
                warn!("Failed to start wifi");
                continue;
            }

            // Connect Wifi
            if self.wifi.connect().is_err() {
                warn!("Failed to connect wifi");
                continue;
            }

            // Wait until the network interface is up
            if self.wifi.wait_netif_up().is_err() {
                warn!("Failed to wait for network interface");
                continue;
            }

            // Print Out Wifi Connection Configuration
            while !self.wifi.is_connected().unwrap() {
                let config = self.wifi.get_configuration().unwrap();
                info!("Waiting for station {:?}", config);
            }
            self.events.on_event(WifiActorEvent::Connected);

            info!("Wifi Connected");
            while self.wifi.is_connected().unwrap() {
                async_wait_millis(1000).await;
            }
        }
    }

    fn add_listener(&mut self, handler: Box<dyn Handler<WifiActorEvent>>) {
        self.events.add(handler);
    }

    fn handler(&self) -> Box<dyn Handler<WifiCmd>> {
        self.cmds.handler()
    }
}
