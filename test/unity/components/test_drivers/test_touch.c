#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

static const char* TAG = "test_touch";

TEST_CASE("Touch controller GPIO configuration test", "[touch]")
{
    ESP_LOGI(TAG, "Testing touch controller GPIO configuration");
    
    // Test CS pin configuration
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_33),  // Common CS pin for touch
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    // Test IRQ pin configuration
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_36);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    
    ret = gpio_config(&io_conf);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    ESP_LOGI(TAG, "Touch controller GPIO configuration test passed");
}

TEST_CASE("Touch coordinate validation test", "[touch]")
{
    ESP_LOGI(TAG, "Testing touch coordinate validation");
    
    // Test coordinate bounds checking
    struct touch_point {
        int16_t x;
        int16_t y;
        bool valid;
    };
    
    struct touch_point test_points[] = {
        {100, 150, true},   // Valid point
        {-10, 150, false},  // Invalid X (negative)
        {100, -20, false},  // Invalid Y (negative)
        {400, 150, false},  // Invalid X (too large for 320px display)
        {100, 300, false},  // Invalid Y (too large for 240px display)
        {319, 239, true},   // Valid max point
        {0, 0, true},       // Valid min point
    };
    
    for (int i = 0; i < sizeof(test_points) / sizeof(test_points[0]); i++) {
        bool is_valid = (test_points[i].x >= 0 && test_points[i].x < 320 &&
                        test_points[i].y >= 0 && test_points[i].y < 240);
        
        TEST_ASSERT_EQUAL(test_points[i].valid, is_valid);
        ESP_LOGI(TAG, "Point (%d, %d) validation: %s", 
                test_points[i].x, test_points[i].y, 
                is_valid ? "PASS" : "FAIL");
    }
    
    ESP_LOGI(TAG, "Touch coordinate validation test passed");
}
