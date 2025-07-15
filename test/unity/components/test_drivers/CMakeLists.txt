idf_component_register(SRCS "test_lcd.c" "test_touch.c"
                    INCLUDE_DIRS "."
                    REQUIRES unity driver nvs_flash esp_timer
                    PRIV_REQUIRES esp_lcd
                    )
