#include "passkey_change.h"
#include "../home/home.h"
#include "../keypad/keypad.h"
#include <string.h>

static const char *TAG = "passkey_change";

// Passkey change screen globals
lv_obj_t *passkey_change_screen = NULL;
lv_obj_t *current_passkey_area = NULL;
lv_obj_t *new_passkey_area = NULL;
lv_obj_t *confirm_passkey_area = NULL;
lv_obj_t *passkey_status_label = NULL;
lv_obj_t *current_display_label = NULL;
lv_obj_t *new_display_label = NULL;
lv_obj_t *confirm_display_label = NULL;

// Input state
typedef enum {
    INPUT_CURRENT_PASSKEY,
    INPUT_NEW_PASSKEY,
    INPUT_CONFIRM_PASSKEY
} input_state_t;

static input_state_t current_input_state = INPUT_CURRENT_PASSKEY;
static char current_passkey_input[5] = {0};
static char new_passkey_input[5] = {0};
static char confirm_passkey_input[5] = {0};
static int current_passkey_index = 0;
static int new_passkey_index = 0;
static int confirm_passkey_index = 0;

// External passkey validation function (from keypad.c)
extern bool validate_passkey(const char* input);
extern void update_passkey(const char* new_passkey);

// Create virtual keypad for passkey change screen
static void create_virtual_keypad(lv_obj_t *parent, lv_event_cb_t event_cb) {
    // Create keypad using button matrix
    static const char *keypad_map[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n", 
        "7", "8", "9", "\n",
        "Clear", "0", "Enter", ""
    };
    
    lv_obj_t *keypad = lv_buttonmatrix_create(parent);
    lv_buttonmatrix_set_map(keypad, keypad_map);
    lv_obj_set_size(keypad, 200, 160);
    lv_obj_align(keypad, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    // Make Clear and Enter buttons wider
    lv_buttonmatrix_set_button_width(keypad, 9, 2);   // Clear button (index 9)
    lv_buttonmatrix_set_button_width(keypad, 11, 2);  // Enter button (index 11)
    
    // Add event handler
    lv_obj_add_event_cb(keypad, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

// Create the passkey change screen
void create_passkey_change_screen(void) {
    ESP_LOGI(TAG, "Creating passkey change screen");
    
    // Create a new screen
    passkey_change_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(passkey_change_screen, lv_color_hex(0x1C1C1C), 0);
    
    // Title
    lv_obj_t *title = lv_label_create(passkey_change_screen);
    lv_label_set_text(title, "Change Passkey");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Status label
    passkey_status_label = lv_label_create(passkey_change_screen);
    lv_label_set_text(passkey_status_label, "Enter current passkey:");
    lv_obj_set_style_text_color(passkey_status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(passkey_status_label, LV_ALIGN_TOP_MID, 0, 50);
    
    // Current passkey input area
    current_passkey_area = lv_obj_create(passkey_change_screen);
    lv_obj_set_size(current_passkey_area, 200, 50);
    lv_obj_align(current_passkey_area, LV_ALIGN_CENTER, 0, -60);
    lv_obj_set_style_bg_color(current_passkey_area, lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_color(current_passkey_area, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(current_passkey_area, 2, 0);
    
    current_display_label = lv_label_create(current_passkey_area);
    lv_label_set_text(current_display_label, "____");
    lv_obj_set_style_text_color(current_display_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(current_display_label);
    
    // New passkey input area
    new_passkey_area = lv_obj_create(passkey_change_screen);
    lv_obj_set_size(new_passkey_area, 200, 50);
    lv_obj_align(new_passkey_area, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(new_passkey_area, lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_color(new_passkey_area, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(new_passkey_area, 1, 0);
    lv_obj_add_flag(new_passkey_area, LV_OBJ_FLAG_HIDDEN);
    
    new_display_label = lv_label_create(new_passkey_area);
    lv_label_set_text(new_display_label, "____");
    lv_obj_set_style_text_color(new_display_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(new_display_label);
    
    // Confirm passkey input area
    confirm_passkey_area = lv_obj_create(passkey_change_screen);
    lv_obj_set_size(confirm_passkey_area, 200, 50);
    lv_obj_align(confirm_passkey_area, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_bg_color(confirm_passkey_area, lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_color(confirm_passkey_area, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(confirm_passkey_area, 1, 0);
    lv_obj_add_flag(confirm_passkey_area, LV_OBJ_FLAG_HIDDEN);
    
    confirm_display_label = lv_label_create(confirm_passkey_area);
    lv_label_set_text(confirm_display_label, "____");
    lv_obj_set_style_text_color(confirm_display_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(confirm_display_label);
    
    // Create virtual keypad
    create_virtual_keypad(passkey_change_screen, passkey_change_keypad_event_handler);
    
    // Back button
    lv_obj_t *back_btn = lv_btn_create(passkey_change_screen);
    lv_obj_set_size(back_btn, 80, 30);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x666666), 0);
    lv_obj_add_event_cb(back_btn, passkey_change_event_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);
    
    ESP_LOGI(TAG, "Passkey change screen created successfully");
}

// Update display for current input
static void update_display(void) {
    char display[5] = "____";
    
    switch(current_input_state) {
        case INPUT_CURRENT_PASSKEY:
            for(int i = 0; i < current_passkey_index && i < 4; i++) {
                display[i] = '*';
            }
            lv_label_set_text(current_display_label, display);
            break;
            
        case INPUT_NEW_PASSKEY:
            for(int i = 0; i < new_passkey_index && i < 4; i++) {
                display[i] = '*';
            }
            lv_label_set_text(new_display_label, display);
            break;
            
        case INPUT_CONFIRM_PASSKEY:
            for(int i = 0; i < confirm_passkey_index && i < 4; i++) {
                display[i] = '*';
            }
            lv_label_set_text(confirm_display_label, display);
            break;
    }
}

// Keypad event handler
void passkey_change_keypad_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        uint32_t btn_id = lv_buttonmatrix_get_selected_button(obj);
        const char *btn_text = lv_buttonmatrix_get_button_text(obj, btn_id);
        
        if (!btn_text) return;
        
        if (strcmp(btn_text, "Clear") == 0) {
            // Clear current input
            switch(current_input_state) {
                case INPUT_CURRENT_PASSKEY:
                    memset(current_passkey_input, 0, sizeof(current_passkey_input));
                    current_passkey_index = 0;
                    break;
                case INPUT_NEW_PASSKEY:
                    memset(new_passkey_input, 0, sizeof(new_passkey_input));
                    new_passkey_index = 0;
                    break;
                case INPUT_CONFIRM_PASSKEY:
                    memset(confirm_passkey_input, 0, sizeof(confirm_passkey_input));
                    confirm_passkey_index = 0;
                    break;
            }
            update_display();
        }
        else if (strcmp(btn_text, "Enter") == 0) {
            // Process current input
            switch(current_input_state) {
                case INPUT_CURRENT_PASSKEY:
                    if (current_passkey_index == 4) {
                        if (validate_passkey(current_passkey_input)) {
                            // Current passkey is correct, move to new passkey input
                            current_input_state = INPUT_NEW_PASSKEY;
                            lv_label_set_text(passkey_status_label, "Enter new passkey:");
                            lv_obj_set_style_border_color(current_passkey_area, lv_color_hex(0x666666), 0);
                            lv_obj_set_style_border_color(new_passkey_area, lv_color_hex(0x00FF00), 0);
                            lv_obj_clear_flag(new_passkey_area, LV_OBJ_FLAG_HIDDEN);
                        } else {
                            lv_label_set_text(passkey_status_label, "Incorrect passkey! Try again:");
                            lv_obj_set_style_text_color(passkey_status_label, lv_color_hex(0xFF0000), 0);
                            memset(current_passkey_input, 0, sizeof(current_passkey_input));
                            current_passkey_index = 0;
                            update_display();
                        }
                    }
                    break;
                    
                case INPUT_NEW_PASSKEY:
                    if (new_passkey_index == 4) {
                        // Move to confirm passkey input
                        current_input_state = INPUT_CONFIRM_PASSKEY;
                        lv_label_set_text(passkey_status_label, "Confirm new passkey:");
                        lv_obj_set_style_text_color(passkey_status_label, lv_color_hex(0xFFFFFF), 0);
                        lv_obj_set_style_border_color(new_passkey_area, lv_color_hex(0x666666), 0);
                        lv_obj_set_style_border_color(confirm_passkey_area, lv_color_hex(0x00FF00), 0);
                        lv_obj_clear_flag(confirm_passkey_area, LV_OBJ_FLAG_HIDDEN);
                    }
                    break;
                    
                case INPUT_CONFIRM_PASSKEY:
                    if (confirm_passkey_index == 4) {
                        if (strcmp(new_passkey_input, confirm_passkey_input) == 0) {
                            // Passkeys match, update the passkey
                            update_passkey(new_passkey_input);
                            lv_label_set_text(passkey_status_label, "Passkey changed successfully!");
                            lv_obj_set_style_text_color(passkey_status_label, lv_color_hex(0x00FF00), 0);
                            ESP_LOGI(TAG, "Passkey changed successfully");
                            
                            // Auto-return to home after 2 seconds
                            // For now, just log the success
                        } else {
                            lv_label_set_text(passkey_status_label, "Passkeys don't match! Try again:");
                            lv_obj_set_style_text_color(passkey_status_label, lv_color_hex(0xFF0000), 0);
                            current_input_state = INPUT_NEW_PASSKEY;
                            memset(new_passkey_input, 0, sizeof(new_passkey_input));
                            memset(confirm_passkey_input, 0, sizeof(confirm_passkey_input));
                            new_passkey_index = 0;
                            confirm_passkey_index = 0;
                            lv_obj_set_style_border_color(new_passkey_area, lv_color_hex(0x00FF00), 0);
                            lv_obj_set_style_border_color(confirm_passkey_area, lv_color_hex(0x666666), 0);
                            lv_obj_add_flag(confirm_passkey_area, LV_OBJ_FLAG_HIDDEN);
                            update_display();
                        }
                    }
                    break;
            }
        }
        else if (strlen(btn_text) == 1 && btn_text[0] >= '0' && btn_text[0] <= '9') {
            // Number input
            switch(current_input_state) {
                case INPUT_CURRENT_PASSKEY:
                    if (current_passkey_index < 4) {
                        current_passkey_input[current_passkey_index] = btn_text[0];
                        current_passkey_index++;
                        update_display();
                    }
                    break;
                case INPUT_NEW_PASSKEY:
                    if (new_passkey_index < 4) {
                        new_passkey_input[new_passkey_index] = btn_text[0];
                        new_passkey_index++;
                        update_display();
                    }
                    break;
                case INPUT_CONFIRM_PASSKEY:
                    if (confirm_passkey_index < 4) {
                        confirm_passkey_input[confirm_passkey_index] = btn_text[0];
                        confirm_passkey_index++;
                        update_display();
                    }
                    break;
            }
        }
    }
}

// General event handler for back button
void passkey_change_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Back button clicked, returning to home");
        switch_to_home();
    }
}

// Switch to passkey change screen
void switch_to_passkey_change(void) {
    ESP_LOGI(TAG, "Switching to passkey change screen");
    if (!passkey_change_screen) {
        create_passkey_change_screen();
    }
    
    // Reset state
    current_input_state = INPUT_CURRENT_PASSKEY;
    memset(current_passkey_input, 0, sizeof(current_passkey_input));
    memset(new_passkey_input, 0, sizeof(new_passkey_input));
    memset(confirm_passkey_input, 0, sizeof(confirm_passkey_input));
    current_passkey_index = 0;
    new_passkey_index = 0;
    confirm_passkey_index = 0;
    
    // Reset UI
    lv_label_set_text(passkey_status_label, "Enter current passkey:");
    lv_obj_set_style_text_color(passkey_status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(current_passkey_area, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_color(new_passkey_area, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_color(confirm_passkey_area, lv_color_hex(0x666666), 0);
    lv_obj_add_flag(new_passkey_area, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(confirm_passkey_area, LV_OBJ_FLAG_HIDDEN);
    update_display();
    
    lv_screen_load(passkey_change_screen);
}

// Cleanup function
void passkey_change_cleanup(void) {
    if (passkey_change_screen) {
        lv_obj_del(passkey_change_screen);
        passkey_change_screen = NULL;
    }
}

// Get screen object
lv_obj_t* get_passkey_change_screen(void) {
    return passkey_change_screen;
}
