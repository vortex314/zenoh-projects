#include "mc_actor.h"

constexpr int MULTICAST_PORT = 50001;
constexpr const char *MULTICAST_GROUP = "224.0.0.1";
constexpr int LISTEN_PORT = 50002;
constexpr int BUF_SIZE = 1024;

McActor::McActor(const char *name, const char *hostname)
    : Actor(name)
{
    _hostname = std::string(hostname);
}

McActor::~McActor()
{
}

void McActor::on_start()
{
    INFO("OTA Actor started");
    init();
    start_listener();
}

void McActor::init_event()
{
    // create multicast socket
    _event_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_event_socket < 0)
    {
        perror("socket");
        return;
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000; // 1ms timeout
    setsockopt(_event_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  //  setsockopt(_event_socket, IPPROTO_IP, IP_MULTICAST_IF, &local.sin_addr, sizeof(local.sin_addr));
    fcntl(_event_socket, F_SETFL, O_NONBLOCK);
    memset(&_event_addr, 0, sizeof(_event_addr));
    _event_addr.sin_family = AF_INET;
    _event_addr.sin_port = htons(MULTICAST_PORT);
    _event_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
}

void McActor::stop_event()
{
    if (_event_socket >= 0)
    {
        close(_event_socket);
        _event_socket = -1;
    }
}

void McActor::send_msg(const char *dst, const char *src, const char *type, const Bytes &payload)
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
        snprintf(src_full, sizeof(src_full), "%s/%s/%s", _hostname.c_str(), src, type);
        err = cbor_encode_text_stringz(&arrayEncoder, src_full);
    }
    else
    {
        err = cbor_encode_null(&arrayEncoder);
    }
    err = cbor_encode_byte_string(&arrayEncoder, payload.data(), payload.size());
    err = cbor_encoder_close_container(&encoder, &arrayEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());

    // send bytes
    int sent = sendto(_event_socket, buffer.data(), used, 0, (sockaddr *)&_event_addr, sizeof(_event_addr));
    if (sent < 0)
    {
        ERROR("Failed to send multicast message: %s [%lu]", strerror(errno), used);
    }
    else
    {
        INFO("Sent multicast message to %s:%d (%d bytes)", MULTICAST_GROUP, MULTICAST_PORT, sent);
    }
}

void McActor::on_message(const Envelope &envelope)
{
    const Msg &msg = *envelope.msg;

    msg.handle<WifiConnected>([&](const auto &msg)
                              { init_event();start_listener(); });

    msg.handle<WifiDisconnected>([&](const auto &msg) {
        stop_listener();
    });
    msg.handle<TimerMsg>([&](const TimerMsg &msg)
                         { INFO("MC Actor Timer Msg id: %d", msg.timer_id); });
    msg.handle<SysEvent>([&](const auto &msg)
                         { SysEvent::json_serialize(msg).just([&](const auto &s)
                                                              { send_msg(NULL, envelope.src->name(), msg.type_name(), s); }); });
    msg.handle<WifiEvent>([&](const auto &msg)
                          { WifiEvent::json_serialize(msg).just([&](const auto &serialized_msg)
                                                                { send_msg(NULL, envelope.src->name(), msg.type_name(), serialized_msg); }); });
    msg.handle<HoverboardEvent>([&](const auto &msg)
                                { HoverboardEvent::json_serialize(msg).just([&](const auto &serialized_msg)
                                                                            { send_msg(NULL, envelope.src->name(), msg.type_name(), serialized_msg); }); });
}

void McActor::start_listener()
{
    xTaskCreatePinnedToCore(udp_listener_task, "udp_listener_task", 8192, this, 5, NULL, 1);
}

void McActor::udp_command_listener(void *pvParameters) {
    McActor *actor = static_cast<McActor *>(pvParameters);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LISTEN_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return;
    }

    char buf[BUF_SIZE];
    while (running) {
        sockaddr_in sender{};
        socklen_t sender_len = sizeof(sender);
        int len = recvfrom(sock, buf, BUF_SIZE - 1, 0, (sockaddr*)&sender, &sender_len);
        if (len < 0) {
            perror("recvfrom");
            continue;
        }
        buf[len] = '\0';
        std::string cmd(buf);
        std::cout << "Received command: " << cmd << " from " << inet_ntoa(sender.sin_addr) << ":" << ntohs(sender.sin_port) << std::endl;
        // Reply to sender
        std::string reply = "ACK: " + cmd;
        sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr*)&sender, sender_len);
    }
    close(sock);
}
