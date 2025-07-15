#include "keypad.h"
#include "../home/home.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <string.h>

static const char *TAG = "keypad";

// Keypad UI globals
lv_obj_t *keypad_screen = NULL;
lv_obj_t *passkey_display;
lv_obj_t *status_label;
char passkey_input[5] = {0}; // 4 digits + null terminator
uint8_t passkey_length = 0;
char correct_passkey[5] = {0}; // Stored passkey from NVS
bool is_passkey_set = false;

// Message box globals for first-time setup
lv_obj_t *setup_msgbox = NULL;
char setup_passkey_input[5] = {0};
uint8_t setup_passkey_length = 0;

// NVS functions for passkey storage
esp_err_t nvs_init_storage(void) {
    ESP_LOGI(TAG, "Initializing NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated and needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_LOGI(TAG, "NVS initialization %s", err == ESP_OK ? "successful" : "failed");
    return err;
}

bool nvs_passkey_exists(void) {
    nvs_handle_t my_handle;
    esp_err_t err;
    
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "NVS: Cannot open namespace for reading: %s", esp_err_to_name(err));
        return false;
    }
    
    size_t required_size = 0;
    err = nvs_get_str(my_handle, PASSKEY_KEY, NULL, &required_size);
    nvs_close(my_handle);
    
    if (err == ESP_OK && required_size > 1) {
        ESP_LOGI(TAG, "NVS: Passkey exists (size: %d)", (int)required_size);
        return true;
    } else {
        ESP_LOGI(TAG, "NVS: Passkey does not exist or is empty");
        return false;
    }
}

esp_err_t nvs_save_passkey(const char *passkey) {
    nvs_handle_t my_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "NVS: Saving passkey: %s", passkey);
    
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS: Error opening for write: %s", esp_err_to_name(err));
        return err;
    }
    
    err = nvs_set_str(my_handle, PASSKEY_KEY, passkey);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS: Error writing passkey: %s", esp_err_to_name(err));
        nvs_close(my_handle);
        return err;
    }
    
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS: Error committing: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "NVS: Passkey saved successfully");
    }
    
    nvs_close(my_handle);
    return err;
}

esp_err_t nvs_load_passkey(char *passkey, size_t max_len) {
    nvs_handle_t my_handle;
    esp_err_t err;
    
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS: Error opening for read: %s", esp_err_to_name(err));
        return err;
    }
    
    size_t required_size = max_len;
    err = nvs_get_str(my_handle, PASSKEY_KEY, passkey, &required_size);
    
    nvs_close(my_handle);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "NVS: Passkey loaded: %s", passkey);
    } else {
        ESP_LOGE(TAG, "NVS: Error loading passkey: %s", esp_err_to_name(err));
    }
    
    return err;
}

// Initialize NVS and load existing passkey
esp_err_t keypad_init_nvs(void) {
    esp_err_t err = nvs_init_storage();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
        is_passkey_set = false;
        return err;
    }
    
    // Check if passkey exists in NVS
    if (nvs_passkey_exists()) {
        // Load existing passkey
        err = nvs_load_passkey(correct_passkey, sizeof(correct_passkey));
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Loaded existing passkey from NVS");
            is_passkey_set = true;
        } else {
            ESP_LOGE(TAG, "Failed to load passkey from NVS: %s", esp_err_to_name(err));
            is_passkey_set = false;
        }
    } else {
        ESP_LOGI(TAG, "No passkey found in NVS - first time setup required");
        is_passkey_set = false;
    }
    
    return ESP_OK;
}

