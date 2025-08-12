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
#include <wifi_actor.h>
#include "esp_netif.h"

static int create_multicast_socket();

void send()
{
  auto msg = McSend("test/topic", Value("Hello, World!"));
  msg.topic = "test/topic";
  msg.value = Value("Hello, World!");
  if (msg.type_id() == McSend::_id)
  {
    INFO("Message ID matches McSend ID");
  }
  else
  {
    INFO("Message ID does not match McSend ID");
  }
  INFO("Sending message: %s", msg.value.toJson().c_str());
}

McActor::McActor(const char *name)
    : Actor(name)
{
  _timer_publish = timer_repetitive(1000); // timer for publishing properties
}

void McActor::on_start()
{
  auto r = start_receiver_task();
  if (r.is_ok())
  {
    _task_handle = r.unwrap();
  }
  else
  {
    ERROR(" starting multicast receiver failed ");
  }
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

void McActor::on_message(const Msg &message)
{
  message.handle<McSend>([&](const McSend &msg)
                         { INFO("Received multicast message on topic %s: %s", msg.topic.c_str(), msg.value.toJson().c_str()); });

  message.handle<TimerMsg>([&](const TimerMsg &msg)
                           { on_timer(msg.timer_id); });

  message.handle<WifiConnected>([&](auto _)
                                { on_wifi_connected(); });
  message.handle<WifiDisconnected>([&](auto _)
                                { disconnect(); });
}

void McActor::on_wifi_connected()
{
  if (!_connected)
  {
    Result res = connect();
    if (res.is_err())
    {
      INFO("Failed to connect to Mc: %s", res.msg());
      //          vTaskDelay(1000 / portTICK_PERIOD_MS);
      //tell(cmd); // tell myself to get connected
    }
    else
    {
      INFO("Connected to Mc.");
    }
  }
}



Result<TaskHandle_t> McActor::start_receiver_task()
{
  TaskHandle_t task_handle;
  BaseType_t rc = xTaskCreate(receiver_task, "udp_rx", 4096, this, 5, &task_handle);
  if (rc == pdPASS)
    return Result<TaskHandle_t>(task_handle);
  return Result<TaskHandle_t>(rc, "xTaskCreate failed");
}
// FreeRTOS task to receive udp messages
void McActor::receiver_task(void *pv)
{
  McActor *mc_actor = (McActor *)pv;
  struct sockaddr_in source_addr;
  socklen_t socklen = sizeof(source_addr);
  INFO("Starting multicast receiver task...");

  while (true)
  {
    while (mc_actor->_mc_socket < 0)
    {
      INFO("Waiting for multicast socket to be created...");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      mc_actor->_mc_socket = create_multicast_socket();
    }
    while (true)
    {
      int len = recvfrom(mc_actor->_mc_socket, mc_actor->_rx_buffer, sizeof(mc_actor->_rx_buffer) - 1, 0,
                         (struct sockaddr *)&source_addr, &socklen);
      if (len < 0)
      {
        ERROR(" Multicast recv failed. Retrying....");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
      else
      {
        mc_actor->_rx_buffer[len] = '\0';
        INFO("UDP RXD [%d]: %s", len, mc_actor->_rx_buffer);
        auto res_msg = Value::fromJson((const char *)mc_actor->_rx_buffer);
        if (!res_msg.is_ok())
        {
          ERROR("Failed to parse multicast message: %s", res_msg.msg());
          continue;
        }

        esp_ip4_addr_t *ip_addr = (esp_ip4_addr_t *)&source_addr.sin_addr.s_addr;
        res_msg.inspect([&](auto v)
                        {
          v["from_ip"]=ip4addr_to_str(ip_addr);
          std::string topic = v["src"].as<std::string>();
          mc_actor->emit(new PublishMsg(mc_actor->ref(),topic,v)); });
      }
    }
  }
}

Res McActor::connect(void)
{
  _mc_socket = create_multicast_socket();
  if (_mc_socket < 0)
  {
    return Res(errno, "Failed to create multicast socket");
  }
  _connected = true;
  INFO("UDP Listening on %s:%d", MULTICAST_IP, MULTICAST_PORT);
  return ResOk;
}

Res McActor::disconnect()
{
  INFO("Closing Mc Session...");
  close(_mc_socket);
  _connected = false;
  _mc_socket = -1;
  return ResOk;
}

bool McActor::is_connected() const
{
  return _connected;
}

McActor::~McActor()
{
  if (_mc_socket > 0)
    close(_mc_socket);
  INFO("Closing Mc Session...");
}

Res McActor::publish_props()
{
  //  INFO("Publishing properties... connected =%d", _connected);
  if (!_connected)
  {
    return Res(ENOTCONN, "Not connected to Mc");
  }
  Value v, publish;
  get_props(publish);
  v["pub"] = publish;
  emit(v);
  return ResOk;
}

//============================================================

void McActor::get_props(Value &v) const
{
  v["ip"] = MULTICAST_IP;
  v["port"] = MULTICAST_PORT;
  v["packet_size"] = MAX_UDP_PACKET_SIZE;
}

static int create_multicast_socket()
{
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0)
  {
    ERROR("Failed to create socket: errno %d : %s", errno, strerror(errno));
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
    ERROR("Failed to get network interface");
    close(sock);
    return -1;
  }
  if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK)
  {
    ERROR("Failed to get IP info");
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
    ERROR("Failed to join multicast group: errno %d : %s", rc, strerror(errno));
    close(sock);
    return -1;
  }

  // Set multicast interface
  struct in_addr if_addr;
  if_addr.s_addr = htonl(INADDR_ANY);
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &if_addr, sizeof(if_addr)));
  // Disable loopback so we receive our own packets (for testing)
  int loop = 0;
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)));

  // Set multicast TTL (time to live)
  int ttl = 32;
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)));

  return sock;
}

