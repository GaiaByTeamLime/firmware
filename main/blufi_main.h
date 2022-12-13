#pragma once

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
#include "gaia.h"

#include "esp_blufi.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "dht11.h"
#include <driver/adc.h>

#define EXAMPLE_WIFI_CONNECTION_MAXIMUM_RETRY CONFIG_WIFI_CONNECTION_MAXIMUM_RETRY
#define EXAMPLE_INVALID_REASON 255
#define EXAMPLE_INVALID_RSSI -128

void example_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);

#define WIFI_LIST_NUM 10

void initialise_wifi(void (*wifi_connected_callback)());