#include "calibration.h"
#include "../keypad/keypad.h"

static const char *TAG = "calibration";

// Calibration point globals
lv_obj_t *calibration_screen = NULL;
lv_obj_t *cal_point1 = NULL;
lv_obj_t *cal_point2 = NULL;
lv_obj_t *cal_point3 = NULL;
lv_obj_t *cal_point4 = NULL;
lv_obj_t *coord_label = NULL;
bool cal_point1_touched = false;
bool cal_point2_touched = false;
bool cal_point3_touched = false;
bool cal_point4_touched = false;

// Helper function to update coordinate display
void update_coordinate_display(lv_point_t *point) {
    if (coord_label != NULL && point != NULL) {
        char coord_text[50];
        snprintf(coord_text, sizeof(coord_text), "Touch: (%ld, %ld)", (long)point->x, (long)point->y);
        lv_label_set_text(coord_label, coord_text);
    }
}

// Global event handler for debugging all LVGL events
void global_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    
    // Get the touch coordinates for all events
    lv_indev_t *indev = lv_indev_get_act();
    lv_point_t point = {0, 0};
    if (indev) {
        lv_indev_get_point(indev, &point);
    }
    
    // Log only important touch-related events with coordinates
    switch(code) {
        case LV_EVENT_PRESSED:
            ESP_LOGI(TAG, "TOUCH: PRESSED at (%ld, %ld)", (long)point.x, (long)point.y);
            update_coordinate_display(&point);
            break;
        case LV_EVENT_PRESSING:
            // Continuous updates while pressing (less frequent logging)
            update_coordinate_display(&point);
            break;
        case LV_EVENT_RELEASED:
            ESP_LOGI(TAG, "TOUCH: RELEASED at (%ld, %ld)", (long)point.x, (long)point.y);
            update_coordinate_display(&point);
            break;
        case LV_EVENT_CLICKED:
            ESP_LOGI(TAG, "TOUCH: CLICKED at (%ld, %ld)", (long)point.x, (long)point.y);
            update_coordinate_display(&point);
            break;
        default:
            // Don't log other events to reduce noise
            break;
    }
}

