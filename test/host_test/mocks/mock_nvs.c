#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Mock NVS implementation for host testing
typedef uint32_t nvs_handle_t;
typedef enum {
    NVS_READONLY,
    NVS_READWRITE
} nvs_open_mode_t;

typedef enum {
    ESP_OK = 0,
    ESP_ERR_NVS_NOT_FOUND = 0x1102,
    ESP_ERR_NVS_NO_FREE_PAGES = 0x1103,
    ESP_ERR_NVS_NEW_VERSION_FOUND = 0x1104,
} esp_err_t;

// Simple mock storage
static char mock_storage[10][2][256]; // 10 key-value pairs max
static int storage_count = 0;

esp_err_t nvs_flash_init(void) {
    return ESP_OK;
}

esp_err_t nvs_flash_erase(void) {
    storage_count = 0;
    return ESP_OK;
}

esp_err_t nvs_open(const char* name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle) {
    *out_handle = 1; // Mock handle
    return ESP_OK;
}

void nvs_close(nvs_handle_t handle) {
    // Mock implementation
}

esp_err_t nvs_set_str(nvs_handle_t handle, const char* key, const char* value) {
    if (storage_count >= 10) return ESP_ERR_NVS_NO_FREE_PAGES;
    
    strcpy(mock_storage[storage_count][0], key);
    strcpy(mock_storage[storage_count][1], value);
    storage_count++;
    return ESP_OK;
}

esp_err_t nvs_get_str(nvs_handle_t handle, const char* key, char* out_value, size_t* length) {
    for (int i = 0; i < storage_count; i++) {
        if (strcmp(mock_storage[i][0], key) == 0) {
            size_t len = strlen(mock_storage[i][1]) + 1;
            if (out_value == NULL) {
                *length = len;
                return ESP_OK;
            }
            if (*length < len) {
                return ESP_ERR_NVS_NOT_FOUND;
            }
            strcpy(out_value, mock_storage[i][1]);
            *length = len;
            return ESP_OK;
        }
    }
    return ESP_ERR_NVS_NOT_FOUND;
}

esp_err_t nvs_commit(nvs_handle_t handle) {
    return ESP_OK;
}
