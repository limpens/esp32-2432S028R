#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <lvgl.h>
#include <esp_log.h>

// Calibration globals
extern lv_obj_t *calibration_screen;
extern lv_obj_t *cal_point1;
extern lv_obj_t *cal_point2;
extern lv_obj_t *cal_point3;
extern lv_obj_t *cal_point4;
extern lv_obj_t *coord_label;
extern bool cal_point1_touched;
extern bool cal_point2_touched;
extern bool cal_point3_touched;
extern bool cal_point4_touched;

// Function declarations
void create_calibration_screen(void);
void switch_to_calibration(void);
void calibration_switch_event_handler(lv_event_t *e);
void calibration_point_event_handler(lv_event_t *e);

#endif // CALIBRATION_H
