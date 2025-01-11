#include <ps4_actor.h>

#define BUTTON_SQUARE 0x04
#define BUTTON_CROSS 0x01
#define BUTTON_CIRCLE 0x02
#define BUTTON_TRIANGLE 0x08

#define BUTTON_LEFT_SHOULDER 0x10
#define BUTTON_RIGHT_SHOULDER 0x20
#define BUTTON_LEFT_TRIGGER 0x40
#define BUTTON_RIGHT_TRIGGER 0x80

#define BUTTON_LEFT_JOYSTICK 0x100
#define BUTTON_RIGHT_JOYSTICK 0x200
#define BUTTON_SHARE 0x400

Ps4Actor::Ps4Actor() : Actor<Ps4Event, Ps4Cmd>(6120, "ps4", 5, 10)
{
    Ps4Actor::ps4_actor_instance = this;
    INFO("Starting PS4 actor sizeof(Ps4Cmd ) : %d ", sizeof(Ps4Cmd));
    add_timer(Timer::Repetitive(1, 1000));
}

void Ps4Actor::on_start()
{
    btstack_stdio_init();
    // Configure BTstack for ESP32 VHCI Controller
    btstack_init();

    // Must be called before uni_init()
    uni_platform_set_custom(get_my_platform());

    // Init Bluepad32.
    uni_init(0 /* argc */, NULL /* argv */);

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
    switch (timer_id)
    {
    case 1:
    {
        emit(Ps4Event{});
        break;
    }
    default:
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

    INFO("custom: init()\n");

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
    INFO("custom: on_init_complete()\n");

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
        INFO("Ignoring keyboard\n");
        return UNI_ERROR_IGNORE_DEVICE;
    }
    // emit(Ps4Event{ .blue_event = DEVICE_DISCOVERED });
    return UNI_ERROR_SUCCESS;
}

void Ps4Actor::on_device_connected(uni_hid_device_t *d)
{
    INFO("custom: device connected: %p\n", d);
    Ps4Actor *ps = get_my_platform_instance(d);
    ps->emit(Ps4Event{.blue_event = DEVICE_CONNECTED});
}

void Ps4Actor::on_device_disconnected(uni_hid_device_t *d)
{
    INFO("custom: device disconnected: %p\n", d);
    Ps4Actor *ps = get_my_platform_instance(d);
    ps->emit(Ps4Event{.blue_event = DEVICE_DISCONNECTED});
}

uni_error_t Ps4Actor::on_device_ready(uni_hid_device_t *d)
{
    INFO("custom: device ready: %p\n", d);
    Ps4Actor *ps = get_my_platform_instance(d);
    ps->emit(Ps4Event{.blue_event = DEVICE_READY});

    trigger_event_on_gamepad(d);
    return UNI_ERROR_SUCCESS;
}

void Ps4Actor::gamepad_to_output(uni_gamepad_t *gp)
{
    ps4_output.dpad = gp->dpad;

    ps4_output.button_square = gp->buttons & BUTTON_SQUARE;
    ps4_output.button_cross = gp->buttons & BUTTON_CROSS;
    ps4_output.button_circle = gp->buttons & BUTTON_CIRCLE;
    ps4_output.button_triangle = gp->buttons & BUTTON_TRIANGLE;

    ps4_output.button_left_sholder = gp->buttons & BUTTON_LEFT_SHOULDER;
    ps4_output.button_right_sholder = gp->buttons & BUTTON_RIGHT_SHOULDER;
    ps4_output.button_left_trigger = gp->buttons & BUTTON_LEFT_TRIGGER;
    ps4_output.button_right_trigger = gp->buttons & BUTTON_RIGHT_TRIGGER;

    ps4_output.button_left_joystick = gp->buttons & BUTTON_LEFT_JOYSTICK;
    ps4_output.button_right_joystick = gp->buttons & BUTTON_RIGHT_JOYSTICK;

    ps4_output.button_share = gp->buttons & BUTTON_SHARE;
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

    emit(Ps4Event{.serdes = PublishSerdes{"ps4", ps4_output}});
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
            loge("ERROR: my_platform_on_oob_event: Invalid NULL device\n");
            return;
        }
        INFO("custom: on_device_oob_event(): %d\n", event);

        Ps4Actor *ps = get_my_platform_instance(d);
        // ins->gamepad_seat = ins->gamepad_seat == GAMEPAD_SEAT_A ? GAMEPAD_SEAT_B : GAMEPAD_SEAT_A;
        ps->emit(Ps4Event{.blue_event = OOB_EVENT});
        trigger_event_on_gamepad(d);
        break;
    }

    case UNI_PLATFORM_OOB_BLUETOOTH_ENABLED:
        INFO("custom: Bluetooth enabled: %d\n", (bool)(data));
        break;

    default:
        INFO("my_platform_on_oob_event: unsupported event: 0x%04x\n", event);
        break;
    }
}

//
// Helpers
//

void Ps4Actor::trigger_event_on_gamepad(uni_hid_device_t *d)
{
    Ps4Actor *ps = get_my_platform_instance(d);

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
