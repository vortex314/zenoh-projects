use alloc::boxed::Box;
use embassy_executor::Spawner;
use embassy_futures::select::select3;
use embassy_futures::select::Either3::First;
use embassy_futures::select::Either3::Second;
use embassy_futures::select::Either3::Third;

use embassy_net::{Config, Stack, StackResources};
use embassy_time::{Duration, Timer};
use esp_hal::{
    clock::{ClockControl, Clocks},
    peripheral::Peripheral,
    peripherals::{self, WIFI},
    rng::Rng,
    timer::{ErasedTimer, PeriodicTimer},
};
use esp_wifi::{
    current_millis,
    wifi::{
        utils::create_network_interface, AccessPointInfo, ClientConfiguration, Configuration,
        WifiController, WifiDevice, WifiError, WifiEvent, WifiStaDevice, WifiState,
    },
    EspWifiInitFor,
};
use log::info;
use minicbor::decode::info;

use crate::limero::{Sink, SinkRef, Source, Timers};
use crate::{limero::ActorTrait, SourceTrait};

const SSID: &str = env!("WIFI_SSID");
const PASSWORD: &str = env!("WIFI_PASS");

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
    cmds: Sink<WifiCmd, 3>,
    events: Source<WifiActorEvent>,
    timers: Timers,
    stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>,
    //   clocks : &'static ClockControl<'static>,
    controller: WifiController<'static>,
}

impl WifiActor {
    pub fn new(
        wifi: WIFI,
        wifi_timer: PeriodicTimer<ErasedTimer>,
        clocks: &Clocks,
        rng: Rng,
        radio_clocks: peripherals::RADIO_CLK,
        _spawner: Spawner,
    ) -> Self {
        let init =
            esp_wifi::initialize(EspWifiInitFor::Wifi, wifi_timer, rng, radio_clocks, &clocks)
                .expect("wifi init failed");

        let (wifi_interface, controller) =
            esp_wifi::wifi::new_with_mode(&init, wifi, WifiStaDevice).unwrap();

        let config = Config::dhcpv4(Default::default());

        let seed = 1234; // very random, very secure seed

        let stack_resource = Box::leak(Box::new(StackResources::<3>::new()));
        let stack/* :  Stack<WifiDevice<'_, WifiStaDevice>>*/ = Stack::new(
            wifi_interface,
            config,
            stack_resource,
            seed,
        );
        let stack: &'static Stack<WifiDevice<WifiStaDevice>> = Box::leak(Box::new(stack));

        //      spawner.spawn(connection(controller)).ok();
        //      spawner.spawn(net_task(stack)).ok();
        WifiActor {
            cmds: Sink::new(),
            events: Source::new(),
            timers: Timers::new(),
            stack,
            //        clocks,
            controller,
        }
    }

    pub fn stack(&self) -> &'static Stack<WifiDevice<'static, WifiStaDevice>> {
        self.stack
    }

    pub fn sink_ref(&self) -> SinkRef<WifiCmd> {
        self.cmds.sink_ref()
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

    async fn net_task(&mut self) {
        self.stack.run().await
    }
}

impl ActorTrait<WifiCmd, WifiActorEvent> for WifiActor {
    async fn run(&mut self) {
        loop {
            info!("WifiActor::run");
            match select3(
                connection(&mut self.controller, self.stack, &self.events),
                self.stack.run(),
                self.cmds.next(),
            )
            .await
            {
                First(_) => {}
                Second(_) => {}
                Third(cmd) => {
                    self.handle_cmd(cmd.unwrap()).await;
                }
            }
        }
    }

    fn sink_ref(&self) -> SinkRef<WifiCmd> {
        self.cmds.sink_ref()
    }

    fn add_listener(&mut self, sink_ref: SinkRef<WifiActorEvent>) {
        self.events.add_listener(sink_ref);
    }
}

async fn connection(
    controller: &mut WifiController<'static>,
    stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>,
    source: &Source<WifiActorEvent>,
) {
    /*    loop {
        info!("Waiting for link up...");
        if stack.is_link_up() {
            break;
        }
        Timer::after(Duration::from_millis(1000)).await;
    }

    info!("Waiting to get IP address...");
    loop {
        if let Some(config) = stack.config_v4() {
            info!("Got IP: {}", config.address);
            break;
        }
        Timer::after(Duration::from_millis(500)).await;
    }*/
    info!("start connection task");
    info!("Device capabilities: {:?}", controller.get_capabilities());
    loop {
        match esp_wifi::wifi::get_wifi_state() {
            WifiState::StaConnected => {
                // wait until we're no longer connected
                controller.wait_for_event(WifiEvent::StaDisconnected).await;
                Timer::after(Duration::from_millis(5000)).await
            }
            _ => {}
        }
        if !matches!(controller.is_started(), Ok(true)) {
            let client_config = Configuration::Client(ClientConfiguration {
                ssid: SSID.try_into().unwrap(),
                password: PASSWORD.try_into().unwrap(),
                ..Default::default()
            });
            controller.set_configuration(&client_config).unwrap();
            info!("Starting wifi");
            controller.start().await.unwrap();
            info!("Wifi started!");
        }
        info!("About to connect...");

        match controller.connect().await {
            Ok(_) => {
                info!("Wifi connected!");
                source.emit(WifiActorEvent::Connected);
            }
            Err(e) => {
                info!("Failed to connect to wifi: {e:?}");
                Timer::after(Duration::from_millis(5000)).await
            }
        }
        info!("Waiting to get IP address...");
        loop {
            if let Some(config) = stack.config_v4() {
                info!("Got IP: {}", config.address);
                break;
            }
            Timer::after(Duration::from_millis(500)).await;
        }
    }
}
