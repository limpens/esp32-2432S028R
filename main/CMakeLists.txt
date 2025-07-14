idf_component_register(SRCS demo.c 
                           drivers/lcd.c 
                           drivers/touch.c
                           frontend/keypad/keypad.c
                           frontend/calibration/calibration.c
                           frontend/home/home.c
                    INCLUDE_DIRS "."
                    REQUIRES esp_lcd driver nvs_flash esp_timer
                    )
