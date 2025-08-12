#ifndef ZENOH_ACTOR_H
#define ZENOH_ACTOR_H

#include "json.h"
#include <actor.h>
#include <functional>
#include <optional>
#include <serdes.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <log.h>
#include <msg_info.h>
#include <map>
#include <util.h>
#include <result.h>
#include <option.h>
#include <value.h>
#include <memory>
#include <freertos/FreeRTOS.h>
#include <freertos/mpu_wrappers.h>
#include <freertos/portmacro.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <wifi_actor.h>
#include "esp_netif.h"
#include <led_actor.h>

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// #pragma GCC diagnostic ignored "-Wunused-variable"

// Multicast configuration
#define MULTICAST_IP "225.0.0.1"
#define MULTICAST_PORT 6502
#define MAX_UDP_PACKET_SIZE 1024

extern std::string ip4addr_to_str(esp_ip4_addr_t *ip);

MSG(McSend, std::string topic; Value value; McSend(const std::string &topic, const Value &value) : topic(topic), value(value){});

void send();

class McActor : public Actor
{
private:
  bool _connected = false;
  int _timer_publish = -1;
  int _prop_counter = 0;
  int _mc_socket = -1;  // socket for multicast communication
  int _udp_socket = -1; // socket for UDP communication
  TaskHandle_t _task_handle = NULL;
  unsigned char _rx_buffer[MAX_UDP_PACKET_SIZE];

public:
  McActor(const char *name);
  ~McActor();
  void run();
  void on_message(const Msg &message) override;
  void on_start() override;
  void on_wifi_connected();
  void on_wifi_disconnected();
  void on_timer(int id);

  void prefix(const char *prefix);
  bool is_connected() const;
  Result<Void> connect(void);
  Result<Void> disconnect();
  Result<Bytes> receive();
  Result<Void> send(const std::string &data);
  Result<Void> publish_props();

  Result<Void> subscribe(const std::string &topic);
  void get_props(Value &v) const;
  Result<TaskHandle_t> start_receiver_task();
  static void receiver_task(void *);
};
#endif