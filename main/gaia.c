#include "gaia.h"

void set_token(char *recv_data)
{
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
    strncpy(token, recv_data, TOKEN_LENGTH);
    token[TOKEN_LENGTH] = '\0';
    printf("Token set to %s!\n", token);

    // Write
    printf("Updating token in NVS ... ");
    ret = nvs_set_str(nvs, TOKEN_KEY, token);
    printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");

    char sid[SID_LENGTH + 1] = {0};
    strncpy(sid, recv_data + TOKEN_LENGTH, SID_LENGTH);
    sid[SID_LENGTH] = '\0';
    printf("sid set to %s!\n", sid);

    // Write
    printf("Updating sid in NVS ... ");
    ret = nvs_set_str(nvs, SID_KEY, sid);
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