#include "mc_actor.h"

constexpr int MULTICAST_PORT = 50000;
constexpr const char *MULTICAST_GROUP = "239.0.0.1";
constexpr int UNICAST_PORT = 50001;
constexpr int BUF_SIZE = 1024;
constexpr const char *BROKER_IP = "192.168.0.148";

std::string socketAddrToString(struct sockaddr_in *addr)
{
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ip_str, INET_ADDRSTRLEN);

    // ntohs converts "Network To Host Short" (fixes byte order)
    int port = ntohs(addr->sin_port);

    return std::string(ip_str) + ":" + std::to_string(port);
}

McActor::McActor(const char *name, const char *hostname)
    : Actor(name)
{
    _hostname = std::string(hostname);
    _timer_publish = timer_repetitive(1000);
}

McActor::~McActor()
{
}

void McActor::on_start()
{
    INFO("OTA Actor started");
}

int validate_rc(int rc, const char *msg)
{
    if (rc < 0)
    {
        ERROR("'%s' failed: [%d] %s", msg, rc, strerror(errno));
        return rc;
    }
    return 0;
}

void McActor::init_event()
{
    // create multicast socket
    _unicast_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (validate_rc(_unicast_socket, "unicast socket"))
        return;
    _multicast_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (validate_rc(_multicast_socket, "multicast socket"))
        return;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000; // 1ms timeout
    setsockopt(_unicast_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    //  fcntl(_unicast_socket, F_SETFL, O_NONBLOCK);

    memset(&_unicast_src_addr, 0, sizeof(_unicast_src_addr));
    _unicast_src_addr.sin_family = AF_INET;
    _unicast_src_addr.sin_port = htons(UNICAST_PORT);
    _unicast_src_addr.sin_addr.s_addr = inet_addr(BROKER_IP);

    memset(&_multicast_addr, 0, sizeof(_multicast_addr));
    _multicast_addr.sin_family = AF_INET;
    _multicast_addr.sin_port = htons(MULTICAST_PORT);
    _multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    setsockopt(_multicast_socket, IPPROTO_IP, IP_MULTICAST_IF, &_multicast_addr.sin_addr, sizeof(_multicast_addr.sin_addr));

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0; // any port

    int rc = bind(_unicast_socket, (sockaddr *)&local, sizeof(local));
    validate_rc(rc, "unicast bind");
    //   rc = bind(_event_socket, (sockaddr *)&local, sizeof(local));
    //   validate_rc(rc, "event bind");
    // Bind multicast socket to receive on multicast port and join group
    sockaddr_in mcast_bind{};
    mcast_bind.sin_family = AF_INET;
    mcast_bind.sin_addr.s_addr = INADDR_ANY;
    mcast_bind.sin_port = htons(MULTICAST_PORT);

    int reuse = 1;
    setsockopt(_multicast_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    rc = bind(_multicast_socket, (sockaddr *)&mcast_bind, sizeof(mcast_bind));
    validate_rc(rc, "multicast bind");

    struct ip_mreq imreq;
    imreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    imreq.imr_interface.s_addr = htonl(INADDR_ANY); // default interface
    rc = setsockopt(_multicast_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imreq, sizeof(imreq));
    validate_rc(rc, "join multicast group");

    unsigned char loop = 0;
    setsockopt(_multicast_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    sockaddr_in broker;
    broker.sin_family = AF_INET;
    broker.sin_port = htons(UNICAST_PORT);
    broker.sin_addr.s_addr = inet_addr(BROKER_IP);
    _broker_addr = broker;
    _broker_name = std::string("pclenovo/broker");
}

void McActor::stop_event()
{
    if (_multicast_socket >= 0)
    {
        close(_multicast_socket);
        _multicast_socket = -1;
    }
}

Result<Bytes> McActor::encode_message(const char *dst, const char *src, const char *type, const Bytes &payload)
{
    UdpMessage msg;
    if (dst)
        msg.dst = std::string(dst);
    if (src)
        msg.src = std::string(src);
    msg.msg_type = std::string(type);
    msg.payload = payload;
    return UdpMessage::cbor_serialize(msg);
}

void McActor::send_unicast(const char *dst, const char *src, const char *type, const Bytes &payload)
{
    dst = dst ? dst : (_broker_name.has_value() ? _broker_name->c_str() : NULL);
    DEBUG("send_unicast %s <= %s : %s = %s ", dst ? dst : "broker", src ? src : "unknown", type, std::string(payload.begin(), payload.end()).c_str());
    auto dst_addr = (dst && _source_map.find(dst) != _source_map.end()) ? _source_map[dst].first : (_broker_addr.has_value() ? *_broker_addr : sockaddr_in{});
    if (dst && dst_addr.sin_family == 0)
    {
        ERROR("Unknown destination address for %s", dst);
        return;
    }

    auto r = encode_message(dst, src, type, payload);
    if (r.is_err())
    {
        ERROR("Failed to encode unicast message: [%d] %s", r.unwrap_err().rc, r.unwrap_err().msg);
        return;
    }
    r.just([&](const Bytes &buffer)
           {  
    DEBUG("Send message_type '%s' to %s", type, dst ? dst : "-");

 int sent = sendto(_unicast_socket, buffer.data(), buffer.size(), 0, (sockaddr *)&dst_addr, sizeof(dst_addr));
    if (sent < 0)
    {
        ERROR("Failed to send unicast message: %s [%lu]", strerror(errno), buffer.size());
    }
    else
    {
        DEBUG("Sent message to %s:%d (%d bytes)", BROKER_IP, UNICAST_PORT, sent);
    } });

    // send bytes
}

void McActor::stop_listener()
{
    _running = false;
}

void McActor::on_message(const Envelope &envelope)
{
    const Msg &msg = *envelope.msg;
    msg.handle<TimerMsg>([&](const TimerMsg &msg)
                         { Alive alive;
                            alive.subscribe.push_back(HoverboardCmd::name_value);
                            alive.publish.push_back(SysEvent::name_value);
                            alive.publish.push_back(WifiEvent::name_value);
                            alive.publish.push_back(HoverboardEvent::name_value);
                            alive.services.push_back(HoverboardCmd::name_value);
                            auto bytes = Alive::json_serialize(alive).unwrap();
                            send_multicast(NULL,DEVICE_NAME,Alive::name_value,bytes);
                            send_ping_req(NULL,0); });

    msg.handle<PingReq>([&](const auto &m) { /*INFO("Received PingReq from '%s' number %d",
                                               envelope.src.has_value() ? envelope.src->name() : "-",
                                               m.number.has_value() ? *(m.number) : -1 );*/
                                             if (m.number.has_value())
                                             {
                                                 send_ping_rep(envelope.src->name(), *(m.number));
                                             }
    });

    msg.handle<WifiConnected>([&](const auto &msg)
                              { _connected = true; 
                                init_event();
                                start_unicast_listener();
                                start_multicast_listener(); });

    msg.handle<WifiDisconnected>([&](const auto &msg) { /*stop_listener(); */ });

    if (!_connected) // ignore other messages if not connected
        return;
    msg.handle<SysEvent>([&](const auto &msg)
                         { SysEvent::json_serialize(msg).just([&](const auto &s)
                                                              { send_unicast(NULL, DEVICE_NAME, msg.type_name(), s); }); });
    msg.handle<WifiEvent>([&](const auto &msg)
                          { WifiEvent::json_serialize(msg).just([&](const auto &serialized_msg)
                                                                { send_unicast(NULL, DEVICE_NAME, msg.type_name(), serialized_msg); }); });
    msg.handle<HoverboardEvent>([&](const auto &msg)
                                { HoverboardEvent::json_serialize(msg).just([&](const auto &serialized_msg)
                                                                            { 
                                                                                send_unicast(NULL, DEVICE_NAME, msg.type_name(), serialized_msg); }); });
}

void McActor::start_unicast_listener()
{
    xTaskCreatePinnedToCore(unicast_listener_task, "udp_listener_task", 8192, this, 5, NULL, 1);
}

void McActor::unicast_listener_task(void *pvParameters)
{
    McActor *actor = static_cast<McActor *>(pvParameters);
    while (true)
    {
        char buf[BUF_SIZE];
        while (actor->_running)
        {
            sockaddr_in sender{};
            socklen_t sender_len = sizeof(sender);
            int len = recvfrom(actor->_unicast_socket, buf, BUF_SIZE - 1, 0, (sockaddr *)&sender, &sender_len);
            if (len < 0)
            {
                ERROR("recvfrom failed: %s", strerror(errno));

                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    continue;
                }
                ERROR("recvfrom failed: %s", strerror(errno));
                continue;
            }
            actor->on_udp_raw(Bytes(buf, buf + len), sender);
        }
    }
}

void McActor::start_multicast_listener()
{
    xTaskCreatePinnedToCore(multicast_listener_task, "udp_listener_task", 8192, this, 5, NULL, 1);
}

void McActor::multicast_listener_task(void *pvParameters)
{
    McActor *actor = static_cast<McActor *>(pvParameters);
    while (true)
    {
        char buf[BUF_SIZE];
        while (actor->_running)
        {
            sockaddr_in sender{};
            socklen_t sender_len = sizeof(sender);
            int len = recvfrom(actor->_multicast_socket, buf, BUF_SIZE - 1, 0, (sockaddr *)&sender, &sender_len);
            if (len < 0)
            {
                ERROR("recvfrom failed: %s", strerror(errno));

                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    continue;
                }
                ERROR("recvfrom failed: %s", strerror(errno));
                continue;
            }
            DEBUG("MC Actor received multicast message (%d bytes)", len);
            actor->on_udp_raw(Bytes(buf, buf + len), sender);
        }
    }
}

void McActor::on_udp_raw(const Bytes &request, const sockaddr_in &sender_addr)
{
    auto r = UdpMessage::cbor_deserialize(request);
    if (r.is_err())
    {
        ERROR("Failed to deserialize UdpMessage");
        return;
    }
    UdpMessage *udp_msg = r.unwrap();
    on_udp_message(*udp_msg, sender_addr);
    delete udp_msg;
}

void McActor::on_udp_message(UdpMessage &udp_msg, const sockaddr_in &sender_addr)
{
    if (!udp_msg.msg_type.has_value() || !udp_msg.payload.has_value())
    {
        ERROR("UdpMessage missing msg_type");
        return;
    }

    Bytes &payload = *(udp_msg.payload);
    const char *msg_type = udp_msg.msg_type->c_str();

    if (strcmp(msg_type, PingReq::name_value) == 0)
    {
        PingReq::json_deserialize(payload).just([&](const PingReq *msg)
                                                { 
                                                    /*INFO(" src '%s' dst '%s' number %d", 
                                                        udp_msg.src.has_value() ? udp_msg.src->c_str() : "-",
                                                        udp_msg.dst.has_value() ? udp_msg.dst->c_str() : "-",
                                                        msg->number.has_value() ? *(msg->number) : -1   );*/
                                                    PingReq* req = new PingReq;
                                                    req->number = msg->number;
                                                    Envelope* env = new Envelope(
                                                    udp_msg.src.has_value() ? ActorRef(udp_msg.src->c_str()) : NULL_ACTOR,
                                                    udp_msg.dst.has_value() ? ActorRef(udp_msg.dst->c_str()) : NULL_ACTOR,
                                                    req
                                                );
                                                emit(env); 
                                            delete msg; });
    }
    else if (strcmp(msg_type, HoverboardCmd::name_value) == 0)
    {
        HoverboardCmd::json_deserialize(payload).just([&](const HoverboardCmd *msg)
                                                      {
            DEBUG(" MC Actor received HoverboardCmd");
            HoverboardCmd* cmd = new HoverboardCmd;
            cmd->speed = msg->speed;
            cmd->steer = msg->steer;
            Envelope* env = new Envelope(
                udp_msg.src.has_value() ? ActorRef(udp_msg.src->c_str()) : NULL_ACTOR,
                udp_msg.dst.has_value() ? ActorRef(udp_msg.dst->c_str()) : NULL_ACTOR,
                cmd
            );
            emit(env);
            delete msg; });
    }

    else if (strcmp(msg_type, PingRep::name_value) == 0)
    {
        send_ping_req(udp_msg.src->c_str(), ++_last_ping_number);
    }
    else if (strcmp(msg_type, Alive::name_value) == 0)
    {
        DEBUG("MC Actor received Alive message");
        Alive::json_deserialize(payload).just([&](const Alive *alive)
                                              {
                if (!udp_msg.src.has_value()) {
                    ERROR("Alive message missing src");
                    return;
                }
                std::string etp = *(udp_msg.src);

//                INFO("  Endpoint: %s @ %s ", etp.c_str(), socketAddrToString((sockaddr_in*)&sender_addr).c_str());
                _source_map[etp] = std::pair<sockaddr_in,uint64_t>(sender_addr,esp_timer_get_time());
                if ( etp == "broker" )
                {
                    INFO("  Setting broker address to %s", socketAddrToString((sockaddr_in*)&sender_addr).c_str());
                    _broker_addr = sender_addr;
                    _broker_name = etp;
                } ;
                            delete alive; });
    }
    else
    {
        INFO("MC Actor received unknown message type: %s", udp_msg.msg_type->c_str());
    }
}

void McActor::send_multicast(const char *dst, const char *src, const char *type, const Bytes &payload)
{
    //   INFO("send_multicast %s <= %s : %s = %s ", dst ? dst : "-", src ? src : "-", type, std::string(payload.begin(), payload.end()).c_str());
    encode_message(dst, src, type, payload).just([&](const Bytes &buffer)
                                                 {  
    int sent = sendto(_unicast_socket, buffer.data(), buffer.size(), 0, (sockaddr *)&_multicast_addr, sizeof(_multicast_addr));
    if (sent < 0)
    {
        ERROR("Multicast failed: %s [%lu]", strerror(errno), buffer.size());
    } });
}

void McActor::send_ping_req(const char *dst, uint32_t number)
{
    static int64_t start_time = esp_timer_get_time();
    static uint32_t counter = 0;
    PingReq ping;
    ping.number = number;
    //    INFO("MC Actor sending Ping %d to %s", number, dst ? dst : "broker");
    send_unicast(dst, DEVICE_NAME, ping.type_name(), PingReq::json_serialize(ping).unwrap());
    counter++;
    if (counter % 1000 == 0)
    {
        int64_t now = esp_timer_get_time();
        INFO("ping %f msg/sec", (counter * 10000000.0) / (now - start_time));
        start_time = now;
        counter = 0;
    }
}

void McActor::send_ping_rep(const char *dst, uint32_t number)
{
    PingRep pong;
    pong.number = number;
    auto b = PingRep::json_serialize(pong);
    //  INFO("Msg Ping resp %d to %s", b.is_ok() ? b.unwrap().size() : -1, dst ? dst : "broker");
    send_unicast(dst, DEVICE_NAME, pong.type_name(), PingRep::json_serialize(pong).unwrap());
}
