#include "ota_actor.h"

OtaActor::OtaActor(const char *name)
    : Actor(name)
{
    _ota_task_handle = nullptr;
}

OtaActor::~OtaActor()
{
}

void OtaActor::on_start()
{
    INFO("OTA Actor started");
}

void OtaActor::on_message(const Envelope &envelope)
{
    const Msg &msg = *envelope.msg;
    msg.handle<WifiConnected>([&](const auto &msg){ start_task();});
    msg.handle<WifiDisconnected>([&](const auto &msg) { stop_task(); });
}

void OtaActor::start_task()
{
    INFO("Starting OTA TFTP server task...");
    if ( _ota_task_handle == nullptr )
    {
        BaseType_t rc = xTaskCreate(
            &OtaActor::tftp_ota_server_task, 
            "tftp_ota_server_task", 
            8192, 
            NULL, 
            5, 
            &_ota_task_handle); 
        if ( rc != pdPASS )
        {
            ERROR("Failed to create OTA TFTP server task: %d", rc);
        } 
    }
}

void OtaActor::stop_task()
{
    INFO("Stopping OTA TFTP server task...");
    if ( _ota_task_handle != nullptr )
    {
        vTaskDelete( _ota_task_handle );
        _ota_task_handle = nullptr;
    }
}



static void tftp_send_ack(int sock,
                          struct sockaddr_in *client,
                          uint16_t block)
{
    uint8_t pkt[4];
    pkt[0] = 0;
    pkt[1] = TFTP_OPCODE_ACK;
    pkt[2] = block >> 8;
    pkt[3] = block & 0xff;

    sendto(sock, pkt, sizeof(pkt), 0,
           (struct sockaddr *)client,
           sizeof(*client));
}

static void tftp_send_error(int sock,
                            struct sockaddr_in *client,
                            uint16_t code,
                            const char *msg)
{
    uint8_t pkt[128];
    size_t len = 4 + strlen(msg) + 1;

    pkt[0] = 0;
    pkt[1] = TFTP_OPCODE_ERROR;
    pkt[2] = code >> 8;
    pkt[3] = code & 0xff;
    strcpy((char *)&pkt[4], msg);

    sendto(sock, pkt, len, 0,
           (struct sockaddr *)client,
           sizeof(*client));
}

void OtaActor::tftp_ota_server_task(void *arg)
{
    int sock = -1;
    uint8_t rx_buf[4 + TFTP_BLOCK_SIZE];
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    esp_ota_handle_t ota_handle = 0;
    const esp_partition_t *ota_part = NULL;

    uint16_t expected_block = 1;
    bool ota_in_progress = false;

    INFO("Starting TFTP OTA server");

    // esp_task_wdt_add(NULL);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ERROR("socket() failed");
        vTaskDelete(NULL);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(TFTP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        ERROR("bind() failed");
        close(sock);
        vTaskDelete(NULL);
    }

    while (1)
    {
        //   esp_task_wdt_reset();

        int len = recvfrom(sock, rx_buf, sizeof(rx_buf), 0,
                           (struct sockaddr *)&client, &client_len);
        if (len < 4)
        {
            continue;
        }

        uint16_t opcode = (rx_buf[0] << 8) | rx_buf[1];

        /* ---------- WRQ ---------- */
        if (opcode == TFTP_OPCODE_WRQ)
        {
            INFO("WRQ received, starting OTA");

            ota_part = esp_ota_get_next_update_partition(NULL);
            if (!ota_part)
            {
                tftp_send_error(sock, &client, 1, "No OTA partition");
                continue;
            }

            if (esp_ota_begin(ota_part, OTA_SIZE_UNKNOWN, &ota_handle) != ESP_OK)
            {
                tftp_send_error(sock, &client, 2, "OTA begin failed");
                continue;
            }

            expected_block = 1;
            ota_in_progress = true;

            /* ACK block 0 */
            tftp_send_ack(sock, &client, 0);
        }

        /* ---------- DATA ---------- */
        else if (opcode == TFTP_OPCODE_DATA && ota_in_progress)
        {
            uint16_t block = (rx_buf[2] << 8) | rx_buf[3];
            int data_len = len - 4;

            if (block != expected_block)
            {
                WARN("Unexpected block %u (expected %u)",
                     block, expected_block);
                continue;
            }

            if (esp_ota_write(ota_handle, &rx_buf[4], data_len) != ESP_OK)
            {
                tftp_send_error(sock, &client, 3, "OTA write failed");
                esp_ota_abort(ota_handle);
                ota_in_progress = false;
                continue;
            }

            tftp_send_ack(sock, &client, block);
            expected_block++;

            /* Last packet < 512 bytes */
            if (data_len < TFTP_BLOCK_SIZE)
            {
                INFO("OTA image received, finalizing");

                if (esp_ota_end(ota_handle) == ESP_OK &&
                    esp_ota_set_boot_partition(ota_part) == ESP_OK)
                {

                    INFO("OTA successful, rebooting");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    esp_restart();
                }
                else
                {
                    ERROR("OTA finalize failed");
                }

                ota_in_progress = false;
            }
        }

        /* ---------- Unsupported ---------- */
        else
        {
            tftp_send_error(sock, &client, 4, "Unsupported operation");
        }
    }
}
