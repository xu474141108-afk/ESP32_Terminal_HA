idf_component_register(SRCS "bsp_xpt2046.c"
                       INCLUDE_DIRS "."
                       REQUIRES "espressif__esp_lcd_touch" "driver"
                       PRIV_REQUIRES "esp_hw_support" "esp_timer" "freertos" "log")

