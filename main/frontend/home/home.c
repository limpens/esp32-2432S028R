#include "home.h"
#include "../keypad/keypad.h"
#include "../passkey/passkey_change.h"
#include "../calibration/calibration.h"
#include <string.h>
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_flash.h"
#include "esp_chip_info.h"
#include "time.h"
#include "sys/time.h"

static const char *TAG = "home";

// Home screen globals
lv_obj_t *home_screen = NULL;
lv_obj_t *tabview = NULL;
lv_obj_t *tab_home = NULL;
lv_obj_t *tab_calendar = NULL;
lv_obj_t *tab_info = NULL;
lv_obj_t *tab_monitor = NULL;
lv_obj_t *tab_settings = NULL;

// Monitor update timer
esp_timer_handle_t monitor_timer = NULL;
static bool monitor_timer_created = false;

// Activity timer
esp_timer_handle_t idle_timer = NULL;
static bool timer_created = false;

// Timer configuration
#define IDLE_TIMEOUT_SECONDS 10
#define IDLE_TIMEOUT_US (IDLE_TIMEOUT_SECONDS * 1000000) // Convert to microseconds

// Idle timer callback - switches back to keypad after timeout
void home_idle_timeout_callback(void *arg) {
    ESP_LOGI(TAG, "Idle timeout reached, switching back to keypad");
    home_stop_idle_timer();
    switch_to_keypad();
}

// Activity event handler - resets timer on any touch/interaction and handles logout
void home_activity_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    
    // Check if logout button was clicked
    if (code == LV_EVENT_CLICKED) {
        // Check if this is the logout button by checking its text
        lv_obj_t *child = lv_obj_get_child(obj, 0);
        if (child && lv_obj_check_type(child, &lv_label_class)) {
            const char *text = lv_label_get_text(child);
            if (text && strcmp(text, "Logout") == 0) {
                ESP_LOGI(TAG, "Logout button clicked, switching to keypad");
                home_stop_idle_timer();
                switch_to_keypad();
                return;
            }
            else if (text && strcmp(text, "Change Passkey") == 0) {
                ESP_LOGI(TAG, "Change Passkey button clicked, switching to passkey change screen");
                home_stop_idle_timer();
                switch_to_passkey_change();
                return;
            }
            else if (text && strcmp(text, "Touch Calibration") == 0) {
                ESP_LOGI(TAG, "Touch Calibration button clicked, switching to calibration screen");
                home_stop_idle_timer();
                switch_to_calibration();
                return;
            }
        }
    }
    
    // Reset timer on any user activity
    switch(code) {
        case LV_EVENT_PRESSED:
        case LV_EVENT_CLICKED:
        case LV_EVENT_LONG_PRESSED:
        case LV_EVENT_KEY:
            ESP_LOGI(TAG, "User activity detected, resetting idle timer");
            home_reset_idle_timer();
            break;
        default:
            // Don't reset timer for other events
            break;
    }
}

// Create idle timer
static esp_err_t create_idle_timer(void) {
    if (timer_created) {
        return ESP_OK; // Timer already exists
    }
    
    esp_timer_create_args_t timer_args = {
        .callback = home_idle_timeout_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "home_idle_timer"
    };
    
    esp_err_t err = esp_timer_create(&timer_args, &idle_timer);
    if (err == ESP_OK) {
        timer_created = true;
        ESP_LOGI(TAG, "Idle timer created successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create idle timer: %s", esp_err_to_name(err));
    }
    
    return err;
}

// Start the idle timer
void home_start_idle_timer(void) {
    if (!timer_created) {
        if (create_idle_timer() != ESP_OK) {
            return;
        }
    }
    
    // Stop any existing timer first
    esp_timer_stop(idle_timer);
    
    // Start the timer
    esp_err_t err = esp_timer_start_once(idle_timer, IDLE_TIMEOUT_US);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Idle timer started (%d seconds)", IDLE_TIMEOUT_SECONDS);
    } else {
        ESP_LOGE(TAG, "Failed to start idle timer: %s", esp_err_to_name(err));
    }
}

