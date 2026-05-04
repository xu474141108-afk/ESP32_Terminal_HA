#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void bsp_display_init(void);
void fill_screen(esp_lcd_panel_handle_t panel_handle,int x,int y, uint16_t color);
#ifdef __cplusplus
}
#endif