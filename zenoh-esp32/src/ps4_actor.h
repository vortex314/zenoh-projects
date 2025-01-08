/*
https://bluepad32.readthedocs.io/en/latest/plat_esp32/


*/

#include <actor.h>
#include <esp_event.h>
#include <esp_wifi.h>

#include <btstack_port_esp32.h>
#include <btstack_run_loop.h>
#include <btstack_stdio_esp32.h>
#include <hci_dump.h>
#include <hci_dump_embedded_stdout.h>
#include <uni.h>

struct Ps4Output : public Serializable
{
  std::optional<bool> button_a = std::nullopt;
  std::optional<bool> button_b = std::nullopt;
  std::optional<bool> button_x = std::nullopt;
  std::optional<bool> button_y = std::nullopt;
  std::optional<bool> button_l1 = std::nullopt;
  std::optional<bool> button_l2 = std::nullopt;
  std::optional<bool> button_r1 = std::nullopt;
  std::optional<bool> button_r2 = std::nullopt;
  std::optional<bool> button_select = std::nullopt;
  std::optional<bool> button_start = std::nullopt;
  std::optional<bool> button_l3 = std::nullopt;
  std::optional<bool> button_r3 = std::nullopt;
  std::optional<bool> button_up = std::nullopt;
  std::optional<bool> button_down = std::nullopt;
  std::optional<bool> button_left = std::nullopt;
  std::optional<bool> button_right = std::nullopt;
  std::optional<bool> button_ps = std::nullopt;
  std::optional<bool> button_touchpad = std::nullopt;
  std::optional<int> left_x = std::nullopt;
  std::optional<int> left_y = std::nullopt;
  std::optional<int> right_x = std::nullopt;
  std::optional<int> right_y = std::nullopt;

  Res deserialize(Deserializer &des) override
  {

    return Res::Err(EAFNOSUPPORT, "Not implemented");
  }
  Res serialize(Serializer &ser) override
  {
    ser.reset();
    ser.map_begin();
    ser.serialize(0, button_a);
    ser.serialize(1, button_b);
    ser.serialize(2, button_x);
    ser.serialize(3, button_y);
    ser.serialize(4, button_l1);
    ser.serialize(5, button_l2);
    ser.serialize(6, button_r1);
    ser.serialize(7, button_r2);
    ser.serialize(8, button_select);
    ser.serialize(9, button_start);
    ser.serialize(10, button_l3);
    ser.serialize(11, button_r3);
    ser.serialize(12, button_up);
    ser.serialize(13, button_down);
    ser.serialize(14, button_left);
    ser.serialize(15, button_right);
    ser.serialize(16, button_ps);
    ser.serialize(17, button_touchpad);
    ser.serialize(18, left_x);
    ser.serialize(19, left_y);
    ser.serialize(20, right_x);
    ser.serialize(21, right_y);
    return ser.map_end();
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
  std::optional<PublishSerdes> publish = std::nullopt;
  std::optional<BlueEvent> blue_event = std::nullopt;
};

class Ps4Actor : public Actor<Ps4Event, Ps4Cmd>
{
public:
  Ps4Actor();
  ~Ps4Actor();
  void on_cmd(Ps4Cmd &cmd);
  void on_timer(int timer_id);
  void on_start();

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
};

  static  Ps4Actor *get_my_platform_instance(uni_hid_device_t *d);

