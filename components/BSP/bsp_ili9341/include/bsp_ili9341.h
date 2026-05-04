#pragma once

#include "esp_err.h"
#include "esp_lcd_types.h" // 必须包含这个，才能识别 esp_lcd_panel_handle_t

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t esp_lcd_new_panel_ili9341(const esp_lcd_panel_io_handle_t io,
                                    const esp_lcd_panel_dev_config_t *panel_dev_config,
                                    esp_lcd_panel_handle_t *ret_panel);

#ifdef __cplusplus
}
#endif