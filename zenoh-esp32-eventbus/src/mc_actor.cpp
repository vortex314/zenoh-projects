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
    _timer_publish = timer_repetitive(5);
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
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, arrayEncoder;
    CborError err;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);
    err = cbor_encoder_create_array(&encoder, &arrayEncoder, CborIndefiniteLength);
    if (dst)
        err = cbor_encode_text_stringz(&arrayEncoder, dst);
    else
        err = cbor_encode_null(&arrayEncoder);
    char src_full[64];
    if (src)
    {
        snprintf(src_full, sizeof(src_full), "%s/%s", _hostname.c_str(), src);
        err = cbor_encode_text_stringz(&arrayEncoder, src_full);
    }
    else
    {
        err = cbor_encode_null(&arrayEncoder);
    }
    err = cbor_encode_text_stringz(&arrayEncoder, type);
    err = cbor_encode_byte_string(&arrayEncoder, payload.data(), payload.size());
    err = cbor_encoder_close_container(&encoder, &arrayEncoder);
    (void)err;
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.data(), buffer.data() + used);
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
                         { on_timer(); });

    msg.handle<Ping>([&](const auto &msg)
                     { INFO("MC Actor received Ping"); });

    msg.handle<WifiConnected>([&](const auto &msg)
                              { init_event();start_listener(); });

    msg.handle<WifiDisconnected>([&](const auto &msg) { /*stop_listener(); */ });

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
            if (validate_rc(len, "recvfrom"))
                continue;
            buf[len] = '\0';
            std::string cmd(buf);
            std::cout << "Received command: " << cmd << " from " << inet_ntoa(sender.sin_addr) << ":" << ntohs(sender.sin_port) << std::endl;
            // Reply to sender
            std::string reply = "ACK: " + cmd;
            sendto(actor->_req_reply_socket, reply.c_str(), reply.size(), 0, (sockaddr *)&sender, sender_len);
        }
    }
}

void McActor::on_request(const Bytes &request, const sockaddr_in &sender_addr)
{
    CborParser parser;
    CborValue decoder, arrayDecoder;
    cbor_parser_init(request.data(), request.size(), 0, &parser, &decoder);
    cbor_value_enter_container(&decoder, &arrayDecoder);
    char dst[64];
    char src[64];
    char type[64];
    Bytes payload;
    // Decode dst
    if (cbor_value_is_text_string(&arrayDecoder))
    {
        size_t dst_len;
        cbor_value_calculate_string_length(&arrayDecoder, &dst_len);
        cbor_value_copy_text_string(&arrayDecoder, dst, &dst_len, &arrayDecoder);
        dst[dst_len] = '\0';
    }
    else
    {
        cbor_value_advance(&arrayDecoder);
        dst[0] = '\0';
    }
    // Decode src
    if (cbor_value_is_text_string(&arrayDecoder))
    {
        size_t src_len;
        cbor_value_calculate_string_length(&arrayDecoder, &src_len);
        cbor_value_copy_text_string(&arrayDecoder, src, &src_len, &arrayDecoder);
        src[src_len] = '\0';
    }
    else
    {
        cbor_value_advance(&arrayDecoder);
        src[0] = '\0';
    }
    // Decode type
    if (cbor_value_is_text_string(&arrayDecoder))
    {
        size_t type_len;
        cbor_value_calculate_string_length(&arrayDecoder, &type_len);
        cbor_value_copy_text_string(&arrayDecoder, type, &type_len, &arrayDecoder);
        type[type_len] = '\0';
    }
    else
    {
        cbor_value_advance(&arrayDecoder);
        type[0] = '\0';
    }
    // Decode payload
    if (cbor_value_is_byte_string(&arrayDecoder))
    {
        size_t payload_len;
        cbor_value_calculate_string_length(&arrayDecoder, &payload_len);
        payload.resize(payload_len);
        cbor_value_copy_byte_string(&arrayDecoder, payload.data(), &payload_len, &arrayDecoder);
    }
    else
    {
        cbor_value_advance(&arrayDecoder);
    }
    cbor_value_leave_container(&decoder, &arrayDecoder);
    INFO("%s => %s : %s %s", src, dst, type, std::string(payload.begin(), payload.end()).c_str());
    // Process the request and prepare a reply
    // Send the reply back to the sender
    //   sendto(_req_reply_socket, reply_str.c_str(), reply_str.size(), 0, (sockaddr *)&sender_addr, sizeof(sender_addr));
}

void McActor::on_message(const char *type, Bytes &payload)
{
    INFO("MC Actor received message of type %s with payload: %s",
         type,
         std::string(payload.begin(), payload.end()).c_str());
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

void McActor::on_timer()
{
    Ping ping;
    ping.number = _ping_counter++;
    send_event(NULL, ref().name(), ping.type_name(), Ping::json_serialize(ping).unwrap());
}
