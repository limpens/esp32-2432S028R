#ifndef HOME_H
#define HOME_H

#include "lvgl.h"
#include "esp_log.h"
#include "esp_timer.h"

// Home screen functions
void create_home_screen(void);
void switch_to_home(void);
void home_start_idle_timer(void);
void home_stop_idle_timer(void);
void home_reset_idle_timer(void);

// Event handlers
void home_idle_timeout_callback(void *arg);
void home_activity_event_handler(lv_event_t *e);

// Screen access
lv_obj_t* get_home_screen(void);

#endif // HOME_H