// Create separate calibration screen
void create_calibration_screen(void) {
    ESP_LOGI(TAG, "Creating calibration screen");
    
    // Create a new screen for calibration
    calibration_screen = lv_obj_create(NULL);
    
    // Add global event handler for touch coordinate debugging - register for more events
    lv_obj_add_event_cb(calibration_screen, global_event_handler, 
                       LV_EVENT_PRESSED | LV_EVENT_RELEASED | LV_EVENT_CLICKED | LV_EVENT_PRESSING, 
                       NULL);
    
    // Title
    lv_obj_t *title = lv_label_create(calibration_screen);
    lv_label_set_text(title, "Touch Calibration");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Instructions
    lv_obj_t *instructions = lv_label_create(calibration_screen);
    lv_label_set_text(instructions, "Touch red corners to calibrate\nWatch coordinates below");
    lv_obj_set_style_text_align(instructions, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(instructions, LV_ALIGN_CENTER, 0, -50);
    
    // Real-time coordinate display
    coord_label = lv_label_create(calibration_screen);
    lv_label_set_text(coord_label, "Touch: (---, ---)");
    lv_obj_set_style_text_color(coord_label, lv_color_hex(0x0000FF), 0);
    lv_obj_set_style_text_align(coord_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(coord_label, LV_ALIGN_CENTER, 0, -10);
    
    // Touch indicator points
    cal_point1 = lv_obj_create(calibration_screen);
    lv_obj_set_size(cal_point1, 20, 20);
    lv_obj_set_style_bg_color(cal_point1, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_radius(cal_point1, 10, 0);
    lv_obj_align(cal_point1, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_event_cb(cal_point1, calibration_point_event_handler, LV_EVENT_CLICKED, NULL);
    
    cal_point2 = lv_obj_create(calibration_screen);
    lv_obj_set_size(cal_point2, 20, 20);
    lv_obj_set_style_bg_color(cal_point2, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_radius(cal_point2, 10, 0);
    lv_obj_align(cal_point2, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_event_cb(cal_point2, calibration_point_event_handler, LV_EVENT_CLICKED, NULL);
    
    cal_point3 = lv_obj_create(calibration_screen);
    lv_obj_set_size(cal_point3, 20, 20);
    lv_obj_set_style_bg_color(cal_point3, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_radius(cal_point3, 10, 0);
    lv_obj_align(cal_point3, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_event_cb(cal_point3, calibration_point_event_handler, LV_EVENT_CLICKED, NULL);
    
    cal_point4 = lv_obj_create(calibration_screen);
    lv_obj_set_size(cal_point4, 20, 20);
    lv_obj_set_style_bg_color(cal_point4, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_radius(cal_point4, 10, 0);
    lv_obj_align(cal_point4, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_add_event_cb(cal_point4, calibration_point_event_handler, LV_EVENT_CLICKED, NULL);
    
    // Reset calibration state
    cal_point1_touched = false;
    cal_point2_touched = false;
    cal_point3_touched = false;
    cal_point4_touched = false;
    
    // Switch button
    lv_obj_t *switch_btn = lv_btn_create(calibration_screen);
    lv_obj_set_size(switch_btn, 120, 40);
    lv_obj_align(switch_btn, LV_ALIGN_BOTTOM_MID, 0, -50);
    
    lv_obj_t *btn_label = lv_label_create(switch_btn);
    lv_label_set_text(btn_label, "To Keypad");
    lv_obj_center(btn_label);
    
    // Add event handler to switch button
    lv_obj_add_event_cb(switch_btn, calibration_switch_event_handler, LV_EVENT_CLICKED, NULL);
    
    ESP_LOGI(TAG, "Calibration screen creation complete");
}

// Switch to calibration screen
void switch_to_calibration(void) {
    ESP_LOGI(TAG, "Switching to calibration screen");
    if (calibration_screen) {
        lv_screen_load(calibration_screen);
        ESP_LOGI(TAG, "Calibration screen loaded");
    } else {
        ESP_LOGW(TAG, "Calibration screen not created yet");
    }
}

// Event handler for calibration screen switch button
void calibration_switch_event_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Switching to keypad screen");
        switch_to_keypad();
    }
}

// Event handler for calibration points
void calibration_point_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    
    if (code == LV_EVENT_CLICKED) {
        // Get touch coordinates
        lv_indev_t *indev = lv_indev_get_act();
        if (indev) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            
            // Update coordinate display
            update_coordinate_display(&point);
            
            // Determine which point was touched and mark it as green
            if (obj == cal_point1) {
                ESP_LOGI(TAG, "CALIBRATION: Point 1 (top-left) touched at (%ld, %ld)", (long)point.x, (long)point.y);
                lv_obj_set_style_bg_color(cal_point1, lv_color_hex(0x00FF00), 0);
                cal_point1_touched = true;
            } else if (obj == cal_point2) {
                ESP_LOGI(TAG, "CALIBRATION: Point 2 (top-right) touched at (%ld, %ld)", (long)point.x, (long)point.y);
                lv_obj_set_style_bg_color(cal_point2, lv_color_hex(0x00FF00), 0);
                cal_point2_touched = true;
            } else if (obj == cal_point3) {
                ESP_LOGI(TAG, "CALIBRATION: Point 3 (bottom-left) touched at (%ld, %ld)", (long)point.x, (long)point.y);
                lv_obj_set_style_bg_color(cal_point3, lv_color_hex(0x00FF00), 0);
                cal_point3_touched = true;
            } else if (obj == cal_point4) {
                ESP_LOGI(TAG, "CALIBRATION: Point 4 (bottom-right) touched at (%ld, %ld)", (long)point.x, (long)point.y);
                lv_obj_set_style_bg_color(cal_point4, lv_color_hex(0x00FF00), 0);
                cal_point4_touched = true;
            }
            
            // Check if all points have been touched
            if (cal_point1_touched && cal_point2_touched && cal_point3_touched && cal_point4_touched) {
                ESP_LOGI(TAG, "CALIBRATION: All points touched! Calibration complete.");
                // Update coordinate display to show completion
                if (coord_label != NULL) {
                    lv_label_set_text(coord_label, "Calibration Complete!");
                    lv_obj_set_style_text_color(coord_label, lv_color_hex(0x00FF00), 0);
                }
            }
        }
    }
}
