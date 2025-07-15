#ifndef PASSKEY_CHANGE_H
#define PASSKEY_CHANGE_H

#include "lvgl.h"
#include "esp_log.h"

// Passkey change screen functions
void create_passkey_change_screen(void);
void switch_to_passkey_change(void);
void passkey_change_cleanup(void);

// Event handlers
void passkey_change_event_handler(lv_event_t *e);
void passkey_change_keypad_event_handler(lv_event_t *e);

// Screen access
lv_obj_t* get_passkey_change_screen(void);

#endif // PASSKEY_CHANGE_H