static int create_udp_socket(uint16_t port)
{
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0)
  {
    ERROR("Failed to create socket: errno %d : %s", errno, strerror(errno));
    return -1;
  }

  // Set socket options
  int opt = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // Bind the socket to any address
  struct sockaddr_in saddr;
  BZERO(saddr);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr = (struct in_addr){
      .s_addr = htonl(INADDR_ANY) // Bind to all interfaces
  };

  T_ESP(bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)));

  // Get the actual network interface IP address
  esp_netif_ip_info_t ip_info;
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (netif == NULL)
  {
    ERROR("Failed to get network interface");
    close(sock);
    return -1;
  }
  if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK)
  {
    ERROR("Failed to get IP info");
    close(sock);
    return -1;
  }

  // Set multicast interface
  struct in_addr if_addr;
  if_addr.s_addr = htonl(INADDR_ANY);
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &if_addr, sizeof(if_addr)));
  // Disable loopback so we receive our own packets (for testing)
  int loop = 0;
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)));

  // Set multicast TTL (time to live)
  int ttl = 32;
  T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)));

  return sock;
}

Result<Void> McActor::send(const std::string &data)
{
  struct sockaddr_in dest_addr;
  BZERO(dest_addr);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(MULTICAST_PORT);
  dest_addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);

  int err = sendto(_mc_socket, data.data(), data.size(), 0,
                   (struct sockaddr *)&dest_addr, sizeof(dest_addr));

  if (err < 0)
  {
    return Result<Void>(errno, "Error occurred during sending ");
  }
  return ResOk;
}
/*
// Function to receive multicast messages
Result<Bytes> McActor::receive()
{
  unsigned char rx_buffer[MAX_UDP_PACKET_SIZE];
  struct sockaddr_in source_addr;
  socklen_t socklen = sizeof(source_addr);

  int len = recvfrom(_socket, rx_buffer, sizeof(rx_buffer) - 1, 0,
                     (struct sockaddr *)&source_addr, &socklen);

  if (len < 0)
  {
    return Result<Bytes>(errno, "recvfrom failed");
  }
  else
  {
    rx_buffer[len] = '\0';

    Bytes bytes = Bytes(rx_buffer[0], rx_buffer[len]);
    return Result<Bytes>(bytes);
  }
}*/
