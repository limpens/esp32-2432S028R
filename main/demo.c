#include <stdio.h>
#include <string.h>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>

#include <lvgl.h>
#include <esp_lvgl_port.h>

#include "drivers/lcd.h"
#include "drivers/touch.h"
#include "frontend/keypad/keypad.h"
#include "frontend/calibration/calibration.h"
#include "frontend/home/home.h"

static const char *TAG = "demo";

// Forward declarations
static void touch_test_task(void *arg);

// Create keypad UI with NVS support
void create_keypad_ui(void) {
    ESP_LOGI(TAG, "create_keypad_ui called - initializing with NVS support");
    
    // Initialize NVS and load passkey
    esp_err_t err = keypad_init_nvs();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize keypad NVS");
    }
    
    // Create all screens
    create_keypad_screen();
    create_calibration_screen();
    create_home_screen();
    
    // Start with calibration screen for testing touch coordinates
    switch_to_calibration();
}

// Debug function to test touch periodically
static void touch_test_task(void *arg) {
    ESP_LOGI(TAG, "Touch test task started");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
        
        // Get input device (touch)
        lv_indev_t *indev = lv_indev_get_next(NULL);
        if (indev) {
            lv_indev_type_t type = lv_indev_get_type(indev);
            ESP_LOGI(TAG, "TOUCH TEST: Found input device type: %d", type);
            
            // Check if there's any touch activity
            lv_indev_state_t state = lv_indev_get_state(indev);
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            
            if (state == LV_INDEV_STATE_PRESSED) {
                ESP_LOGI(TAG, "TOUCH TEST: Active touch at (%ld, %ld)", (long)point.x, (long)point.y);
            } else {
                ESP_LOGI(TAG, "TOUCH TEST: No touch detected (last coordinates: %ld, %ld)", (long)point.x, (long)point.y);
            }
            
            // Also check raw touch data if available
            ESP_LOGI(TAG, "TOUCH TEST: Input device state: %d, coordinates: (%ld, %ld)", state, (long)point.x, (long)point.y);
        } else {
            ESP_LOGW(TAG, "TOUCH TEST: No input devices found!");
        }
    }
}

static esp_err_t app_lvgl_main(void)
{
    ESP_LOGI(TAG, "Starting LVGL main with keypad UI");
    
    lvgl_port_lock(0);

    // Create the keypad interface
    create_keypad_ui();

    ESP_LOGI(TAG, "LVGL keypad UI created successfully");
    lvgl_port_unlock();

    // Start touch test task
    xTaskCreate(touch_test_task, "touch_test", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Touch test task created");

    return ESP_OK;
}

void app_main(void)
{
    esp_lcd_panel_io_handle_t lcd_io;
    esp_lcd_panel_handle_t lcd_panel;
    esp_lcd_touch_handle_t tp;
    lv_display_t *lvgl_display = NULL;

    ESP_LOGI(TAG, "=== ESP32 CYD Touch Debug Application Starting ===");

    ESP_ERROR_CHECK(lcd_display_brightness_init());
    ESP_LOGI(TAG, "LCD brightness initialized");

    ESP_ERROR_CHECK(app_lcd_init(&lcd_io, &lcd_panel));
    ESP_LOGI(TAG, "LCD initialized");
    
    lvgl_display = app_lvgl_init(lcd_io, lcd_panel);
    if (lvgl_display == NULL)
    {
        ESP_LOGE(TAG, "Fatal error in app_lvgl_init");
        esp_restart();
    }
    ESP_LOGI(TAG, "LVGL initialized");
    
    ESP_ERROR_CHECK(touch_init(&tp));
    ESP_LOGI(TAG, "Touch controller initialized");
    
    // Use the simple touch integration with esp_lvgl_port
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_display,
        .handle = tp,
    };
    lvgl_port_add_touch(&touch_cfg);
    ESP_LOGI(TAG, "Touch added to LVGL port");

    ESP_ERROR_CHECK(lcd_display_brightness_set(75));
    ESP_LOGI(TAG, "LCD brightness set to 75%%");
    
    ESP_ERROR_CHECK(lcd_display_rotate(lvgl_display, LV_DISPLAY_ROTATION_90));
    ESP_LOGI(TAG, "LCD rotated to landscape mode");
    
    ESP_ERROR_CHECK(app_lvgl_main());
    ESP_LOGI(TAG, "LVGL main initialized");

    ESP_LOGI(TAG, "=== Application setup complete - Touch the screen ===");
    ESP_LOGI(TAG, "=== Watch serial output for touch debug information ===");

    // Main loop - just keep alive
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
