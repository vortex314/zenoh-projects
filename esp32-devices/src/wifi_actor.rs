use esp_wifi::{
    current_millis,
    wifi::{
        ClientConfiguration,Configuration, WifiController, WifiDevice, WifiError, WifiEvent, WifiStaDevice, WifiState,
        utils::create_network_interface, AccessPointInfo, 
    },
    EspWifiInitFor,
};

use limero::ActorTrait;

const SSID: &str = env!("WIFI_SSID");
const PASSWORD: &str = env!("WIFI_PASS");

enum WifiCmd {
    Scan,
    Connect,
    Disconnect,
}

pub enum WifiEvent {
    Connected ,
    Disconnected,
}

struct WifiActor {
    command: Sink<WifiCmd, 3>,
    events: Source<WifiEvent>,
    timers: Timers,
    stack : &'static Stack<WifiDevice<'static,WifiStaDevice>>,
    clocks : &'static ClockControl,
}

impl WifiActor {
    pub fn new( swifi:WIFI, wifi_timer: PeriodicTimer, clocks: ClockControl, spawner: Spawner) -> Self {
        let init = esp_wifi::initialize(
            EspWifiInitFor::Wifi,
            wifi_timer,
            Rng::new(peripherals.RNG),
            peripherals.RADIO_CLK,
            &clocks,
        )
        .expect("wifi init failed");
    
        let wifi = peripherals.WIFI;
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
        let self.stack:&'static Stack<WifiDevice<WifiStaDevice>>  = Box::leak(Box::new(stack));
    
        spawner.spawn(connection(controller)).ok();
        spawner.spawn(net_task(stack)).ok();
        WifiActor {
            command: Sink::new(),
            events: Source::new(),
            timers: Timers::new(),
            stack,
        }
    }

    pub fn stack(&self) -> &'static Stack<WifiDevice<'static,WifiStaDevice>> {
        self.stack
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.command.sink_ref()
    }
}


#[embassy_executor::task]
async fn connection(mut controller: WifiController<'static>) {

    loop {
        if stack.is_link_up() {
            break;
        }
        Timer::after(Duration::from_millis(500)).await;
    }

    info!("Waiting to get IP address...");
    loop {
        if let Some(config) = stack.config_v4() {
            info!("Got IP: {}", config.address);
            break;
        }
        Timer::after(Duration::from_millis(500)).await;
    }
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
            Ok(_) => info!("Wifi connected!"),
            Err(e) => {
                info!("Failed to connect to wifi: {e:?}");
                Timer::after(Duration::from_millis(5000)).await
            }
        }
    }
}

#[embassy_executor::task]
async fn net_task(stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>) {
    stack.run().await
}
