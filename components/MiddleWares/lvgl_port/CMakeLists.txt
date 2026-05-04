idf_component_register(SRCS "lvgl_port.c"
                       INCLUDE_DIRS "."
                       REQUIRES lvgl esp_lcd_touch lvgl_ui
                       PRIV_REQUIRES esp_timer freertos common_types)

