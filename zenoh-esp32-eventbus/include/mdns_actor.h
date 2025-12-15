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


class MdnsActor : public Actor
{
private:

std::string _hostname;

public:
  MdnsActor(const char *name,const char* hostname);
  ~MdnsActor();
  void on_message(const Envelope &);
  void on_start();
  void init();
};