// Message box functions for first-time setup
void show_setup_msgbox(void) {
    ESP_LOGI(TAG, "Showing setup message box");
    
    // Create a simple container instead of msgbox for better compatibility
    setup_msgbox = lv_obj_create(lv_screen_active());
    lv_obj_set_size(setup_msgbox, 200, 200);
    lv_obj_center(setup_msgbox);
    lv_obj_set_style_bg_color(setup_msgbox, lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_color(setup_msgbox, lv_color_hex(0x555555), 0);
    lv_obj_set_style_border_width(setup_msgbox, 2, 0);
    lv_obj_set_style_radius(setup_msgbox, 10, 0);
    
    // Title
    lv_obj_t *title = lv_label_create(setup_msgbox);
    lv_label_set_text(title, "First Time Setup");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Instructions and passkey display
    lv_obj_t *text_label = lv_label_create(setup_msgbox);
    lv_label_set_text(text_label, "Set your 4-digit security code:\n____");
    lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(text_label, LV_ALIGN_TOP_MID, 0, 40);
    
    // Store reference to text label for updates
    lv_obj_set_user_data(setup_msgbox, text_label);
    
    // Create setup keypad
    static const char *setup_keypad_map[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n", 
        "7", "8", "9", "\n",
        "", "0", "", ""
    };
    
    lv_obj_t *setup_keypad = lv_buttonmatrix_create(setup_msgbox);
    lv_buttonmatrix_set_map(setup_keypad, setup_keypad_map);
    lv_obj_set_size(setup_keypad, 180, 140);
    lv_obj_align(setup_keypad, LV_ALIGN_CENTER, 0, -10);
    
    // Add event handler for keypad
    lv_obj_add_event_cb(setup_keypad, setup_keypad_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Create Clear button
    lv_obj_t *clear_btn = lv_btn_create(setup_msgbox);
    lv_obj_set_size(clear_btn, 80, 35);
    lv_obj_align(clear_btn, LV_ALIGN_BOTTOM_LEFT, 20, -15);
    lv_obj_t *clear_label = lv_label_create(clear_btn);
    lv_label_set_text(clear_label, "Clear");
    lv_obj_center(clear_label);
    lv_obj_set_user_data(clear_btn, (void*)0); // Button ID 0
    lv_obj_add_event_cb(clear_btn, setup_msgbox_event_handler, LV_EVENT_CLICKED, NULL);
    
    // Create Save button
    lv_obj_t *save_btn = lv_btn_create(setup_msgbox);
    lv_obj_set_size(save_btn, 80, 35);
    lv_obj_align(save_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -15);
    lv_obj_t *save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, "Save");
    lv_obj_center(save_label);
    lv_obj_set_user_data(save_btn, (void*)1); // Button ID 1
    lv_obj_add_event_cb(save_btn, setup_msgbox_event_handler, LV_EVENT_CLICKED, NULL);
    
    // Reset setup state
    setup_passkey_length = 0;
    memset(setup_passkey_input, 0, sizeof(setup_passkey_input));
    
    ESP_LOGI(TAG, "Setup message box created");
}

void setup_msgbox_event_handler(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        // Get button ID from user data
        uintptr_t btn_id = (uintptr_t)lv_obj_get_user_data(obj);
        const char *btn_text = (btn_id == 0) ? "Clear" : "Save";
        
        ESP_LOGI(TAG, "Setup msgbox button pressed: %s", btn_text);
        
        if (strcmp(btn_text, "Clear") == 0) {
            // Clear the setup passkey
            ESP_LOGI(TAG, "Setup: Clearing passkey");
            setup_passkey_length = 0;
            memset(setup_passkey_input, 0, sizeof(setup_passkey_input));
            
            // Update text label (stored in user data of setup_msgbox)
            lv_obj_t *text_label = (lv_obj_t*)lv_obj_get_user_data(setup_msgbox);
            if (text_label) {
                lv_label_set_text(text_label, "Set your 4-digit security code:\n____");
            }
        }
        else if (strcmp(btn_text, "Save") == 0) {
            // Save the passkey
            ESP_LOGI(TAG, "Setup: Save pressed, passkey length: %d", setup_passkey_length);
            if (setup_passkey_length == 4) {
                ESP_LOGI(TAG, "Setup: Saving passkey: '%s'", setup_passkey_input);
                esp_err_t err = nvs_save_passkey(setup_passkey_input);
                if (err == ESP_OK) {
                    // Copy to runtime variable
                    strcpy(correct_passkey, setup_passkey_input);
                    is_passkey_set = true;
                    ESP_LOGI(TAG, "Setup: Passkey saved successfully!");
                    
                    // Close message box
                    lv_obj_del(setup_msgbox);
                    setup_msgbox = NULL;
                    
                    // Update status
                    lv_label_set_text(status_label, "Passkey set! Enter code:");
                } else {
                    ESP_LOGE(TAG, "Setup: Failed to save passkey");
                    lv_obj_t *text_label = (lv_obj_t*)lv_obj_get_user_data(setup_msgbox);
                    if (text_label) {
                        lv_label_set_text(text_label, "Save failed! Try again\n____");
                    }
                }
            } else {
                ESP_LOGI(TAG, "Setup: Not enough digits entered");
                lv_obj_t *text_label = (lv_obj_t*)lv_obj_get_user_data(setup_msgbox);
                if (text_label) {
                    lv_label_set_text(text_label, "Enter all 4 digits!\n____");
                }
            }
        }
    }
}

