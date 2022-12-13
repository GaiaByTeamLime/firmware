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
#include "esp_http_client.h"
#include <bh1750.h>

#include "esp_blufi_api.h"
#include "blufi.h"
#include "blufi_init.h"
#include "blufi_security.h"
#include "blufi_main.h"

#include "esp_blufi.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "dht11.h"
#include <driver/adc.h>
#include "esp_crt_bundle.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

void handle_custom_data(esp_blufi_cb_param_t *param);
void read_sensors_and_format(char *post_data);
void send_to_temporal(char *mac, char *token, char *post_data);
void listen_for_devices();