// Stop the idle timer
void home_stop_idle_timer(void) {
    if (timer_created && idle_timer) {
        esp_timer_stop(idle_timer);
        ESP_LOGI(TAG, "Idle timer stopped");
    }
}

// Reset the idle timer (restart it)
void home_reset_idle_timer(void) {
    home_stop_idle_timer();
    home_start_idle_timer();
}

// Forward declarations for tab creation functions
static void create_home_tab(void);
static void create_calendar_tab(void);
static void create_info_tab(void);
static void create_monitor_tab(void);
static void create_settings_tab(void);
static void update_monitor_tab(void);
static void monitor_timer_callback(void *arg);

// Create the home screen
void create_home_screen(void) {
    ESP_LOGI(TAG, "Creating home screen with tabview");
    
    // Create a new screen for home
    home_screen = lv_obj_create(NULL);
    
    // Add activity event handler to detect user interaction
    lv_obj_add_event_cb(home_screen, home_activity_event_handler, 
                       LV_EVENT_PRESSED | LV_EVENT_CLICKED | LV_EVENT_LONG_PRESSED, NULL);
    
    // Set background color
    lv_obj_set_style_bg_color(home_screen, lv_color_hex(0x1C1C1C), 0);
    
    // Create tabview
    tabview = lv_tabview_create(home_screen);
    lv_obj_set_size(tabview, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(tabview, lv_color_hex(0x1C1C1C), 0);
    lv_obj_add_event_cb(tabview, home_activity_event_handler, LV_EVENT_ALL, NULL);
    
    // Set tab position to top
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_LEFT);
    lv_tabview_set_tab_bar_size(tabview, 80);
    
    // Create tabs
    tab_home = lv_tabview_add_tab(tabview, "Home");
    tab_calendar = lv_tabview_add_tab(tabview, "Calendar");
    tab_info = lv_tabview_add_tab(tabview, "Info");
    tab_monitor = lv_tabview_add_tab(tabview, "Monitor");
    tab_settings = lv_tabview_add_tab(tabview, "Settings");
    
    // Set tab styles
    lv_obj_set_style_bg_color(tab_home, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_color(tab_calendar, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_color(tab_info, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_color(tab_monitor, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_color(tab_settings, lv_color_hex(0x1C1C1C), 0);
    
    // Create content for each tab
    create_home_tab();
    create_calendar_tab();
    create_info_tab();
    create_monitor_tab();
    create_settings_tab();
    
    // Create idle timer
    create_idle_timer();
    
    ESP_LOGI(TAG, "Home screen with tabs created successfully");
}

// Create home tab content
static void create_home_tab(void) {
    // Welcome title
    // lv_obj_t *title = lv_label_create(tab_home);
    // lv_label_set_text(title, "Welcome!");
    // lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    // lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Access granted message
    // lv_obj_t *access_msg = lv_label_create(tab_home);
    // lv_label_set_text(access_msg, "Access Granted");
    // lv_obj_set_style_text_color(access_msg, lv_color_hex(0xFFFFFF), 0);
    // lv_obj_align(access_msg, LV_ALIGN_TOP_MID, 0, 50);
    
    // System status container
    lv_obj_t *status_container = lv_obj_create(tab_home);
    lv_obj_set_size(status_container, 180, 180);
    lv_obj_align(status_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(status_container, lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_color(status_container, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(status_container, 2, 0);
    lv_obj_set_style_radius(status_container, 10, 0);
    
    // Status items
    lv_obj_t *status1 = lv_label_create(status_container);
    lv_label_set_text(status1, "System: Online");
    lv_obj_set_style_text_color(status1, lv_color_hex(0x00FF00), 0);
    lv_obj_align(status1, LV_ALIGN_TOP_LEFT, 10, 10);
    
    lv_obj_t *status2 = lv_label_create(status_container);
    lv_label_set_text(status2, "Security: Active");
    lv_obj_set_style_text_color(status2, lv_color_hex(0x00FF00), 0);
    lv_obj_align(status2, LV_ALIGN_TOP_LEFT, 10, 35);
    
    lv_obj_t *status3 = lv_label_create(status_container);
    lv_label_set_text(status3, "Display: CYD 2.8\"");
    lv_obj_set_style_text_color(status3, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(status3, LV_ALIGN_TOP_LEFT, 10, 60);
    
    lv_obj_t *status4 = lv_label_create(status_container);
    lv_label_set_text(status4, "Touch: Calibrated");
    lv_obj_set_style_text_color(status4, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(status4, LV_ALIGN_TOP_LEFT, 10, 85);
}

// Create calendar tab content
static void create_calendar_tab(void) {
    // Title
    lv_obj_t *title = lv_label_create(tab_calendar);
    lv_label_set_text(title, "Calendar");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Current date and time - simplified
    lv_obj_t *date_label = lv_label_create(tab_calendar);
    lv_label_set_text(date_label, "Date: 2025-07-14");
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, -40);
    
    lv_obj_t *time_label = lv_label_create(tab_calendar);
    lv_label_set_text(time_label, "Time: 12:00:00");
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -10);
    
    // Month and year
    lv_obj_t *month_year_label = lv_label_create(tab_calendar);
    lv_label_set_text(month_year_label, "Month: July 2025");
    lv_obj_set_style_text_color(month_year_label, lv_color_hex(0x00FFFF), 0);
    lv_obj_align(month_year_label, LV_ALIGN_CENTER, 0, 20);
    
    // Day of week
    lv_obj_t *day_label = lv_label_create(tab_calendar);
    lv_label_set_text(day_label, "Day: Monday");
    lv_obj_set_style_text_color(day_label, lv_color_hex(0xFFFF00), 0);
    lv_obj_align(day_label, LV_ALIGN_CENTER, 0, 50);
}

// Create info tab content
static void create_info_tab(void) {
    // Title
    lv_obj_t *title = lv_label_create(tab_info);
    lv_label_set_text(title, "Device Information");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Device info - simplified without chip info API
    lv_obj_t *device_info = lv_label_create(tab_info);
    
    char info_text[256];
    snprintf(info_text, sizeof(info_text),
        "Device: ESP32-2432S028R\n"
        "Display: 2.8\" ILI9341\n"
        "Touch: XPT2046\n"
        "Resolution: 320x240\n"
        "Framework: ESP-IDF\n"
        "UI: LVGL");
    
    lv_label_set_text(device_info, info_text);
    lv_obj_set_style_text_color(device_info, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(device_info, LV_ALIGN_TOP_LEFT, 10, 40);
    
    // Flash firmware button
    lv_obj_t *flash_btn = lv_btn_create(tab_info);
    lv_obj_set_size(flash_btn, 120, 40);
    lv_obj_align(flash_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(flash_btn, lv_color_hex(0x0066CC), 0);
    
    lv_obj_t *flash_label = lv_label_create(flash_btn);
    lv_label_set_text(flash_label, "Flash FW");
    lv_obj_center(flash_label);
}

// Create monitor tab content
static void create_monitor_tab(void) {
    // Title
    lv_obj_t *title = lv_label_create(tab_monitor);
    lv_label_set_text(title, "System Monitor");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Memory info (will be updated by timer)
    lv_obj_t *mem_label = lv_label_create(tab_monitor);
    lv_obj_set_style_text_color(mem_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(mem_label, LV_ALIGN_TOP_LEFT, 10, 40);
    
    // Flash info
    lv_obj_t *flash_label = lv_label_create(tab_monitor);
    lv_obj_set_style_text_color(flash_label, lv_color_hex(0x00FFFF), 0);
    lv_obj_align(flash_label, LV_ALIGN_TOP_LEFT, 10, 100);
    
    // Update monitor data initially
    update_monitor_tab();
    
    // Create monitor update timer
    if (!monitor_timer_created) {
        esp_timer_create_args_t timer_args = {
            .callback = monitor_timer_callback,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "monitor_timer"
        };
        
        esp_err_t err = esp_timer_create(&timer_args, &monitor_timer);
        if (err == ESP_OK) {
            monitor_timer_created = true;
            esp_timer_start_periodic(monitor_timer, 2000000); // Update every 2 seconds
        }
    }
}

// Create settings tab content
static void create_settings_tab(void) {
    // Title
    lv_obj_t *title = lv_label_create(tab_settings);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Change passkey button
    lv_obj_t *passkey_btn = lv_btn_create(tab_settings);
    lv_obj_set_size(passkey_btn, 150, 40);
    lv_obj_align(passkey_btn, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_bg_color(passkey_btn, lv_color_hex(0x0066CC), 0);
    lv_obj_add_event_cb(passkey_btn, home_activity_event_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *passkey_label = lv_label_create(passkey_btn);
    lv_label_set_text(passkey_label, "Change Passkey");
    lv_obj_center(passkey_label);
    
    // Touch calibration button
    lv_obj_t *calib_btn = lv_btn_create(tab_settings);
    lv_obj_set_size(calib_btn, 150, 40);
    lv_obj_align(calib_btn, LV_ALIGN_CENTER, 0, -5);
    lv_obj_set_style_bg_color(calib_btn, lv_color_hex(0x006600), 0);
    lv_obj_add_event_cb(calib_btn, home_activity_event_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *calib_label = lv_label_create(calib_btn);
    lv_label_set_text(calib_label, "Touch Calibration");
    lv_obj_center(calib_label);
    
    // Logout button
    lv_obj_t *logout_btn = lv_btn_create(tab_settings);
    lv_obj_set_size(logout_btn, 100, 40);
    lv_obj_align(logout_btn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_style_bg_color(logout_btn, lv_color_hex(0xCC0000), 0);
    lv_obj_add_event_cb(logout_btn, home_activity_event_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *logout_label = lv_label_create(logout_btn);
    lv_label_set_text(logout_label, "Logout");
    lv_obj_center(logout_label);
    
    // Timeout information
    lv_obj_t *timeout_info = lv_label_create(tab_settings);
    char timeout_text[60];
    snprintf(timeout_text, sizeof(timeout_text), "Auto-logout in %d seconds", IDLE_TIMEOUT_SECONDS);
    lv_label_set_text(timeout_info, timeout_text);
    lv_obj_set_style_text_color(timeout_info, lv_color_hex(0x888888), 0);
    lv_obj_align(timeout_info, LV_ALIGN_BOTTOM_MID, 0, -10);
}

// Update monitor tab data
static void update_monitor_tab(void) {
    if (!tab_monitor) return;
    
    // Get memory info
    size_t free_heap = esp_get_free_heap_size();
    size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    size_t used_heap = total_heap - free_heap;
    
    // Find memory label (second child)
    lv_obj_t *mem_label = lv_obj_get_child(tab_monitor, 1);
    if (mem_label) {
        char mem_text[128];
        snprintf(mem_text, sizeof(mem_text),
            "Memory Usage:\n"
            "Free: %u bytes\n"
            "Used: %u bytes\n"
            "Total: %u bytes",
            (unsigned)free_heap, (unsigned)used_heap, (unsigned)total_heap);
        lv_label_set_text(mem_label, mem_text);
    }
    
    // Flash info - simplified
    lv_obj_t *flash_label = lv_obj_get_child(tab_monitor, 2);
    if (flash_label) {
        char flash_text[64];
        snprintf(flash_text, sizeof(flash_text),
            "Flash: Available\n"
            "Storage: 4MB typical");
        lv_label_set_text(flash_label, flash_text);
    }
}

// Monitor timer callback
static void monitor_timer_callback(void *arg) {
    update_monitor_tab();
}

// Switch to home screen and start idle timer
void switch_to_home(void) {
    ESP_LOGI(TAG, "Switching to home screen");
    if (home_screen) {
        lv_screen_load(home_screen);
        home_start_idle_timer();
        ESP_LOGI(TAG, "Home screen loaded with idle timer started");
    } else {
        ESP_LOGW(TAG, "Home screen not created yet");
    }
}

// Get home screen object (for external access)
lv_obj_t* get_home_screen(void) {
    return home_screen;
}
