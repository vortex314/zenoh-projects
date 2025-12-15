#include "mdns_actor.h"
#include <mdns.h>

static const char *MDNS_INSTANCE = "ESP32 mDNS Service";

MdnsActor::MdnsActor(const char *name, const char *hostname)
    : Actor(name)
{
    _hostname = std::string(hostname);
}

MdnsActor::~MdnsActor()
{
}

void MdnsActor::on_start()
{
    INFO("OTA Actor started");
}

void MdnsActor::init()
{
    esp_err_t err;
    INFO("Initializing mDNS with hostname: %s", _hostname.c_str());

    err = mdns_init();
    if (err != ESP_OK)
    {
        ERROR("mdns_init failed: %s", esp_err_to_name(err));
        return;
    }

    err = mdns_hostname_set(_hostname.c_str());
    if (err != ESP_OK)
    {
        ERROR("mdns_hostname_set failed: %s", esp_err_to_name(err));
        return;
    }

    err = mdns_instance_name_set(MDNS_INSTANCE);
    if (err != ESP_OK)
    {
        ERROR("mdns_instance_name_set failed: %s", esp_err_to_name(err));
        return;
    }

    INFO("mDNS started, hostname: %s.local", _hostname.c_str());
}

void MdnsActor::on_message(const Envelope &envelope)
{
    const Msg &msg = *envelope.msg;

    msg.handle<WifiConnected>([&](const auto &msg)
                              { init(); });

    msg.handle<WifiDisconnected>([&](const auto &msg) {});
}
