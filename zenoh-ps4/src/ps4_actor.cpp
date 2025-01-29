#include <ps4_actor.h>

#define BUTTON_SQUARE_MASK 0x04
#define BUTTON_CROSS_MASK 0x01
#define BUTTON_CIRCLE_MASK 0x02
#define BUTTON_TRIANGLE_MASK 0x08

#define BUTTON_LEFT_MASK 0x08
#define BUTTON_DOWN_MASK 0x02
#define BUTTON_RIGHT_MASK 0x04
#define BUTTON_UP_MASK 0x01

#define BUTTON_LEFT_SHOULDER_MASK 0x10
#define BUTTON_RIGHT_SHOULDER_MASK 0x20
#define BUTTON_LEFT_TRIGGER_MASK 0x40
#define BUTTON_RIGHT_TRIGGER_MASK 0x80

#define BUTTON_LEFT_JOYSTICK_MASK 0x100
#define BUTTON_RIGHT_JOYSTICK_MASK 0x200
#define BUTTON_SHARE_MASK 0x400

struct InfoProp ps4_props[] = {
    {Ps4Props::BUTTON_LEFT, "button_left", "Left button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_RIGHT, "button_right", "Right button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_UP, "button_up", "Up button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_DOWN, "button_down", "Down button", PropType::PROP_UINT, PropMode::PROP_READ},

