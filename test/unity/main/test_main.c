#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char* TAG = "test_main";

void setUp(void) {
    // Set up before each test
    ESP_LOGI(TAG, "Setting up test");
}

void tearDown(void) {
    // Clean up after each test
    ESP_LOGI(TAG, "Tearing down test");
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting ESP32-2432S028R Test Suite");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Wait a bit for system to stabilize
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    ESP_LOGI(TAG, "Running Unity tests...");
    
    // Initialize Unity
    UNITY_BEGIN();
    
    // Add your test function calls here
    // unity_run_menu() allows interactive test selection
    unity_run_menu();
}