void setup_keypad_event_handler(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        uint32_t btn_id = lv_buttonmatrix_get_selected_button(obj);
        if (btn_id == LV_BUTTONMATRIX_BUTTON_NONE) return;
        
        const char *btn_text = lv_buttonmatrix_get_button_text(obj, btn_id);
        if (btn_text == NULL || strlen(btn_text) == 0) return;
        
        ESP_LOGI(TAG, "Setup keypad: '%s' pressed", btn_text);
        
        if (strlen(btn_text) == 1 && btn_text[0] >= '0' && btn_text[0] <= '9') {
            // Number button pressed
            if (setup_passkey_length < 4) {
                setup_passkey_input[setup_passkey_length] = btn_text[0];
                setup_passkey_length++;
                
                // Update display (text label is stored in user data of setup_msgbox)
                lv_obj_t *text_label = (lv_obj_t*)lv_obj_get_user_data(setup_msgbox);
                if (text_label) {
                    char display_text[50];
                    char stars[5] = "____";
                    for (int i = 0; i < setup_passkey_length; i++) {
                        stars[i] = '*';
                    }
                    snprintf(display_text, sizeof(display_text), "Set your 4-digit security code:\n%s", stars);
                    lv_label_set_text(text_label, display_text);
                }
                
                ESP_LOGI(TAG, "Setup: Updated display, length: %d", setup_passkey_length);
            }
        }
    }
}

// Keypad button event handler
void keypad_event_handler(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    ESP_LOGI(TAG, "KEYPAD EVENT: Code %d on button matrix %p", code, obj);
    
    // Check if passkey is set, if not show setup dialog
    if (!is_passkey_set && setup_msgbox == NULL) {
        ESP_LOGI(TAG, "No passkey set, showing setup message box");
        show_setup_msgbox();
        return;
    }
    
    // Handle only VALUE_CHANGED events for button matrix (this is the primary event for button selection)
    if (code == LV_EVENT_VALUE_CHANGED) {
        uint32_t btn_id = lv_buttonmatrix_get_selected_button(obj);
        if (btn_id == LV_BUTTONMATRIX_BUTTON_NONE) {
            ESP_LOGW(TAG, "KEYPAD: No button selected");
            return;
        }
        
        const char *btn_text = lv_buttonmatrix_get_button_text(obj, btn_id);
        if (btn_text == NULL) {
            ESP_LOGW(TAG, "KEYPAD: Button text is NULL for button %u", (unsigned int)btn_id);
            return;
        }
        
        ESP_LOGI(TAG, "KEYPAD: Button %u pressed: '%s'", (unsigned int)btn_id, btn_text);
        
        if (strcmp(btn_text, "CLR") == 0) {
            // Clear passkey
            ESP_LOGI(TAG, "KEYPAD: Clearing passkey");
            passkey_length = 0;
            memset(passkey_input, 0, sizeof(passkey_input));
            lv_label_set_text(passkey_display, "____");
            lv_label_set_text(status_label, "Enter 4-digit passkey");
        }
        else if (strcmp(btn_text, "OK") == 0) {
            // Check passkey
            ESP_LOGI(TAG, "KEYPAD: OK pressed, passkey length: %d", passkey_length);
            if (passkey_length == 4) {
                ESP_LOGI(TAG, "KEYPAD: Checking passkey: '%s' vs '%s'", passkey_input, correct_passkey);
                if (strcmp(passkey_input, correct_passkey) == 0) {
                    ESP_LOGI(TAG, "KEYPAD: Access granted!");
                    lv_label_set_text(status_label, "Access Granted!");
                    
                    // Clear input and switch to home screen after brief delay
                    passkey_length = 0;
                    memset(passkey_input, 0, sizeof(passkey_input));
                    lv_label_set_text(passkey_display, "____");
                    
                    // Switch to home screen
                    ESP_LOGI(TAG, "KEYPAD: Switching to home screen");
                    switch_to_home();
                } else {
                    ESP_LOGI(TAG, "KEYPAD: Access denied!");
                    lv_label_set_text(status_label, "Access Denied");
                    // Auto-clear after wrong entry
                    passkey_length = 0;
                    memset(passkey_input, 0, sizeof(passkey_input));
                    lv_label_set_text(passkey_display, "____");
                }
            } else {
                ESP_LOGI(TAG, "KEYPAD: Not enough digits entered");
                lv_label_set_text(status_label, "Enter all 4 digits");
            }
        }
        else if (strlen(btn_text) == 1 && btn_text[0] >= '0' && btn_text[0] <= '9') {
            // Number button pressed
            ESP_LOGI(TAG, "KEYPAD: Number '%c' pressed, current length: %d", btn_text[0], passkey_length);
            if (passkey_length < 4) {
                passkey_input[passkey_length] = btn_text[0];
                passkey_length++;
                
                // Update display with stars
                char display_text[5] = "____";
                for (int i = 0; i < passkey_length; i++) {
                    display_text[i] = '*';
                }
                lv_label_set_text(passkey_display, display_text);
                ESP_LOGI(TAG, "KEYPAD: Updated display to: %s", display_text);
                
                if (passkey_length == 4) {
                    lv_label_set_text(status_label, "Press OK to confirm");
                }
            }
        } else {
            ESP_LOGW(TAG, "KEYPAD: Unhandled button text: '%s'", btn_text);
        }
    }
}

