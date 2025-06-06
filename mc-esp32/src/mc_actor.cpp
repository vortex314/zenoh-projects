#include <mc_actor.h>

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

static int create_multicast_socket();
static void send_multicast_json(int sock, const Bytes &data);
static void receive_multicast_messages(void *pvParameters);

// Multicast configuration
#define MULTICAST_IP "225.0.0.1"
#define MULTICAST_PORT 6502
#define MAX_UDP_PACKET_SIZE 1024

/*InfoProp info_props_zenoh[] = {
    InfoProp(0, "zid", "Mc ID", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(1, "what_am_i", "What am I", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(2, "peers", "Peers", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(3, "prefix", "Prefix", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(4, "routers", "Routers", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(5, "connect", "Connect", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(6, "listen", "Listen", PropType::PROP_STR, PropMode::PROP_READ),
};*/

McActor::McActor() : McActor("zenoh", 4096, 5, 5) {}

McActor::McActor(const char *name, size_t stack_size, int priority, size_t queue_depth)
    : Actor<McEvent, McCmd>(stack_size, name, priority, queue_depth)
{
  _timer_publish = timer_repetitive(1000); // timer for publishing properties
}

void McActor::on_timer(int id)
{
  if (id == _timer_publish)
  {
    publish_props();
  }
  else
    INFO("Unknown timer id: %d", id);
}

void McActor::on_cmd(McCmd &cmd)
{
  if (cmd.action)
  {
    switch (cmd.action.value())
    {
    case McAction::Subscribe:
      subscribe(cmd.topic.value());
      break;
    case McAction::Connect:
      INFO("Connecting to Mc...");
      if (!_connected)
      {
        Result res = connect();
        if (res.is_err())
        {
          INFO("Failed to connect to Mc: %s", res.msg());
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          tell(new McCmd{.action = McAction::Connect});
        }
        else
        {
          INFO("Connected to Mc.");
          for (const auto &topic : _subscribed_topics)
          {
            auto r = subscribe(topic);
            if (r.is_err())
            {
              INFO("Failed to subscribe to topic: %s", topic.c_str());
            }
            else
            {
              INFO("Subscribed to topic: %s", topic.c_str());
            }
          }
        }
      }
      break;
    case McAction::Disconnect:
      INFO("Disconnecting from Mc...");
      if (_connected)
      {
        disconnect();
      }
      break;
    case McAction::Stop:
      INFO("Stopping McActor...");
      return;
    }
  }
  if (cmd.publish_bytes && _connected)
  {
    send_multicast_json(_socket, cmd.publish_bytes.value().payload);
  }
}

Res McActor::connect(void)
{
  _socket = create_multicast_socket();
  if (_socket < 0)
  {
    return Res(-1, "Failed to create multicast socket");
  }
  xTaskCreate(receive_multicast_messages, "udp_rx", 4096, NULL, 5, NULL);

  return ResOk;
}

Res McActor::disconnect()
{
  INFO("Closing Mc Session...");

  _connected = false;

  INFO("Mc session closed ");
  return ResOk;
}

bool McActor::is_connected() const
{
  return _connected;
}

void McActor::prefix(const char *prefix)
{
  _src_prefix = "src/";
  _src_prefix += prefix;
  _dst_prefix = "dst/";
  _dst_prefix += prefix;
  _subscribed_topics.push_back(_dst_prefix + "/**");
}

Res McActor::subscribe(const std::string &topic)
{
  if (_connected)
  {
  }
  else
  {
    return Res(-1, "Not connected to Mc");
  }
  return Res(true);
}

McActor::~McActor()
{
  INFO("Closing Mc Session...");

  INFO("Mc session closed ");
}

Res McActor::publish_props()
{
  if (!_connected)
  {
    return Res(ENOTCONN, "Not connected to Mc");
  }
  return ResOk;
}

//============================================================

