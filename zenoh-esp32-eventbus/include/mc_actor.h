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
  sockaddr_in _event_addr;
  int _req_reply_socket;

public:
  McActor(const char *name, const char *hostname);
  ~McActor();
  void on_message(const Envelope &);
  void on_start();
  void init();
  void send_msg(const char *dst, const char *src, const char *msg_type, const Bytes &bytes);
};
