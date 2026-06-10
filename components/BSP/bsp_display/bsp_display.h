#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void bsp_display_init(void);
void bsp_lcd_test_clear_screen(uint16_t color_rgb565);

#ifdef __cplusplus
}
#endif