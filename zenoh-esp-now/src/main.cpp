
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
#include <codec.h>
#include <esp_gtw.h>

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
FrameDecoder frame_decoder;
MsgHeader msg_header;
struct ComposedId
{
    uint32_t topic_id;
    uint8_t prop_id;
};

struct Desc
{
    uint32_t Option<topic_id>;
    uint8_t prop_id;
    uint8_t type;
    std::string name;
    Option<std::string> desc;
};

std::map<struct ComposedId, struct Desc> msg_type_map;

void cbor_to_json(FrameDecoder &decoder, std::map<struct ComposedId, struct Desc> &msg_type_map)
{
    struct ComposedId composed_id;
    struct Desc desc;
}

void on_esp_now_publish(MsgHeader &msg_header, FrameDecoder &decoder)
{
    if (decoder.decode_array().is_ok())
    { //

        cbor_to_json(decoder, msg_type_map);
    }
    else
    {
        ERROR("Error decoding ESP-NOW message");
    }
}

void on_esp_now_desc(MsgHeader &msg_header, FrameDecoder &decoder)
{
    if (decoder.decode_array().is_ok())
    { //
        struct Desc desc;
        desc.topic_id = msg_header.src.unwrap();
        desc.topic_id = decoder.decode_uint32().unwrap();
    }
    else if (decoder.decode_map().is_ok())
    {
    }
    else
    {
        ERROR("Error decoding ESP-NOW message");
    }
}

void on_esp_now_message(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    Serial.print("Received ESP-NOW message from ");
    for (int i = 0; i < 6; i++)
    {
        Serial.print(mac_addr[i], HEX);
        if (i < 5)
        {
            Serial.print(":");
        }
    }
    Serial.print(" with data: ");
    Serial.println((const char *)data);
    if (msg_header.decode(frame_decoder).is_err())
    {
        Serial.println("Error decoding message header");
        return;
    }
    switch (msg_header.msg_type)
    {
    case 0:
        Serial.println("Received message type 0");
        break;
    case 1:
        Serial.println("Received message type 1");
        break;
    default:
    {
        break;
    }
    }
}

void setup()
{
    // Initialize Serial for debug
    Serial.begin(115200);
    while (!Serial)
    {
        delay(1000);
    }

    // Set WiFi in STA mode and trigger attachment
    Serial.print("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(5000);
    }
    esp_gtw.init().on_error([](const char *msg)
                            {
        Serial.print("Failed to initialize ESP-NOW" );
        Serial.println(msg);
        while (1)
        {
            ;
        } });
    frame_decoder = FrameDecoder(256);
    esp_gtw.on_message(on_esp_now_message);

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
        Serial.println("Unable to open session!");
        delay(1000);
    }
    Serial.println("OK");

    // Start read and lease tasks for zenoh-pico
    if (zp_start_read_task(z_session_loan_mut(&s), NULL) < 0 || zp_start_lease_task(z_session_loan_mut(&s), NULL) < 0)
    {
        Serial.println("Unable to start read and lease tasks\n");
        z_close(z_session_move(&s), NULL);
        while (1)
        {
            ;
        }
    }

    // Declare Zenoh publisher
    Serial.print("Declaring publisher for ");
    Serial.print(KEYEXPR);
    Serial.println("...");
    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str_unchecked(&ke, KEYEXPR);
    if (z_declare_publisher(&pub, z_session_loan(&s), z_view_keyexpr_loan(&ke), NULL) < 0)
    {
        Serial.println("Unable to declare publisher for key expression!");
        while (1)
        {
            ;
        }
    }
    Serial.println("OK");
    Serial.println("Zenoh setup finished!");

    delay(300);
}

void loop()
{
    delay(100);
    char buf[256];
    sprintf(buf, "[%4d] %s", idx++, VALUE);

    Serial.print("Writing Data ('");
    Serial.print(KEYEXPR);
    Serial.print("': '");
    Serial.print(buf);
    Serial.println("')");

    // Create payload
    z_owned_bytes_t payload;
    z_bytes_serialize_from_str(&payload, buf);

    if (z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&payload), NULL) < 0)
    {
        Serial.println("Error while publishing data");
    }
    esp_gtw.send((uint8_t *)buf, strlen(buf)).on_error([](const char *msg)
                                                       { Serial.println(msg); });
}
