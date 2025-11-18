#ifndef PS4_ACTOR_H
#define PS4_ACTOR_H
/*
https://bluepad32.readthedocs.io/en/latest/plat_esp32/


*/

#include <actor.h>
#include <msg_info.h>
#include <esp_event.h>
#include <esp_wifi.h>
//#include <esp_bt.h>

#include <btstack_port_esp32.h>
#include <btstack_run_loop.h>
#include <btstack_stdio_esp32.h>
#include <hci_dump.h>
#include <hci_dump_embedded_stdout.h>
#include <uni.h>
#include <limero.h>

// #include "sdkconfig.h"

// Sanity check
#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Must use BLUEPAD32_PLATFORM_CUSTOM"
#endif
/*
typedef enum Ps4Props {
  BUTTON_LEFT = 0,
  BUTTON_RIGHT,
  BUTTON_UP,
  BUTTON_DOWN,

  BUTTON_SQUARE,
  BUTTON_CROSS,
  BUTTON_CIRCLE,
  BUTTON_TRIANGLE,

  BUTTON_LEFT_SHOULDER, // 8
  BUTTON_RIGHT_SHOULDER,
  BUTTON_LEFT_TRIGGER,
  BUTTON_RIGHT_TRIGGER,
  BUTTON_LEFT_JOYSTICK,
  BUTTON_RIGHT_JOYSTICK,
  BUTTON_SHARE,

  STICK_LEFT_X, // 15
  STICK_LEFT_Y,
  STICK_RIGHT_X,
  STICK_RIGHT_Y,

  GYRO_X, // 19
  GYRO_Y,
  GYRO_Z,

  ACCEL_X, // 22
  ACCEL_Y,
  ACCEL_Z,

  RUMBLE,
  LIGHTBAR_RGB

} Ps4Props;


struct Ps4Msg : public Serializable
{
  std::optional<bool> button_left = std::nullopt;
  std::optional<bool> button_right = std::nullopt;
  std::optional<bool> button_up = std::nullopt;
  std::optional<bool> button_down = std::nullopt;

  std::optional<bool> button_square = std::nullopt;
  std::optional<bool> button_cross = std::nullopt;
  std::optional<bool> button_circle = std::nullopt;
  std::optional<bool> button_triangle = std::nullopt;

  std::optional<bool> button_left_sholder = std::nullopt;
  std::optional<bool> button_right_sholder = std::nullopt;
  std::optional<bool> button_left_trigger = std::nullopt;
  std::optional<bool> button_right_trigger = std::nullopt;
  std::optional<bool> button_left_joystick = std::nullopt;
  std::optional<bool> button_right_joystick = std::nullopt;
  std::optional<bool> button_share = std::nullopt;

  std::optional<int> axis_lx = std::nullopt;
  std::optional<int> axis_ly = std::nullopt;
  std::optional<int> axis_rx = std::nullopt;
  std::optional<int> axis_ry = std::nullopt;

  std::optional<int> gyro_x = std::nullopt;
  std::optional<int> gyro_y = std::nullopt;
  std::optional<int> gyro_z = std::nullopt;

  std::optional<int> accel_x = std::nullopt;
  std::optional<int> accel_y = std::nullopt;
  std::optional<int> accel_z = std::nullopt;

  std::optional<int> rumble = std::nullopt;
  std::optional<int> led_rgb = std::nullopt;

  ~Ps4Msg()
  {
    INFO("Ps4Msg destructor");
  }

  Res deserialize(Deserializer &des) override
  {
    return Res::Err(EAFNOSUPPORT, "Not implemented");
  }
  Res serialize(Serializer &ser) override
  {
    ser.reset();
    ser.map_begin();
    ser.serialize("blw",button_left); //0
    ser.serialize("ble",button__right);
    ser.serialize("bln",button_up);
    ser.serialize("bls",button_down);

    ser.serialize("brw",button_square); // 4
    ser.serialize("brs",button_cross);
    ser.serialize("bre",button_circle);
    ser.serialize("brn",button_triangle);

    ser.serialize("bsl",button_left_sholder); // 8
    ser.serialize("bsr",button_right_sholder);
    ser.serialize("btl",button_left_trigger);
    ser.serialize("btr",button_right_trigger);

    ser.serialize("bjl",button_left_joystick); // 12
    ser.serialize("bjr",button_right_joystick);

    ser.serialize(button_share); // 14
    ser.serialize(axis_lx); // 15
    ser.serialize(axis_ly);
    ser.serialize(axis_rx);
    ser.serialize(axis_ry);
    ser.serialize(gyro_x); // 19
    ser.serialize(gyro_y);
    ser.serialize(gyro_z);
    ser.serialize(accel_x); // 22
    ser.serialize(accel_y);
    ser.serialize(accel_z);
    ser.serialize(rumble); // 25
    ser.serialize(led_rgb);
    return ser.array_end();
  }
};

struct Ps4Input
{
  std::optional<uni_hid_device_t *> device = std::nullopt;
};

struct Ps4Cmd
{
  std::optional<PublishSerdes> serdes = std::nullopt;
  std::optional<bool> stop_actor = std::nullopt;
};*/

typedef enum BlueEvent
{
  DEVICE_DISCOVERED,
  DEVICE_CONNECTED,
  DEVICE_DISCONNECTED,
  DEVICE_READY,
  CONTROLLER_DATA,
  OOB_EVENT

} BlueEvent;

DEFINE_MSG(Ps4Event,
    BlueEvent blue_event;
    Ps4Event(BlueEvent ev) : blue_event(ev) {}
);



class Ps4Actor : public Actor
{
  int _timer_id = 0;
  int _prop_counter = 0;

public:
  Ps4Actor(const char* name);
  ~Ps4Actor();
  void on_cmd(Ps4Cmd &cmd);
  void on_timer(int timer_id);
  void on_start();

  void gamepad_to_output(uni_gamepad_t *gp);

  static Ps4Actor *ps4_actor_instance;

  static void init(int argc, const char **argv);
  static void on_init_complete(void);
  static uni_error_t on_device_discovered(bd_addr_t addr, const char *name, uint16_t cod, uint8_t rssi);
  static void on_device_connected(uni_hid_device_t *d);
  static void on_device_disconnected(uni_hid_device_t *d);
  static uni_error_t on_device_ready(uni_hid_device_t *d);
  static void on_gamepad_data(uni_hid_device_t *d, uni_gamepad_t *gp);
  static void on_controller_data(uni_hid_device_t *d, uni_controller_t *ctl);
  static const uni_property_t *get_property(uni_property_idx_t idx);
  static void on_oob_event(uni_platform_oob_event_t event, void *data);
  static struct uni_platform *get_my_platform(void);
  static void trigger_event_on_gamepad(uni_hid_device_t *d);
  //  static  Ps4Actor *get_my_platform_instance(uni_hid_device_t *d);
};

extern "C" Ps4Actor *get_my_platform_instance(uni_hid_device_t *d);

#endif