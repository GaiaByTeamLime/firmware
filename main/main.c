#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include <bh1750.h>

#include "esp_blufi_api.h"
#include "blufi.h"
#include "blufi_init.h"
#include "blufi_security.h"
#include "blufi_main.h"
#include "gaia.h"

#include "esp_blufi.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "dht11.h"
#include <driver/adc.h>
#include "blufi_main.h"
static const char *TAG = "MAIN";
static bool sta_connected = false;

void wifi_callback()
{
    printf("setting sta_connected");
    sta_connected = true;
}

void app_main(void)
{
    esp_err_t ret;

    // Set lower button to input mode
    gpio_set_direction(LOWER_BUTTON, GPIO_MODE_INPUT);

    vTaskDelay(10);

    // If the button is pressed during boot
    int reset = !gpio_get_level(LOWER_BUTTON);
    if (reset)
    {
        // Reset flash
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());

        nvs_handle_t nvs;
        ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs));
        ESP_ERROR_CHECK(nvs_erase_all(nvs));
        ESP_ERROR_CHECK(nvs_commit(nvs));
        nvs_close(nvs);

        // Wait until the button is released
        do
        {
            reset = !gpio_get_level(LOWER_BUTTON);
            BLUFI_INFO("Waiting for release..");
            vTaskDelay(1);
        } while (reset);

        // Restart esp
        BLUFI_INFO("Restart..");
        esp_restart();
    }

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Start wifi
    initialise_wifi(wifi_callback);

    // Read if we have a token
    nvs_handle_t nvs;
    ret = nvs_open("storage", NVS_READWRITE, &nvs);
    if (ret != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
        return;
    }

    // Read
    size_t required_size = 100;
    char token[100];
    ret = nvs_get_str(nvs, TOKEN_KEY, token, &required_size);

    char mac[100];
    ret += nvs_get_str(nvs, MAC_KEY, mac, &required_size);

    // Close
    nvs_close(nvs);

    if (ret == ESP_OK)
    {
        // Let's dial the wifi down a bit
        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);

        char json_post_data[500];
        read_sensors_and_format(json_post_data);
        printf("json: %s\n", json_post_data);

        while (!sta_connected)
            vTaskDelay(10);

        send_to_temporal(mac, token, json_post_data);
    }
    else
    {
        listen_for_devices();
    }

    // Enter deep sleep
    printf("Entering sleep. Goodnight.");
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION * 1000000));
    esp_deep_sleep_start();
}
