#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "log.h"
#include "actor.h"
#include <wifi_actor.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <string.h>
#include <lwip/def.h>
#include <lwip/inet.h>
#include <msgs.h>

class McActor : public Actor
{
private:
  std::string _hostname;
  int _unicast_socket;
  int _multicast_socket;
  sockaddr_in _unicast_src_addr;
  sockaddr_in _multicast_addr;
  bool _running = true;
  int _timer_publish;
  uint32_t _ping_counter = 0;
  bool _connected = false;
  std::unordered_map<std::string, std::pair<sockaddr_in,uint64_t>> _source_map;
  std::optional<sockaddr_in> _broker_addr = std::nullopt;
  std::optional<std::string> _broker_name = std::nullopt;
  uint32_t _last_ping_number = 1;

public:
  McActor(const char *name, const char *hostname);
  ~McActor();
  void on_message(const Envelope &);
  void on_start();
  void init_event();
  void stop_event();
  void start_unicast_listener();
  void start_multicast_listener();
  void stop_listener();
  void send_unicast(const char *dst, const char *src, const char *msg_type, const Bytes &bytes);
  void send_multicast(const char *dst, const char *src, const char *msg_type, const Bytes &bytes);
  void on_udp(const Bytes &request, const sockaddr_in &sender_addr);
  void on_udp_message(UdpMessage& udp_message,const sockaddr_in &sender_addr);
  void send_ping_req(const char* dst,uint32_t number);
  void send_ping_rep(const char* dst,uint32_t number);
  Bytes encode_message(const char *dst, const char *src, const char *type, const Bytes &payload);
  static void unicast_listener_task(void *param);
  static void multicast_listener_task(void *param);
};
