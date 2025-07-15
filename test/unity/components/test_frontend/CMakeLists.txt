idf_component_register(SRCS "test_keypad.c" "test_calibration.c"
                    INCLUDE_DIRS "."
                    REQUIRES unity driver nvs_flash esp_timer
                    )
