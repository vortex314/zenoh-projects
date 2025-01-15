/*
https://bluepad32.readthedocs.io/en/latest/plat_esp32/


*/

#include <actor.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_bt.h>

#include <btstack_port_esp32.h>
#include <btstack_run_loop.h>
#include <btstack_stdio_esp32.h>
#include <hci_dump.h>
#include <hci_dump_embedded_stdout.h>
#include <uni.h>

// #include "sdkconfig.h"

// Sanity check
#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Must use BLUEPAD32_PLATFORM_CUSTOM"
#endif

/*
 {Ps4::BUTTON_SQUARE, "button_square", "Square button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_CROSS, "button_cross", "Cross button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_CIRCLE, "button_circle", "Circle button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_TRIANGLE, "button_triangle", "Triangle button", ValueType::UINT, ValueMode::READ},

    {Ps4::BUTTON_LEFT_SHOULDER, "button_left_shoulder", "Left Shoulder button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_RIGHT_SHOULDER, "button_right_shoulder", "Right Shoulder button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_LEFT_TRIGGER, "button_left_trigger", "Left Trigger button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_RIGHT_TRIGGER, "button_right_trigger", "Right Trigger button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_LEFT_JOYSTICK, "button_left_joystick", "Left Joystick button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_RIGHT_JOYSTICK, "button_right_joystick", "Right Joystick button", ValueType::UINT, ValueMode::READ},
    {Ps4::BUTTON_SHARE, "button_share", "Share button", ValueType::UINT, ValueMode::READ},

    {Ps4::STICK_LEFT_X, "axis_x", "Left Stick X", ValueType::INT, ValueMode::READ},
    {Ps4::STICK_LEFT_Y, "axis_y", "Left Stick Y", ValueType::INT, ValueMode::READ},
    {Ps4::STICK_RIGHT_X, "axis_rx", "Right Stick X", ValueType::INT, ValueMode::READ},
    {Ps4::STICK_RIGHT_Y, "axis_ry", "Right Stick Y", ValueType::INT, ValueMode::READ},

    {Ps4::GYRO_X, "gyro_x", "Gyro X axis", ValueType::INT, ValueMode::READ},
    {Ps4::GYRO_Y, "gyro_y", "Gyro Y axis", ValueType::INT, ValueMode::READ},
    {Ps4::GYRO_Z, "gyro_z", "Gyro Z axis", ValueType::INT, ValueMode::READ},

    {Ps4::ACCEL_X, "accel_x", "Accelerometer X Axis ", ValueType::INT, ValueMode::READ},
    {Ps4::ACCEL_Y, "accel_y", "Accelerometer Y Axis ", ValueType::INT, ValueMode::READ},
    {Ps4::ACCEL_Z, "accel_z", "Accelerometer Z Axis ", ValueType::INT, ValueMode::READ},

    {Ps4::RUMBLE, "rumble", "Rumble", ValueType::UINT, ValueMode::WRITE},
    {Ps4::LIGHTBAR_RGB, "led_green", "Green LED", ValueType::UINT, ValueMode::WRITE}

*/

struct Ps4Output : public Serializable
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

  ~Ps4Output() override {
    INFO("Ps4Output destructor");
  }

  Res deserialize(Deserializer &des) override
  {
    return Res::Err(EAFNOSUPPORT, "Not implemented");
  }
  Res serialize(Serializer &ser) override
  {
    ser.reset();
    ser.array_begin();
    ser.serialize(dpad);

    ser.serialize(button_square);
    ser.serialize(button_cross);
    ser.serialize(button_circle);
    ser.serialize(button_triangle);

    ser.serialize(button_left_sholder);
    ser.serialize(button_right_sholder);
    ser.serialize(button_left_trigger);
    ser.serialize(button_right_trigger);

    ser.serialize(button_left_joystick);
    ser.serialize(button_right_joystick);
    
    ser.serialize(button_share);
    ser.serialize(axis_lx);
    ser.serialize(axis_ly);
    ser.serialize(axis_rx);
    ser.serialize(axis_ry);
    ser.serialize(gyro_x);
    ser.serialize(gyro_y);
    ser.serialize(gyro_z);
    ser.serialize(accel_x);
    ser.serialize(accel_y);
    ser.serialize(accel_z);
    ser.serialize(rumble);
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
  std::optional<PublishSerdes> publish = std::nullopt;
  std::optional<bool> stop_actor = std::nullopt;
};

typedef enum BlueEvent
{
  DEVICE_DISCOVERED,
  DEVICE_CONNECTED,
  DEVICE_DISCONNECTED,
  DEVICE_READY,
  CONTROLLER_DATA,
  OOB_EVENT

} BlueEvent;

struct Ps4Event
{
  std::optional<PublishSerdes> serdes = std::nullopt;
  std::optional<BlueEvent> blue_event = std::nullopt;
  std::optional<Ps4Output> output = std::nullopt;
};

class Ps4Actor : public Actor<Ps4Event, Ps4Cmd>
{
public:
  Ps4Actor();
  ~Ps4Actor();
  void on_cmd(Ps4Cmd &cmd);
  void on_timer(int timer_id);
  void on_start();
  Ps4Output ps4_output  ;

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

 extern "C"   Ps4Actor *get_my_platform_instance(uni_hid_device_t *d);

