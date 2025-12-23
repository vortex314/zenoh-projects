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

class McActor : public Actor
{
private:
  std::string _hostname;
  int _event_socket;
  int _req_reply_socket;
  sockaddr_in _event_addr;
  sockaddr_in _req_reply_addr;
  bool _running = true;
  int _timer_publish;
  uint32_t _ping_counter = 0;
  bool _connected = false;

public:
  McActor(const char *name, const char *hostname);
  ~McActor();
  void on_message(const Envelope &);
  void on_start();
  void init_event();
  void stop_event();
  void start_listener();
  void stop_listener();
  void send_event(const char *dst, const char *src, const char *msg_type, const Bytes &bytes);
  void send_request_reply(const char *dst, const char *src, const char *msg_type, const Bytes &bytes);
  void on_request(const Bytes &request, const sockaddr_in &sender_addr);
  void on_message(const char *type,Bytes& payload);
  void send_ping();
  Bytes encode_message(const char *dst, const char *src, const char *type, const Bytes &payload);
  static void udp_listener_task(void *param);
};