Res McMsg::serialize(Serializer &ser) const
{
  //  int idx = 0;
  ser.reset();
  ser.map_begin();
  ser.serialize(KEY("zid"), zid);
  ser.serialize(KEY("what_am_i"), what_am_i);
  ser.serialize(KEY("peers"), peers);
  ser.serialize(KEY("prefix"), prefix);
  ser.serialize(KEY("routers"), routers);
  ser.serialize(KEY("connect"), connect);
  return ser.map_end();
}

Res McMsg::deserialize(Deserializer &des)
{
  return Res(EAFNOSUPPORT, "not implemented");
}

static int create_multicast_socket()
{
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0)
  {
    ERROR( "Failed to create socket: errno %d : %s", errno, strerror(errno));
    return -1;
  }

  // Set socket options
  int opt = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // Bind the socket to any address
  struct sockaddr_in saddr;
  BZERO(saddr);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(MULTICAST_PORT);
  saddr.sin_addr = (struct in_addr){
      .s_addr = htonl(INADDR_ANY) // Bind to all interfaces
  };

  T_ESP(bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)));

  // Get the actual network interface IP address
  esp_netif_ip_info_t ip_info;
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (netif == NULL)
  {
    ERROR( "Failed to get network interface");
    close(sock);
    return -1;
  }
  if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK)
  {
    ERROR( "Failed to get IP info");
    close(sock);
    return -1;
  }
  // Join multicast group
  struct ip_mreq mreq;
  BZERO(mreq);
  mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
  mreq.imr_interface.s_addr = ip_info.ip.addr; // Use the actual interface IP address
  // mreq.imr_interface.s_addr = htonl(INADDR_ANY); // Use INADDR_ANY to join on all interfaces

  int rc = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
  if (rc < 0)
  {
    ERROR( "Failed to join multicast group: errno %d : %s", rc, strerror(errno));
    close(sock);
    return -1;
  }

  // Set multicast interface
  struct in_addr if_addr;
  if_addr.s_addr = htonl(INADDR_ANY);
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &if_addr, sizeof(if_addr)));
  // Enable loopback so we receive our own packets (for testing)
  int loop = 1;
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)));

  // Set multicast TTL (time to live)
  int ttl = 32;
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)));

  return sock;
}

static void send_multicast_json(int sock, const Bytes &data)
{
  struct sockaddr_in dest_addr;
  BZERO(dest_addr);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(MULTICAST_PORT);
  dest_addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);

  int err = sendto(sock, data.data(), data.length(), 0,
                   (struct sockaddr *)&dest_addr, sizeof(dest_addr));

  if (err < 0)
  {
    ERROR("Error occurred during sending: errno %d", errno);
  }
  else
  {
    ERROR("Message sent: %d", data.length());
  }
}

// Function to receive multicast messages
static void receive_multicast_messages(void *pvParameters)
{
  char rx_buffer[MAX_UDP_PACKET_SIZE];
  struct sockaddr_in source_addr;
  socklen_t socklen = sizeof(source_addr);

  while (1)
  {
    int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                       (struct sockaddr *)&source_addr, &socklen);

    if (len < 0)
    {
      ERROR( "recvfrom failed: errno %d", errno);
      break;
    }
    else
    {
      rx_buffer[len] = '\0'; // Null-terminate the received data
      printf("Received %s\n", rx_buffer);

      // Parse JSON
      cJSON *json = cJSON_Parse(rx_buffer);
      if (json == NULL)
      {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
          ERROR( "JSON parse error before: %s", error_ptr);
        }
      }
      else
      {
        // Print the received JSON
        char *json_str = cJSON_Print(json);
        ERROR("Received JSON from %s:%d: %s",
                 inet_ntoa(source_addr.sin_addr), ntohs(source_addr.sin_port), json_str);
        free(json_str);

        // You can access JSON fields here
        // Example:
        // cJSON *field = cJSON_GetObjectItemCaseSensitive(json, "field_name");
        // if (cJSON_IsString(field) && (field->valuestring != NULL)) {
        //     printf("field_name: %s\n", field->valuestring);
        // }

        cJSON_Delete(json);
      }
    }
  }

  vTaskDelete(NULL);
}
