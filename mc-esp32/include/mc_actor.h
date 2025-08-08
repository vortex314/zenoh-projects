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

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// #pragma GCC diagnostic ignored "-Wunused-variable"

// Multicast configuration
#define MULTICAST_IP "225.0.0.1"
#define MULTICAST_PORT 6502
#define MAX_UDP_PACKET_SIZE 1024

extern std::string ip4addr_to_str(esp_ip4_addr_t *ip);

struct McSend : Msg
{
  constexpr static uint32_t _id = FILE_LINE_HASH;
  uint32_t type_id() const { return _id; }
  McSend() = default;
  McSend(const std::string &topic, const Value &value) : topic(topic), value(value) {}
  std::string topic;
  Value value;
};

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
  McActor();
  McActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~McActor();
  void run();
  void on_timer(int id);
  void on_cmd(const Value &);
  void on_start() override;
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