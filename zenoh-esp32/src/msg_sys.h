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

  
};

#endif