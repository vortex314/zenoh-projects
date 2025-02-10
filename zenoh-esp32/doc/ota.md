To use **Zenoh** for downloading the new firmware image instead of HTTP, we need to leverage Zenoh's data transmission capabilities. Zenoh allows you to publish and subscribe to data streams, which can be used to transfer the firmware binary directly over the Zenoh network.

Below is an updated implementation where the ESP32 subscribes to a Zenoh topic to receive the firmware binary and performs the OTA update using the received data.

---

### Updated Code: Using Zenoh for OTA

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "zenoh.h"

#define TAG "OTA_ZENOH"

// Zenoh topic to receive the firmware binary
#define FIRMWARE_TOPIC "firmware/update"

// OTA update task
static void ota_update_task(const uint8_t *data, size_t size) {
    esp_err_t err;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "Failed to find OTA update partition");
        return;
    }

    ESP_LOGI(TAG, "Writing firmware to partition: %s", update_partition->label);

    // Initialize OTA update
    esp_ota_handle_t ota_handle;
    err = esp_ota_begin(update_partition, size, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to begin OTA update: %s", esp_err_to_name(err));
        return;
    }

    // Write firmware data to the partition
    err = esp_ota_write(ota_handle, data, size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write OTA data: %s", esp_err_to_name(err));
        esp_ota_abort(ota_handle);
        return;
    }

    // Finalize OTA update
    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to end OTA update: %s", esp_err_to_name(err));
        return;
    }

    // Set the new firmware as bootable
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set boot partition: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "OTA update successful, restarting...");
    esp_restart();
}

// Zenoh message callback
void on_message(const z_sample_t *sample, void *arg) {
    ESP_LOGI(TAG, "Received firmware binary of size: %zu", sample->payload.len);

    // Start OTA update task with the received firmware binary
    xTaskCreatePinnedToCore((TaskFunction_t)ota_update_task, "ota_update_task", 8192, (void *)sample->payload.start, sample->payload.len, 5, NULL, 1);
}

void app_main(void) {
    // Initialize NVS (required for OTA)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Zenoh
    z_owned_config_t config = z_config_default();
    z_owned_session_t session = z_open(z_move(config));
    if (!z_check(session)) {
        ESP_LOGE(TAG, "Unable to open Zenoh session!");
        return;
    }

    // Declare a subscriber to receive the firmware binary
    z_owned_subscriber_t sub = z_declare_subscriber(z_loan(session), z_expr(FIRMWARE_TOPIC), z_move(z_sample_closure(on_message, NULL)), NULL);
    if (!z_check(sub)) {
        ESP_LOGE(TAG, "Unable to declare Zenoh subscriber!");
        return;
    }

    ESP_LOGI(TAG, "Waiting for firmware binary on topic: %s", FIRMWARE_TOPIC);

    // Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Cleanup
    z_undeclare_subscriber(z_move(sub));
    z_close(z_move(session));
}
```

---

### Explanation of the Code

1. **Zenoh Subscriber**:
   - The ESP32 subscribes to the topic `firmware/update` to receive the firmware binary.
   - When a message is received, the `on_message` callback is triggered.

2. **OTA Update**:
   - The received firmware binary is written to the OTA partition using the `esp_ota_write` function.
   - After writing the binary, the new firmware is set as the bootable partition using `esp_ota_set_boot_partition`.

3. **Reboot**:
   - Once the OTA update is complete, the ESP32 reboots to run the new firmware.

---

### Sending the Firmware Binary via Zenoh

To send the firmware binary to the ESP32, you can use the Zenoh CLI or a Zenoh client in Python/C++.

#### Example: Sending Firmware Binary Using Zenoh CLI

1. **Prepare the Firmware Binary**:
   - Build your firmware using `idf.py build`.
   - The firmware binary is located at `build/ota_zenoh.bin`.

2. **Publish the Binary**:
   - Use the Zenoh CLI to publish the binary to the `firmware/update` topic:

     ```bash
     zenoh pub firmware/update --file build/ota_zenoh.bin
     ```

---

### Notes

1. **Firmware Size**:
   - Ensure the firmware binary fits within the available OTA partition size on the ESP32.

2. **Zenoh Configuration**:
   - Configure Zenoh to use the appropriate transport (e.g., UDP, TCP, or serial) based on your network setup.

3. **Error Handling**:
   - Add robust error handling for cases like insufficient memory, invalid firmware binary, or network issues.

4. **Security**:
   - For production, consider encrypting the firmware binary and verifying its integrity using a checksum or digital signature.

---

This implementation allows you to perform OTA updates over Zenoh, eliminating the need for HTTP/HTTPS and leveraging Zenoh's efficient data transmission capabilities.