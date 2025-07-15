#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char* TAG = "test_keypad";

// Mock keypad functionality tests
TEST_CASE("Keypad input validation test", "[keypad]")
{
    ESP_LOGI(TAG, "Testing keypad input validation");
    
    // Test PIN validation logic
    const char* test_pins[] = {
        "1234",     // Valid 4-digit PIN
        "0000",     // Valid but weak PIN
        "9999",     // Valid but weak PIN  
        "123",      // Invalid - too short
        "12345",    // Invalid - too long
        "abcd",     // Invalid - non-numeric
        "",         // Invalid - empty
    };
    
    bool expected_results[] = {true, true, true, false, false, false, false};
    
    for (int i = 0; i < sizeof(test_pins) / sizeof(test_pins[0]); i++) {
        bool is_valid = (strlen(test_pins[i]) == 4);
        
        // Additional check for numeric characters
        if (is_valid) {
            for (int j = 0; j < 4; j++) {
                if (test_pins[i][j] < '0' || test_pins[i][j] > '9') {
                    is_valid = false;
                    break;
                }
            }
        }
        
        TEST_ASSERT_EQUAL(expected_results[i], is_valid);
        ESP_LOGI(TAG, "PIN '%s' validation: %s", test_pins[i], is_valid ? "PASS" : "FAIL");
    }
    
    ESP_LOGI(TAG, "Keypad input validation test passed");
}

TEST_CASE("NVS PIN storage test", "[keypad][nvs]")
{
    ESP_LOGI(TAG, "Testing NVS PIN storage");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    // Test storing a PIN
    const char* test_pin = "1234";
    ret = nvs_set_str(nvs_handle, "user_pin", test_pin);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    ret = nvs_commit(nvs_handle);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    // Test reading the PIN back
    size_t required_size = 0;
    ret = nvs_get_str(nvs_handle, "user_pin", NULL, &required_size);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(5, required_size); // 4 chars + null terminator
    
    char* read_pin = malloc(required_size);
    TEST_ASSERT_NOT_NULL(read_pin);
    
    ret = nvs_get_str(nvs_handle, "user_pin", read_pin, &required_size);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL_STRING(test_pin, read_pin);
    
    free(read_pin);
    nvs_close(nvs_handle);
    
    ESP_LOGI(TAG, "NVS PIN storage test passed");
}
