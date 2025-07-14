#include "home.h"
#include "../keypad/keypad.h"
#include <string.h>

static const char *TAG = "home";

// Home screen globals
lv_obj_t *home_screen = NULL;
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

// Create the home screen
void create_home_screen(void) {
    ESP_LOGI(TAG, "Creating home screen");
    
    // Create a new screen for home
    home_screen = lv_obj_create(NULL);
    
    // Add activity event handler to detect user interaction
    lv_obj_add_event_cb(home_screen, home_activity_event_handler, 
                       LV_EVENT_PRESSED | LV_EVENT_CLICKED | LV_EVENT_LONG_PRESSED, NULL);
    
    // Set background color
    lv_obj_set_style_bg_color(home_screen, lv_color_hex(0x1C1C1C), 0);
    
    // Welcome title
    lv_obj_t *title = lv_label_create(home_screen);
    lv_label_set_text(title, "Welcome!");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    // lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    
    // Access granted message
    lv_obj_t *access_msg = lv_label_create(home_screen);
    lv_label_set_text(access_msg, "Access Granted");
    lv_obj_set_style_text_color(access_msg, lv_color_hex(0xFFFFFF), 0);
    // lv_obj_set_style_text_font(access_msg, &lv_font_montserrat_18, 0);
    lv_obj_align(access_msg, LV_ALIGN_TOP_MID, 0, 70);
    
    // System status
    lv_obj_t *status_container = lv_obj_create(home_screen);
    lv_obj_set_size(status_container, 260, 120);
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
    
    // Timeout information
    lv_obj_t *timeout_info = lv_label_create(home_screen);
    char timeout_text[60];
    snprintf(timeout_text, sizeof(timeout_text), "Auto-logout in %d seconds of inactivity", IDLE_TIMEOUT_SECONDS);
    lv_label_set_text(timeout_info, timeout_text);
    lv_obj_set_style_text_color(timeout_info, lv_color_hex(0x888888), 0);
    // lv_obj_set_style_text_font(timeout_info, &lv_font_montserrat_12, 0);
    lv_obj_align(timeout_info, LV_ALIGN_BOTTOM_MID, 0, -50);
    
    // Manual logout button
    lv_obj_t *logout_btn = lv_btn_create(home_screen);
    lv_obj_set_size(logout_btn, 120, 40);
    lv_obj_align(logout_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(logout_btn, lv_color_hex(0xFF4444), 0);
    
    lv_obj_t *logout_label = lv_label_create(logout_btn);
    lv_label_set_text(logout_label, "Logout");
    lv_obj_set_style_text_color(logout_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(logout_label);
    
    // Add logout button event handler
    lv_obj_add_event_cb(logout_btn, home_activity_event_handler, LV_EVENT_CLICKED, NULL);
    
    // Create idle timer
    create_idle_timer();
    
    ESP_LOGI(TAG, "Home screen creation complete");
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
