
//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//

#include <Arduino.h>
#include <WiFi.h>
#include <zenoh-pico.h>
#include <esp_now.h>
#include <malloc.h>
#include <esp_log.h>
#include <map>
#include <esp_gtw.h>
#include <codec.h>

#define SSID XSTR(WIFI_SSID)
#define PASS XSTR(WIFI_PASS)

// Client mode values (comment/uncomment as needed)
#define MODE "client"
#define CONNECT "tcp/192.168.0.197:7447" // If empty, it will scout
// Peer mode values (comment/uncomment as needed)
// #define MODE "peer"
// #define CONNECT "udp/224.0.0.225:7447#iface=en0"

#define KEYEXPR "demo/example/zenoh-pico-pub"
#define VALUE "[ARDUINO]{ESP32} Publication from Zenoh-Pico!"

z_owned_session_t s;
z_owned_publisher_t pub;
static int idx = 0;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
EspGtw esp_gtw;
FrameDecoder frame_decoder(256);
MsgHeader msg_header;
struct ComposedId
{
    uint32_t topic_id;
    uint8_t prop_id;
};

std::map<struct ComposedId, DescMsg> msg_type_map;

void cbor_to_json(FrameDecoder &decoder, std::map<struct ComposedId, DescMsg> &msg_type_map)
{
    struct ComposedId composed_id;
    DescMsg desc;
}

void on_esp_now_publish(MsgHeader &msg_header, FrameDecoder &decoder)
{
}

Result<Void> on_esp_now_desc_msg(MsgHeader &msg_header, FrameDecoder &decoder)
{
    return Result<Void>::Ok(Void());
}

void on_esp_now_message(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    frame_decoder.fill_buffer((uint8_t *)data, len);
    INFO("Received ESP-NOW message %d", len);
    auto msg = MsgHeader::decode(frame_decoder);
    if (msg.is_err())
    {
        ERROR("Error decoding ESP-NOW message");
        return;
    }
    INFO("Received ESP-NOW message");
    MsgHeader msg_header = msg.unwrap();
    switch (msg_header.msg_type)
    {
    case MsgType::Pub:
        on_esp_now_publish(msg_header, frame_decoder);
        break;
    case MsgType::Info:
        on_esp_now_desc_msg(msg_header, frame_decoder);
        break;

    case MsgType::Alive:
        break;
    }
}

Log logger(256);

void setup()
{
    // Initialize Serial for debug
    Serial.begin(115200);
    frame_decoder = FrameDecoder(256);

    while (!Serial)
    {
        delay(1000);
    }

    // Set WiFi in STA mode and trigger attachment
    Serial.print("Connecting to WiFi...");
    INFO("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.channel(1);
    WiFi.begin(SSID, PASS);
    WiFi.channel(1);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(5000);
    }
    esp_gtw.init().on_error([](const char *msg)
                            {
        INFO("Failed to initialize ESP-NOW {} ",msg );
        while (1)
        {
            ;
        } });
    esp_gtw.set_callback_receive(on_esp_now_message);

    // Initialize Zenoh Session and other parameters
    z_owned_config_t config;
    z_config_default(&config);
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, MODE);
    if (strcmp(CONNECT, "") != 0)
    {
        zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, CONNECT);
    }

    // Open Zenoh session
    Serial.print("Opening Zenoh Session...");
    while (z_open(&s, z_config_move(&config), NULL) < 0)
    {
        INFO("Unable to open session!");
        delay(1000);
    }
    INFO("OK");

    // Start read and lease tasks for zenoh-pico
    if (zp_start_read_task(z_session_loan_mut(&s), NULL) < 0 || zp_start_lease_task(z_session_loan_mut(&s), NULL) < 0)
    {
        INFO("Unable to start read and lease tasks\n");
        z_close(z_session_move(&s), NULL);
        while (1)
        {
            ;
        }
    }

    // Declare Zenoh publisher
    Serial.print("Declaring publisher for ");
    Serial.print(KEYEXPR);
    INFO("...");
    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str_unchecked(&ke, KEYEXPR);
    if (z_declare_publisher(&pub, z_session_loan(&s), z_view_keyexpr_loan(&ke), NULL) < 0)
    {
        INFO("Unable to declare publisher for key expression!");
        while (1)
        {
            ;
        }
    }
    INFO("OK");
    INFO("Zenoh setup finished!");

    delay(1000);
}

void loop()
{
    delay(1000);
    char buf[256];
    sprintf(buf, "[%4d] %s", idx++, VALUE);
    INFO("Publishing data. %s = %s ", KEYEXPR, buf);

    // Create payload
    z_owned_bytes_t payload;
    z_bytes_serialize_from_str(&payload, buf);

    if (z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&payload), NULL) < 0)
    {
        ERROR("Error while publishing data");
    }
    esp_gtw.send((uint8_t *)buf, strlen(buf)).on_error([](const char *msg)
                                                       { ERROR(msg); });
}
