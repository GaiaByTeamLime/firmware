#pragma once

#include <string.h>
#include "nvs_flash.h"
#include "esp_system.h"

#define TOKEN_LENGTH 36
#define SID_LENGTH 36
#define TOKEN_KEY "token_id"
#define SID_KEY "sensor_id"

void set_token(char *recv_data);