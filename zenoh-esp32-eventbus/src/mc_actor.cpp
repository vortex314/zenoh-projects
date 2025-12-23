#include "mc_actor.h"

constexpr int MULTICAST_PORT = 50000;
constexpr const char *MULTICAST_GROUP = "224.0.0.1";
constexpr int UNICAST_PORT = 50001;
constexpr int BUF_SIZE = 1024;
constexpr const char *BROKER_IP = "192.168.0.148";

McActor::McActor(const char *name, const char *hostname)
    : Actor(name)
{
    _hostname = std::string(hostname);
    _timer_publish = timer_repetitive(10000);
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
    _event_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (validate_rc(_event_socket, "event socket"))
        return;
    _req_reply_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (validate_rc(_req_reply_socket, "req_reply socket"))
        return;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000; // 1ms timeout
    setsockopt(_event_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    //  setsockopt(_event_socket, IPPROTO_IP, IP_MULTICAST_IF, &local.sin_addr, sizeof(local.sin_addr));
    fcntl(_event_socket, F_SETFL, O_NONBLOCK);
    memset(&_event_addr, 0, sizeof(_event_addr));
    _event_addr.sin_family = AF_INET;
    _event_addr.sin_port = htons(UNICAST_PORT);
    _event_addr.sin_addr.s_addr = inet_addr(BROKER_IP);
    memset(&_req_reply_addr, 0, sizeof(_req_reply_addr));
    _req_reply_addr.sin_family = AF_INET;
    _req_reply_addr.sin_port = htons(MULTICAST_PORT);
    _req_reply_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(UNICAST_PORT);

    int rc = bind(_req_reply_socket, (sockaddr *)&local, sizeof(local));
    validate_rc(rc, "req_reply bind");
    //   rc = bind(_event_socket, (sockaddr *)&local, sizeof(local));
    //   validate_rc(rc, "event bind");
}

void McActor::stop_event()
{
    if (_req_reply_socket >= 0)
    {
        close(_req_reply_socket);
        _req_reply_socket = -1;
    }
}

Bytes McActor::encode_message(const char *dst, const char *src, const char *type, const Bytes &payload)
{
    UdpMessage msg;
    if (dst)
        msg.dst = std::string(dst);
    if (src)
    {
        char src_full[64];
        snprintf(src_full, sizeof(src_full), "%s/%s", _hostname.c_str(), src);
        msg.src = std::string(src_full);
    }
    msg.msg_type = std::string(type);
    msg.payload = payload;
    return UdpMessage::cbor_serialize(msg).unwrap();
}

void McActor::send_event(const char *dst, const char *src, const char *type, const Bytes &payload)
{

    Bytes buffer = encode_message(dst, src, type, payload);
    size_t used = buffer.size();

    // send bytes
    int sent = sendto(_req_reply_socket, buffer.data(), used, 0, (sockaddr *)&_event_addr, sizeof(_event_addr));
    if (sent < 0)
    {
        ERROR("Failed to send multicast message: %s [%lu]", strerror(errno), used);
    }
    else
    {
        DEBUG("Sent message to %s:%d (%d bytes)", BROKER_IP, UNICAST_PORT, sent);
    }
}

void McActor::stop_listener()
{
    _running = false;
}

void McActor::on_message(const Envelope &envelope)
{
    const Msg &msg = *envelope.msg;
    msg.handle<TimerMsg>([&](const TimerMsg &msg)
                         { send_ping(); });

    msg.handle<Ping>([&](const auto &msg)
                     { INFO("MC Actor received Ping"); });

    msg.handle<WifiConnected>([&](const auto &msg)
                              { _connected = true; init_event();start_listener(); });

    msg.handle<WifiDisconnected>([&](const auto &msg) { /*stop_listener(); */ });

    if (!_connected) // ignore other messages if not connected
        return;
    msg.handle<SysEvent>([&](const auto &msg)
                         { SysEvent::json_serialize(msg).just([&](const auto &s)
                                                              { send_event(NULL, envelope.src->name(), msg.type_name(), s); }); });
    msg.handle<WifiEvent>([&](const auto &msg)
                          { WifiEvent::json_serialize(msg).just([&](const auto &serialized_msg)
                                                                { send_event(NULL, envelope.src->name(), msg.type_name(), serialized_msg); }); });
    msg.handle<HoverboardEvent>([&](const auto &msg)
                                { HoverboardEvent::json_serialize(msg).just([&](const auto &serialized_msg)
                                                                            { send_event(NULL, envelope.src->name(), msg.type_name(), serialized_msg); }); });
}

void McActor::start_listener()
{
    xTaskCreatePinnedToCore(udp_listener_task, "udp_listener_task", 8192, this, 5, NULL, 1);
}

void McActor::udp_listener_task(void *pvParameters)
{
    McActor *actor = static_cast<McActor *>(pvParameters);
    while (true)
    {
        /*int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (validate_rc(sock, "socket"))
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(UNICAST_PORT);
        addr.sin_addr.s_addr = INADDR_ANY;

        int rc = bind(sock, (sockaddr *)&addr, sizeof(addr));
        if (validate_rc(rc, "bind"))
        {
            close(sock);
            continue;
        }*/

        char buf[BUF_SIZE];
        while (actor->_running)
        {
            sockaddr_in sender{};
            socklen_t sender_len = sizeof(sender);
            int len = recvfrom(actor->_req_reply_socket, buf, BUF_SIZE - 1, 0, (sockaddr *)&sender, &sender_len);
            actor->on_request(Bytes(buf, buf + len), sender);
            /*            if (validate_rc(len, "recvfrom"))
                            continue;
                        buf[len] = '\0';
                        std::string cmd(buf);
                        std::cout << "Received command: " << cmd << " from " << inet_ntoa(sender.sin_addr) << ":" << ntohs(sender.sin_port) << std::endl;
                        // Reply to sender
                        std::string reply = "ACK: " + cmd;*/
        }
    }
}

void McActor::on_request(const Bytes &request, const sockaddr_in &sender_addr)
{
    // INFO("MC Actor received request (%lu bytes)", request.size());
    auto r = UdpMessage::cbor_deserialize(request);
    if (r.is_err())
    {
        ERROR("Failed to deserialize UdpMessage");
        return;
    }
    UdpMessage *msg = r.unwrap();
    /*INFO("Received UdpMessage from %s to %s of type %s",
          msg->src.has_value() ? msg->src->c_str() : "unknown",
          msg->dst.has_value() ? msg->dst->c_str() : "unknown",
          msg->msg_type.has_value() ? msg->msg_type->c_str() : "unknown");*/
    // Process UdpMessage
    if (msg->msg_type.has_value() && msg->payload.has_value())
    {
        on_message(msg->msg_type->c_str(), *(msg->payload));
    }

    //    INFO("%s => %s : %s %s", src, dst, type, std::string(payload.begin(), payload.end()).c_str());
    if (msg->msg_type.has_value() && strcmp(msg->msg_type->c_str(), "Pong") == 0)
    {
        send_ping();
    }
    delete msg;
    // Process the request and prepare a reply
    // Send the reply back to the sender
    //   sendto(_req_reply_socket, reply_str.c_str(), reply_str.size(), 0, (sockaddr *)&sender_addr, sizeof(sender_addr));
}

void McActor::on_message(const char *type, Bytes &payload)
{
    /*INFO("MC Actor received message of type %s with payload: %s",
         type,
         std::string(payload.begin(), payload.end()).c_str());*/
    if (strcmp(type, "Ping") == 0)
    {
        INFO("MC Actor received Ping message");
        Ping::json_deserialize(payload).just([&](const Ping *msg)
                                             {  auto pong = new Pong();
                                                pong->number = msg->number;
                                                emit(pong); });
    }
}

void McActor::send_request_reply(const char *dst, const char *src, const char *type, const Bytes &payload)
{
    Bytes buffer = encode_message(dst, src, type, payload);
    int sent = sendto(_req_reply_socket, buffer.data(), buffer.size(), 0, (sockaddr *)&_req_reply_addr, sizeof(_req_reply_addr));
    if (sent < 0)
    {
        ERROR("Failed to send request-reply message: %s [%lu]", strerror(errno), buffer.size());
    }
    else
    {
        INFO("Sent request-reply message to %s:%d (%d bytes)", MULTICAST_GROUP, MULTICAST_PORT, sent);
    }
}

void McActor::send_ping()
{
    static int64_t start_time = esp_timer_get_time();
    Ping ping;
    ping.number = _ping_counter++;
    send_event(NULL, ref().name(), ping.type_name(), Ping::json_serialize(ping).unwrap());
    if (_ping_counter % 1000 == 0)
    {
        int64_t now = esp_timer_get_time();
        INFO("ping %f msg/sec", 1000000000.0 / (now - start_time));
        start_time = now;
    }
}
