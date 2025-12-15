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


#define TFTP_PORT          69
#define TFTP_BLOCK_SIZE    512
#define TFTP_TIMEOUT_MS    5000

#define TFTP_OPCODE_RRQ    1
#define TFTP_OPCODE_WRQ    2
#define TFTP_OPCODE_DATA   3
#define TFTP_OPCODE_ACK    4
#define TFTP_OPCODE_ERROR  5


class OtaActor : public Actor
{
private:

    TaskHandle_t _ota_task_handle = nullptr;

public:
  OtaActor(const char *name);
  ~OtaActor();
  void on_message(const Envelope &);
  void on_start();
  static void tftp_ota_server_task(void *arg);

};
