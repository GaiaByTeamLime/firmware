#include "gaia.h"
static const char *TAG = "GAIA";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            if (evt->user_data)
            {
                memcpy(evt->user_data + output_len, evt->data, evt->data_len);
            }
            else
            {
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client));
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                memcpy(output_buffer + output_len, evt->data, evt->data_len);
            }
            output_len += evt->data_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}

static esp_blufi_callbacks_t example_callbacks = {
    .event_cb = example_event_callback,
    .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    .encrypt_func = blufi_aes_encrypt,
    .decrypt_func = blufi_aes_decrypt,
    .checksum_func = blufi_crc_checksum,
};

void handle_custom_data(esp_blufi_cb_param_t *param)
{

    if (param->custom_data.data_len != TOKEN_LENGTH)
    {
        printf("Custom data is not a token! length=%d\n", param->custom_data.data_len);
        return;
    }

    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("storage", NVS_READWRITE, &nvs);
    if (ret != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
        return;
    }
    printf("Done\n");

    char token[TOKEN_LENGTH + 1] = {0};
    strncpy(token, (char *)param->custom_data.data, TOKEN_LENGTH);
    token[TOKEN_LENGTH] = '\0';
    printf("Token set to %s!\n", token);

    // Write
    printf("Updating token in NVS ... ");
    ret = nvs_set_str(nvs, TOKEN_KEY, token);
    printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");

    char mac[50] = {0};
    sprintf(mac, ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));
    BLUFI_INFO("MAC: %s\n", mac);

    // Write
    printf("Updating mac in NVS ... ");
    ret = nvs_set_str(nvs, MAC_KEY, mac);
    printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");

    // Commit written value.
    // After setting any values, nvs_commit() must be called to ensure changes are written
    // to flash storage. Implementations may write to storage at other times,
    // but this is not guaranteed.
    printf("Committing updates in NVS ... ");
    ret = nvs_commit(nvs);
    printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");

    // Close
    nvs_close(nvs);
    esp_restart();
}

static struct dht11_reading stDht11Reading;

void read_sensors_and_format(char *post_data)
{
    // FIRST ENABLE THE SENSORS JESSE
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 1);
    vTaskDelay(100);

    ESP_ERROR_CHECK(i2cdev_init());

    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));

    ESP_ERROR_CHECK(bh1750_init_desc(&dev, BH1750_ADDR_LO, 0, I2C_MASTER_SDA, I2C_MASTER_SCL));
    ESP_ERROR_CHECK(bh1750_setup(&dev, BH1750_MODE_CONTINUOUS, BH1750_RES_HIGH));

    int tries = 0;

    DHT11_init(DHT11_DATA_GPIO);

    do
    {
        printf("Trying DHT11 read %d\n", tries);
        stDht11Reading = DHT11_read();

        if (DHT11_OK == stDht11Reading.status)
        {
            printf(
                "Temperature: %d °C\tHumidity: %d %%\n",
                stDht11Reading.temperature,
                stDht11Reading.humidity);
        }
        else
        {
            printf(
                "Cannot read from sensor: %s\n",
                DHT11_TIMEOUT_ERROR == stDht11Reading.status
                    ? "Timeout"
                    : "Bad CRC");
        }
        vTaskDelay(pdMS_TO_TICKS(20));
        tries++;
    } while (DHT11_OK != stDht11Reading.status && tries < 50);

    tries = 0;
    uint16_t lux;

    while (lux == 0 && tries < 50)
    {
        printf("Trying bh1750 read %d\n", tries);
        if (bh1750_read(&dev, &lux) != ESP_OK)
            printf("Could not read lux data\n");
        else
            printf("Lux: %d\n", lux);
        vTaskDelay(pdMS_TO_TICKS(20));
        tries++;
    }

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); // capacitibe
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11); // vbat
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);  // resistive

    int soil_humid = adc1_get_raw(ADC1_CHANNEL_4);
    int bat = adc1_get_raw(ADC1_CHANNEL_5);
    int soil_salt = adc1_get_raw(ADC1_CHANNEL_6);

    // We're done, disable the sensors again
    gpio_set_level(GPIO_NUM_4, 0);

    sprintf(post_data,
            "{\"illumination\":%d,\"humidity\":%d,\"temperature\":%d,\"voltage\":%d,\"soilHumidity\":%d,\"soilSalt\":%d}",
            lux, stDht11Reading.humidity, stDht11Reading.temperature, bat, soil_humid, soil_salt);
}

void send_to_temporal(char *mac, char *token, char *post_data)
{
    vTaskDelay(10);

    // Connect to server and enter deepsleep directly.
    printf("Token value: %s\n", token);
    vTaskDelay(10);
    printf("Mac value: %s\n", mac);
    vTaskDelay(10);

    char url[100];
    vTaskDelay(10);
    sprintf(url, "%s/log/%s", TEMPORAL_BASE_URL, mac);
    vTaskDelay(10);
    printf("url: %s\n", url);
    vTaskDelay(10);

    char auth_header[100];
    vTaskDelay(10);
    sprintf(auth_header, "Bearer %s", token);
    vTaskDelay(10);
    printf("auth: %s\n", auth_header);
    vTaskDelay(10);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
    };
    printf("a");
    esp_http_client_handle_t client = esp_http_client_init(&config);
    printf("b");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    printf("c");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    printf("d");
    esp_http_client_set_header(client, "Content-Type", "application/json");
    printf("e");
    esp_http_client_set_header(client, "Authorization", auth_header);
    printf("f");
    esp_err_t err = esp_http_client_perform(client);
    printf("g");

    if (err == ESP_OK)
    {
        printf("h");
        BLUFI_INFO("HTTPS Status = %d, content_length = %lld",
                   esp_http_client_get_status_code(client),
                   esp_http_client_get_content_length(client));
    }
    else
    {
        printf("i");
        BLUFI_ERROR("Error perform http request %s", esp_err_to_name(err));
    }
    printf("j");
    esp_http_client_cleanup(client);
    printf("k");
}

void listen_for_devices()
{

    BLUFI_ERROR("%s read token failed.\n", __func__);

    // Continue with enabling bluetooth
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        BLUFI_ERROR("%s initialize bt controller failed: %s\n", __func__, esp_err_to_name(ret));
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        BLUFI_ERROR("%s enable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_blufi_host_and_cb_init(&example_callbacks);
    if (ret)
    {
        BLUFI_ERROR("%s initialise failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    BLUFI_INFO("BLUFI VERSION %04x\n", esp_blufi_get_version());

    // Delay for 5 minutes
    vTaskDelay(5 * 60 * 1000 / portTICK_PERIOD_MS);
}
