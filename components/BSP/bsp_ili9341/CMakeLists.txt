idf_component_register(SRCS "src/bsp_ili9341.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "esp_lcd" "driver"
                       PRIV_REQUIRES "esp_hw_support" "esp_timer" "freertos" "log")

