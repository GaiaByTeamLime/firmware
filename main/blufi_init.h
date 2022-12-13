#pragma once

#include <stdio.h>
#include "esp_err.h"
#include "esp_blufi_api.h"
#include "blufi.h"
#include "blufi_init.h"
#include "blufi_security.h"
#include "esp_log.h"
#include "esp_blufi.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

esp_err_t esp_blufi_host_init(void);
esp_err_t esp_blufi_host_deinit(void);
esp_err_t esp_blufi_gap_register_callback(void);
esp_err_t esp_blufi_host_and_cb_init(esp_blufi_callbacks_t *example_callbacks);