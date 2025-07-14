#ifndef KEYPAD_H
#define KEYPAD_H

#include <lvgl.h>
#include <esp_log.h>
#include <esp_err.h>

// Keypad globals
extern lv_obj_t *keypad_screen;
extern lv_obj_t *passkey_display;
extern lv_obj_t *status_label;
extern char passkey_input[5];
extern uint8_t passkey_length;
extern char correct_passkey[5];
extern bool is_passkey_set;

// Message box globals for first-time setup
extern lv_obj_t *setup_msgbox;
extern char setup_passkey_input[5];
extern uint8_t setup_passkey_length;

// NVS configuration
#define STORAGE_NAMESPACE "security"
#define PASSKEY_KEY "passkey"

// Function declarations
void create_keypad_screen(void);
void switch_to_keypad(void);
void keypad_event_handler(lv_event_t *e);

// NVS functions
esp_err_t nvs_save_passkey(const char *passkey);
esp_err_t nvs_load_passkey(char *passkey, size_t max_len);
esp_err_t nvs_init_storage(void);
bool nvs_passkey_exists(void);
esp_err_t keypad_init_nvs(void);

// Message box functions
void show_setup_msgbox(void);
void setup_msgbox_event_handler(lv_event_t *e);
void setup_keypad_event_handler(lv_event_t *e);

// Keypad state management
void keypad_reset_state(void);

#endif // KEYPAD_H
