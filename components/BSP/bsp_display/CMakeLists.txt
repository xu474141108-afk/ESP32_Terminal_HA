idf_component_register(SRCS "bsp_display.c"
                       INCLUDE_DIRS "."
                       REQUIRES "esp_lcd" "driver" "bsp_ili9488" "lvgl" "bsp_xpt2046" "bsp_ili9341" 
                       PRIV_REQUIRES "esp_hw_support" "esp_timer" "freertos" "log" "esp_lcd_touch" "lvgl_port")

