#pragma once

#include "esp_lcd_touch.h"
#include "gui_guider.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t h_res;
    uint16_t v_res;
    spi_host_device_t lcd_host; 
    esp_lcd_panel_handle_t panel_handle;
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_touch_handle_t tp_handle;
    bool flush_done; 
} lvgl_panel_t;

void lvgl_port_init(lvgl_panel_t *panel);
#ifdef __cplusplus
}
#endif