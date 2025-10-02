#include <mc_actor.h>
#include <string>

static int create_multicast_socket();

/*void send()
{
  auto msg = McSend("test/topic", Value("Hello, World!"));
  msg.topic = "test/topic";
  msg.value = Value("Hello, World!");
  msg.handle<McSend>([&](const McSend &m)
                     { INFO("Sending message: %s", msg.value.toJson().c_str()); });
}*/

McActor::McActor(const char *name)
    : Actor(name), _json_bytes(1024), _json_serializer(_json_bytes)
{
  _timer_publish = timer_repetitive(5000); // timer for publishing properties
}

void McActor::on_start()
{
  auto r = start_receiver_task();
  if (r.is_ok())
  {
    _task_handle = r.unwrap();
    for (auto actor : eventbus()->actors())
    {
      _local_objects.emplace(actor->name(), LocalObject{actor->ref()});
    }
  }
  else
  {
    ERROR(" starting multicast receiver failed ");
  }
}

void McActor::on_timer(int id)
{
  for (auto &[src, dsts] : _subscriptions)
  {
    for (auto &[dst, timeout] : dsts)
    {
      INFO("Subscription : %s -> %s : %llu", src.c_str(), dst.c_str(), timeout);
    }
  }
  if (id == _timer_publish)
  {
    publish_props(); // publish own props
    /*for (auto &[name, local_obj] : _local_objects) // iterate over local objects
    {
      Value msg;
      msg["src"] = name;
      msg["subs"].array();
      for (auto &[src, dsts] : _subscriptions)
      {
        for (auto &[dst, timeout] : dsts)
        {
          if (dst == name)
          {
            msg["subs"].add(src);
          }
        }
      }
      send(msg.toJson());
    }*/
  }
  /* else if (id == _timer_broadcast)
   {
     for (const auto &[key, sub] : _subscriptions)
     {
       Value v;
       std::vector<Value> values;
       v["src"] = key;

       for (std::string src : sub.srcs)
       {
         values.push_back(Value(src));
         INFO("Broadcasting subscription: %s", v.toJson().c_str());
       }
       v["sub"] = values;
     }
   }*/
  else
    INFO("Unknown timer id: %d", id);
}

void McActor::on_message(const Envelope &env)
{
  const Msg &message = *env.msg;
  message.handle<McSend>([&](const McSend &msg)
                         { INFO("Received multicast message on topic %s: %s", msg.topic.c_str(), msg.value.toJson().c_str()); });

  message.handle<TimerMsg>([&](const TimerMsg &msg)
                           { on_timer(msg.timer_id); });
  message.handle<WifiConnected>([&](auto _)
                                { on_wifi_connected(); });
  message.handle<WifiDisconnected>([&](auto _)
                                   { disconnect(); });
  message.handle<Subscribe>([&](const Subscribe &sub)
                            {
                              INFO("Subscribe %s => %s", sub.src.c_str(), sub.dst.c_str());
                              add_subscription(sub.src, sub.dst, current_time()+sub.timeout); });
  message.handle<PublishTxd>([&](const PublishTxd &msg) { // INFO("Received publish message on topic %s: %s", msg.topic.c_str(), msg.value.toJson().c_str());
    std::string s;
    serializeJson(msg.doc, s);
    send("", Bytes(s.begin(),s.end()));
  });
  message.handle<SysInfo>([&](const SysInfo &sys_info)
                          {
                            std::string topic = "src/" + std::string(this->name()) + "/SysInfo/JSON";
                            send(topic,sys_info.serialize()); });
  message.handle<WifiInfo>([&](const WifiInfo &wifi_info)
                           {
                              std::string topic = "src/" + std::string(this->name()) + "/WifiInfo/JSON";
                            send(topic,wifi_info.serialize()); });
  message.handle<MulticastInfo>([&](const MulticastInfo &multicast_info)
                                {
                                  std::string topic = "src/" + std::string(this->name()) + "/MulticastInfo/JSON";
                            send(topic,multicast_info.serialize()); });
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
      // tell(cmd); // tell myself to get connected
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
      INFO("Waiting for multicast message...");
      struct sockaddr_in source_addr;
      socklen_t socklen = sizeof(source_addr);

      int len = recvfrom(mc_actor->_mc_socket, mc_actor->_rx_buffer, sizeof(mc_actor->_rx_buffer) - 1, 0,
                         (struct sockaddr *)&source_addr, &socklen);
      if (len < 0)
      {
        ERROR(" Multicast recv failed. Retrying....");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
      else
      {
        Bytes bytes(mc_actor->_rx_buffer, mc_actor->_rx_buffer + len);

        //    esp_ip4_addr_t *ip_addr = (esp_ip4_addr_t *)&source_addr.sin_addr.s_addr;
        mc_actor->on_multicast_message(source_addr, bytes);
      }
    }
  }
}

