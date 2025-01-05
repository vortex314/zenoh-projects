#ifndef MSG_SYS_H
#define MSG_SYS_H
#include <serdes.h>
#include <string>
#include <vector>
#include <optional>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_timer.h>
#include <esp_flash.h>

#include <msg_info.h>

  const static InfoProp info_props_sys_msg[8] = {
    InfoProp(0, "cpu", "CPU", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(1, "clock", "Clock", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(2, "flash_size", "Flash Size", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(3, "ram_size", "RAM Size", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(4, "free_heap", "Free Heap", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(5, "up_time", "Up Time", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(6, "log_message", "Log Message", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(7, "state", "State", PropType::PROP_STR, PropMode::PROP_READ),
  };

struct SysMsg : public Serializable {
public:
  // SYSTEM
  std::optional<std::string> cpu=std::nullopt;
  std::optional<uint32_t> clock=std::nullopt;
  std::optional<uint32_t> flash_size=std::nullopt;
  std::optional<uint32_t> ram_size=std::nullopt;
  std::optional<uint32_t> free_heap=std::nullopt;      // dynamic
  std::optional<uint64_t> up_time=std::nullopt;        // dynamic
  std::optional<std::string> log_message=std::nullopt; // dynamic
  std::optional<std::string> state=std::nullopt;       // dynamic

  Res serialize(Serializer &ser) {
    int idx = 0;
    ser.reset();
    ser.serialize(idx++, cpu);
    ser.serialize(idx++, clock);
    ser.serialize(idx++, flash_size);
    ser.serialize(idx++, ram_size);
    ser.serialize(idx++, free_heap);
    ser.serialize(idx++, up_time);
    ser.serialize(idx++, log_message);
    return ser.serialize(idx++, state);
  }

  Res deserialize(Deserializer &des) {
    des.deserialize(cpu);
    des.deserialize(clock);
    des.deserialize(flash_size);
    des.deserialize(ram_size);
    des.deserialize(free_heap);
    des.deserialize(up_time);
    des.deserialize(log_message);
    return des.deserialize(state);
  }

  Res fill() {
    cpu = "ESP32";
    clock = 240000000;
    /*esp_flash_t chip;
    esp_flash_init(&chip);
    uint32_t size;
    RET_ERRI(esp_flash_get_size(&chip, &size), "Failed to get flash size");
    flash_size = size;*/
    ram_size = esp_get_free_heap_size();
    up_time = esp_timer_get_time();
    return Res::Ok();
  }

  const InfoProp* info(int idx) {
    if ( idx >= sizeof(info_props_sys_msg)/sizeof(InfoProp)) return nullptr;
    return &info_props_sys_msg[idx];
  }
};

#endif