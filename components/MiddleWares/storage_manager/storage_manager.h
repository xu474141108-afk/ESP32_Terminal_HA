#pragma once

#include "ha_ws_client.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif


esp_err_t storage_init(void);


esp_err_t ha_date_item_save(const char *p_buf);
esp_err_t ha_date_item_load(char *p_buf);
extern QueueHandle_t nvs_save_queue;

void all_nvs_erase(void);
void all_date_load_init();
void test_main_ui_item_save();
#ifdef __cplusplus
}
#endif