void McActor::on_multicast_message(const sockaddr_in &source_addr, Bytes &data)
{

  data.push_back('\0');
  INFO("UDP RXD [%d]: %s", data.size(), data.data());
  JsonDocument doc;
  auto erc = deserializeJson(doc, (const char *)data.data());
  if (erc != DeserializationError::Ok)
  {
    ERROR("Failed to parse multicast message: %s", erc.c_str());
    return;
  }
  auto msg = doc.as<JsonObject>();

  if (msg["type"].as<std::string>() == "pub")
  {
    // Handle publish message
    if (msg["SysCmd"].is<JsonObject>())
    {
      auto m = msg["SysCmd"].as<JsonObject>();
      SysCmd *sys_cmd = new SysCmd();
      if (m["set_time"])
      {
        sys_cmd->set_time = msg["set_time"].as<uint64_t>();
      }
      if (m["reboot"])
      {
        sys_cmd->reboot = msg["reboot"].as<bool>();
      }
      emit(sys_cmd);
      // Process sys_cmd
    }
  }
  else if (msg["type"].as<std::string>() == "sub")
  {
    // Handle subscribe message
  }
  /* v.get("sub").handle<Value::ObjectType>([&](const Value::ArrayType &sub)
                                          {
                                            auto dst = v.get("src").as<std::string>();
                                            for (const auto &item : sub)
                                            {
                                             auto src = item.as<Value::ArrayType>();
                                             if (eventbus() && eventbus()->find_actor(src.c_str()) != NULL_ACTOR)
                                             {
                                             _subscriptions.emplace(src, Subscription{src, dst, current_time() + 60000}); // 1 minute subscription
                                             _remote_objects.emplace(src, RemoteObject{source_addr});
                                            } } });
   v.get("pub").handle<Value::ObjectType>([&](const Value &pub)
                                          {
                                            std::string topic = v["src"].template as<std::string>();
                                            v.hasKey("sub");
                                            emit(new PublishRxd(ref(), topic, v)); });*/
  /*msg.get("dst").handle<std::string>([&](const std::string &dst)
                                     { ActorRef dst_actor = eventbus()->find_actor(dst.c_str());
                                    if (dst_actor != NULL_ACTOR)
                                    {
                                        emit(new PublishRxd(ref(), dst, msg));
                                    }
                                    else
                                    {
                                        ERROR("No actor found for destination: %s", dst.c_str());
                                    } });
  msg.get("subs").handle<Value::ArrayType>([&](const Value::ArrayType &subs)
                                           {
                                            auto dst = msg["src"].template as<std::string>();
                                      for (const auto &sub : subs)
                                      {
                                        ActorRef dst_actor = eventbus()->find_actor(dst.c_str());
                                        if (dst_actor != NULL_ACTOR)
                                        {
                                          add_subscription(dst, sub.as<std::string>(), 60000);
                                        }
                                      } });*/
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
  MulticastInfo *info = new MulticastInfo();
  info->group = MULTICAST_IP;
  info->port = MULTICAST_PORT;
  info->mtu = MAX_UDP_PACKET_SIZE;
  emit(info);
  return ResOk;
}

//============================================================

Result<JsonDocument> McActor::get_props() const
{
  JsonDocument doc;
  doc["src"] = ref().name();
  doc["type"] = "pub";
  doc["object"] = "mc";
  doc["ip"] = MULTICAST_IP;
  doc["port"] = MULTICAST_PORT;
  doc["packet_size"] = MAX_UDP_PACKET_SIZE;
  return Result<JsonDocument>(doc);
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

Result<Void> McActor::send(const std::string &topic, const Bytes &data)
{
  struct sockaddr_in dest_addr;
  BZERO(dest_addr);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(MULTICAST_PORT);
  dest_addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);
  INFO("Multicast send : %d ", data.size());

  int err = sendto(_mc_socket, data.data(), data.size(), 0,
                   (struct sockaddr *)&dest_addr, sizeof(dest_addr));

  if (err < 0)
  {
    return Result<Void>(errno, "Error occurred during sending ");
  }
  emit(new LedPulse(10));
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

void McActor::add_subscription(const std::string &src, const std::string &dst, uint32_t timeout)
{
  if (_subscriptions.find(src) == _subscriptions.end())
  {
    _subscriptions.emplace(src, std::unordered_map<std::string, uint64_t>());
  }
  _subscriptions[src][dst] = timeout;
}