// Create separate keypad screen
void create_keypad_screen(void) {
    ESP_LOGI(TAG, "Creating keypad screen");
    
    // Create a new screen for the keypad
    keypad_screen = lv_obj_create(NULL);
    
    ESP_LOGI(TAG, "Creating keypad UI on dedicated screen %p", keypad_screen);
    
    // Title
    lv_obj_t *title = lv_label_create(keypad_screen);
    lv_label_set_text(title, "Security Keypad");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    ESP_LOGI(TAG, "Created title label %p", title);
    
    // Passkey display
    passkey_display = lv_label_create(keypad_screen);
    lv_label_set_text(passkey_display, "____");
    lv_obj_set_style_text_color(passkey_display, lv_color_hex(0x00FF00), 0);
    lv_obj_align(passkey_display, LV_ALIGN_TOP_MID, 0, 50);
    ESP_LOGI(TAG, "Created passkey display %p", passkey_display);
    
    // Status label
    status_label = lv_label_create(keypad_screen);
    lv_label_set_text(status_label, is_passkey_set ? "Enter 4-digit code" : "Touch to setup passkey");
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 85);
    ESP_LOGI(TAG, "Created status label %p", status_label);
    
    // Create keypad using button matrix
    static const char *keypad_map[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n", 
        "7", "8", "9", "\n",
        "CLR", "0", "OK", ""
    };
    
    lv_obj_t *keypad = lv_buttonmatrix_create(keypad_screen);
    lv_buttonmatrix_set_map(keypad, keypad_map);
    lv_obj_set_size(keypad, 200, 160);
    lv_obj_align(keypad, LV_ALIGN_CENTER, 0, 40);
    
    // Make CLR and OK buttons wider
    lv_buttonmatrix_set_button_width(keypad, 9, 2);   // CLR button (index 9)
    lv_buttonmatrix_set_button_width(keypad, 11, 2);  // OK button (index 11)
    
    // Add event handlers - only VALUE_CHANGED for button matrix functionality
    lv_obj_add_event_cb(keypad, keypad_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    ESP_LOGI(TAG, "Created button matrix keypad %p", keypad);
    ESP_LOGI(TAG, "Keypad screen creation complete");
}

// Switch to keypad screen
void switch_to_keypad(void) {
    ESP_LOGI(TAG, "Switching to keypad screen");
    if (keypad_screen) {
        keypad_reset_state(); // Reset keypad state when switching back
        lv_screen_load(keypad_screen);
        ESP_LOGI(TAG, "Keypad screen loaded");
    } else {
        ESP_LOGW(TAG, "Keypad screen not created yet");
    }
}

// Reset keypad state (called when returning from home screen)
void keypad_reset_state(void) {
    ESP_LOGI(TAG, "Resetting keypad state");
    
    // Clear input
    passkey_length = 0;
    memset(passkey_input, 0, sizeof(passkey_input));
    
    // Reset display
    if (passkey_display) {
        lv_label_set_text(passkey_display, "____");
    }
    
    // Reset status message
    if (status_label) {
        lv_label_set_text(status_label, is_passkey_set ? "Enter 4-digit code" : "Touch to setup passkey");
    }
}

// Passkey validation function
bool validate_passkey(const char* input) {
    if (!input || strlen(input) != 4) {
        return false;
    }
    
    if (is_passkey_set) {
        return strcmp(input, correct_passkey) == 0;
    }
    
    return false;
}

// Update passkey function
void update_passkey(const char* new_passkey) {
    if (!new_passkey || strlen(new_passkey) != 4) {
        ESP_LOGE(TAG, "Invalid passkey format");
        return;
    }
    
    // Update the current passkey
    strncpy(correct_passkey, new_passkey, sizeof(correct_passkey) - 1);
    correct_passkey[4] = '\0';
    
    // Save to NVS
    esp_err_t err = nvs_save_passkey(new_passkey);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Passkey updated successfully");
        is_passkey_set = true;
    } else {
        ESP_LOGE(TAG, "Failed to save new passkey to NVS: %s", esp_err_to_name(err));
    }
}
