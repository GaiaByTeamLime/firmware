/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#define BLUFI_DEVICE_NAME "Gaia Plant Sensor"
#define BLUFI_EXAMPLE_TAG "BLUFI_EXAMPLE"
#define BLUFI_INFO(fmt, ...) ESP_LOGI(BLUFI_EXAMPLE_TAG, fmt, ##__VA_ARGS__)
#define BLUFI_ERROR(fmt, ...) ESP_LOGE(BLUFI_EXAMPLE_TAG, fmt, ##__VA_ARGS__)

#define LOWER_BUTTON GPIO_NUM_35
#define TOKEN_LENGTH 36
#define DEEP_SLEEP_DURATION 30 * 60
#define TOKEN_KEY "token_id"
#define MAC_KEY "mac_key"
#define TEMPORAL_BASE_URL "https://temporal.dev.gaiaplant.app"
#define SENSOR_TYPE DHT_TYPE_DHT11
#define DHT11_DATA_GPIO GPIO_NUM_16
#define I2C_MASTER_SDA GPIO_NUM_25
#define I2C_MASTER_SCL GPIO_NUM_26