    {Ps4Props::BUTTON_SQUARE, "button_square", "Square button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_CROSS, "button_cross", "Cross button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_CIRCLE, "button_circle", "Circle button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_TRIANGLE, "button_triangle", "Triangle button", PropType::PROP_UINT, PropMode::PROP_READ},

    {Ps4Props::BUTTON_LEFT_SHOULDER, "button_left_shoulder", "Left Shoulder button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_RIGHT_SHOULDER, "button_right_shoulder", "Right Shoulder button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_LEFT_TRIGGER, "button_left_trigger", "Left Trigger button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_RIGHT_TRIGGER, "button_right_trigger", "Right Trigger button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_LEFT_JOYSTICK, "button_left_joystick", "Left Joystick button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_RIGHT_JOYSTICK, "button_right_joystick", "Right Joystick button", PropType::PROP_UINT, PropMode::PROP_READ},
    {Ps4Props::BUTTON_SHARE, "button_share", "Share button", PropType::PROP_UINT, PropMode::PROP_READ},

    {Ps4Props::STICK_LEFT_X, "axis_x", "Left Stick X", PropType::PROP_SINT, PropMode::PROP_READ},
    {Ps4Props::STICK_LEFT_Y, "axis_y", "Left Stick Y", PropType::PROP_SINT, PropMode::PROP_READ},
    {Ps4Props::STICK_RIGHT_X, "axis_rx", "Right Stick X", PropType::PROP_SINT, PropMode::PROP_READ},
    {Ps4Props::STICK_RIGHT_Y, "axis_ry", "Right Stick Y", PropType::PROP_SINT, PropMode::PROP_READ},

    {Ps4Props::GYRO_X, "gyro_x", "Gyro X axis", PropType::PROP_SINT, PropMode::PROP_READ},
    {Ps4Props::GYRO_Y, "gyro_y", "Gyro Y axis", PropType::PROP_SINT, PropMode::PROP_READ},
    {Ps4Props::GYRO_Z, "gyro_z", "Gyro Z axis", PropType::PROP_SINT, PropMode::PROP_READ},

    {Ps4Props::ACCEL_X, "accel_x", "Accelerometer X Axis ", PropType::PROP_SINT, PropMode::PROP_READ},
    {Ps4Props::ACCEL_Y, "accel_y", "Accelerometer Y Axis ", PropType::PROP_SINT, PropMode::PROP_READ},
    {Ps4Props::ACCEL_Z, "accel_z", "Accelerometer Z Axis ", PropType::PROP_SINT, PropMode::PROP_READ},

    {Ps4Props::RUMBLE, "rumble", "Rumble", PropType::PROP_UINT, PropMode::PROP_WRITE},
    {Ps4Props::LIGHTBAR_RGB, "led_green", "Green LED", PropType::PROP_UINT, PropMode::PROP_WRITE}

};

Ps4Actor::Ps4Actor() : Actor<Ps4Event, Ps4Cmd>(6120, "ps4", 5, 10)
{
    Ps4Actor::ps4_actor_instance = this;
    INFO("Starting PS4 actor sizeof(Ps4Cmd ) : %d ", sizeof(Ps4Cmd));
 //   _timer_id = timer_repetitive(1000);
}
Ps4Actor::Ps4Actor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<Ps4Event, Ps4Cmd>(stack_size, name, priority, queue_depth)
{
    Ps4Actor::ps4_actor_instance = this;
   // _timer_id = timer_repetitive(1000); // timer not used, goes via btstack
}
#define INTERVAL_MS 1000
static void timer_handler(btstack_timer_source_t *ts)
{
    // Your code here
    Ps4Actor *ps = (Ps4Actor *)ts->context;
    // INFO("Timer handler called");
    ps-> on_timer(0);

    // If you want the timer to repeat, re-register it:
    btstack_run_loop_set_timer(ts, INTERVAL_MS);
    btstack_run_loop_add_timer(ts);
}

extern "C" const char *btdm_controller_get_compile_version();
void Ps4Actor::on_start()
{
    // btstack_stdio_init();
    INFO("btdm_controller_get_compile_version()=%s", btdm_controller_get_compile_version());
    // Configure BTstack for ESP32 VHCI Controller
    btstack_init();

    // Must be called before uni_init()
    uni_platform_set_custom(get_my_platform());

    // Init Bluepad32.
    uni_init(0 /* argc */, NULL /* argv */);
    struct btstack_timer_source timer;
    timer.process = timer_handler;
    timer.context = this;

 //   btstack_run_loop_set_timer_handler(&timer, timer_handler);
 //   btstack_run_loop_set_timer(&timer, INTERVAL_MS);

    // Add timer to run loop
 //   btstack_run_loop_add_timer(&timer);
    // Does not return.
    btstack_run_loop_execute(); // Blocking call, no timers will be triggered.No cmd will be processed.
}

void Ps4Actor::on_cmd(Ps4Cmd &cmd)
{
    if (cmd.stop_actor)
    {
        stop();
    }
}

void Ps4Actor::on_timer(int timer_id)
{
    if (timer_id == _timer_id)
    {
        //   emit(Ps4Event{.serdes = PublishSerdes{ps4_output}});
        INFO("publishing ps4 props %s", ps4_props[_prop_counter].name.value().c_str());
        emit(Ps4Event{.prop_info = PublishSerdes{ps4_props[_prop_counter]}});
        _prop_counter++;
        if (_prop_counter >= sizeof(ps4_props) / sizeof(InfoProp))
            _prop_counter = 0;
    }
    else
    {
        INFO("Unknown timer expired ps4");
    }
}

Ps4Actor::~Ps4Actor()
{
    INFO("Stopping PS4 actor");
}

//
// Platform Overrides
//
void Ps4Actor::init(int argc, const char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    INFO("custom: init()");

#if 0
    uni_gamepad_mappings_t mappings = GAMEPAD_DEFAULT_MAPPINGS;

    // Inverted axis with inverted Y in RY.
    mappings.axis_x = UNI_GAMEPAD_MAPPINGS_AXIS_RX;
    mappings.axis_y = UNI_GAMEPAD_MAPPINGS_AXIS_RY;
    mappings.axis_ry_inverted = true;
    mappings.axis_rx = UNI_GAMEPAD_MAPPINGS_AXIS_X;
    mappings.axis_ry = UNI_GAMEPAD_MAPPINGS_AXIS_Y;

    // Invert A & B
    mappings.button_a = UNI_GAMEPAD_MAPPINGS_BUTTON_B;
    mappings.button_b = UNI_GAMEPAD_MAPPINGS_BUTTON_A;

    uni_gamepad_set_mappings(&mappings);
#endif
    //    uni_bt_service_set_enabled(true);
}

void Ps4Actor::on_init_complete(void)
{
    INFO("custom: on_init_complete()");

    // Safe to call "unsafe" functions since they are called from BT thread

    // Start scanning
    uni_bt_enable_new_connections_unsafe(true);

    // Based on runtime condition, you can delete or list the stored BT keys.
    if (1)
        uni_bt_del_keys_unsafe();
    else
        uni_bt_list_keys_unsafe();
}

uni_error_t Ps4Actor::on_device_discovered(bd_addr_t addr, const char *name, uint16_t cod, uint8_t rssi)
{
    // You can filter discovered devices here.
    // Just return any value different from UNI_ERROR_SUCCESS;
    // @param addr: the Bluetooth address
    // @param name: could be NULL, could be zero-length, or might contain the name.
    // @param cod: Class of Device. See "uni_bt_defines.h" for possible values.
    // @param rssi: Received Signal Strength Indicator (RSSI) measured in dBms. The higher (255) the better.

    // As an example, if you want to filter out keyboards, do:
    if (((cod & UNI_BT_COD_MINOR_MASK) & UNI_BT_COD_MINOR_KEYBOARD) == UNI_BT_COD_MINOR_KEYBOARD)
    {
        INFO("Ignoring keyboard");
        return UNI_ERROR_IGNORE_DEVICE;
    }
    // emit(Ps4Event{ .blue_event = DEVICE_DISCOVERED });
    return UNI_ERROR_SUCCESS;
}

void Ps4Actor::on_device_connected(uni_hid_device_t *d)
{
    INFO("custom: device connected: %p", d);
    Ps4Actor *ps = get_my_platform_instance(d);
    ps->emit(Ps4Event{.blue_event = DEVICE_CONNECTED});
}

void Ps4Actor::on_device_disconnected(uni_hid_device_t *d)
{
    INFO("custom: device disconnected: %p", d);
    Ps4Actor *ps = get_my_platform_instance(d);
    ps->emit(Ps4Event{.blue_event = DEVICE_DISCONNECTED});
}

uni_error_t Ps4Actor::on_device_ready(uni_hid_device_t *d)
{
    INFO("custom: device ready: %p", d);
    Ps4Actor *ps = get_my_platform_instance(d);
    ps->emit(Ps4Event{.blue_event = DEVICE_READY});

    trigger_event_on_gamepad(d);
    return UNI_ERROR_SUCCESS;
}

void Ps4Actor::gamepad_to_output(uni_gamepad_t *gp)
{
    ps4_output.button_left = gp->dpad & BUTTON_LEFT_MASK;
    ps4_output.button_right = gp->dpad & BUTTON_RIGHT_MASK;
    ps4_output.button_up = gp->dpad & BUTTON_UP_MASK;
    ps4_output.button_down = gp->dpad & BUTTON_DOWN_MASK;

    ps4_output.button_square = gp->buttons & BUTTON_SQUARE_MASK;
    ps4_output.button_cross = gp->buttons & BUTTON_CROSS_MASK;
    ps4_output.button_circle = gp->buttons & BUTTON_CIRCLE_MASK;
    ps4_output.button_triangle = gp->buttons & BUTTON_TRIANGLE_MASK;

    ps4_output.button_left_sholder = gp->buttons & BUTTON_LEFT_SHOULDER_MASK;
    ps4_output.button_right_sholder = gp->buttons & BUTTON_RIGHT_SHOULDER_MASK;
    ps4_output.button_left_trigger = gp->buttons & BUTTON_LEFT_TRIGGER_MASK;
    ps4_output.button_right_trigger = gp->buttons & BUTTON_RIGHT_TRIGGER_MASK;

    ps4_output.button_left_joystick = gp->buttons & BUTTON_LEFT_JOYSTICK_MASK;
    ps4_output.button_right_joystick = gp->buttons & BUTTON_RIGHT_JOYSTICK_MASK;

    ps4_output.button_share = gp->buttons & BUTTON_SHARE_MASK;
    ps4_output.axis_lx = gp->axis_x >> 2;
    ps4_output.axis_ly = gp->axis_y >> 2;
    ps4_output.axis_rx = gp->axis_rx >> 2;
    ps4_output.axis_ry = gp->axis_ry >> 2;
    ps4_output.gyro_x = gp->gyro[0];
    ps4_output.gyro_y = gp->gyro[1];
    ps4_output.gyro_z = gp->gyro[2];
    ps4_output.accel_x = gp->accel[0];
    ps4_output.accel_y = gp->accel[1];
    ps4_output.accel_z = gp->accel[2];
    // rumble and led not used on output
    ps4_output.rumble = std::nullopt;
    ps4_output.led_rgb = std::nullopt;

    emit(Ps4Event{.serdes = PublishSerdes{ps4_output}});
}

void Ps4Actor::on_controller_data(uni_hid_device_t *d, uni_controller_t *ctl)
{
    Ps4Actor *ps = get_my_platform_instance(d);
    uni_gamepad_t *gp;
    if (ctl->klass == UNI_CONTROLLER_CLASS_GAMEPAD)
    {
        gp = &ctl->gamepad;
        ps->gamepad_to_output(gp);
    }
}

const uni_property_t *Ps4Actor::get_property(uni_property_idx_t idx)
{
    ARG_UNUSED(idx);
    return NULL;
}

void Ps4Actor::on_oob_event(uni_platform_oob_event_t event, void *data)
{
    switch (event)
    {
    case UNI_PLATFORM_OOB_GAMEPAD_SYSTEM_BUTTON:
    {
        uni_hid_device_t *d = (uni_hid_device_t *)data;

        if (d == NULL)
        {
            loge("ERROR: my_platform_on_oob_event: Invalid NULL device");
            return;
        }
        INFO("custom: on_device_oob_event(): %d", event);

        Ps4Actor *ps = get_my_platform_instance(d);
        // ins->gamepad_seat = ins->gamepad_seat == GAMEPAD_SEAT_A ? GAMEPAD_SEAT_B : GAMEPAD_SEAT_A;
        ps->emit(Ps4Event{.blue_event = OOB_EVENT});
        trigger_event_on_gamepad(d);
        break;
    }

    case UNI_PLATFORM_OOB_BLUETOOTH_ENABLED:
        INFO("custom: Bluetooth enabled: %d", (bool)(data));
        break;

    default:
        INFO("my_platform_on_oob_event: unsupported event: 0x%04x", event);
        break;
    }
}

//
// Helpers
//

void Ps4Actor::trigger_event_on_gamepad(uni_hid_device_t *d)
{
    // Ps4Actor *ps = get_my_platform_instance(d);

    if (d->report_parser.play_dual_rumble != NULL)
    {
        d->report_parser.play_dual_rumble(d, 0 /* delayed start ms */, 150 /* duration ms */, 128 /* weak magnitude */,
                                          40 /* strong magnitude */);
    }

    if (d->report_parser.set_player_leds != NULL)
    {
        //   d->report_parser.set_player_leds(d, ins->gamepad_seat);
    }

    if (d->report_parser.set_lightbar_color != NULL)
    {
        /*  uint8_t red = (ins->gamepad_seat & 0x01) ? 0xff : 0;
          uint8_t green = (ins->gamepad_seat & 0x02) ? 0xff : 0;
          uint8_t blue = (ins->gamepad_seat & 0x04) ? 0xff : 0;
          d->report_parser.set_lightbar_color(d, red, green, blue);*/
    }
}

void Ps4Actor::on_gamepad_data(uni_hid_device_t *d, uni_gamepad_t *gp)
{
    Ps4Actor *ps = get_my_platform_instance(d);
    ps->gamepad_to_output(gp);
}

//
// Entry Point
//
struct uni_platform *Ps4Actor::get_my_platform(void)
{
    static struct uni_platform plat = {
        .name = "custom",
        .init = Ps4Actor::init,
        .on_init_complete = Ps4Actor::on_init_complete,
        .on_device_discovered = Ps4Actor::on_device_discovered,
        .on_device_connected = Ps4Actor::on_device_connected,
        .on_device_disconnected = Ps4Actor::on_device_disconnected,
        .on_device_ready = Ps4Actor::on_device_ready,
        .on_gamepad_data = Ps4Actor::on_gamepad_data,
        .on_controller_data = Ps4Actor::on_controller_data,
        .get_property = Ps4Actor::get_property,
        .on_oob_event = Ps4Actor::on_oob_event,
        .device_dump = 0,
        .register_console_cmds = 0,
    };

    return &plat;
}

Ps4Actor *Ps4Actor::ps4_actor_instance = 0;

extern "C" Ps4Actor *get_my_platform_instance(uni_hid_device_t *d)
{
    return Ps4Actor::ps4_actor_instance;
}
