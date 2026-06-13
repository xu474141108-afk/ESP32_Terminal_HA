#pragma once

#include "ha_http_req.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_ITEM_RT,
    UI_ITEM_RM,
    UI_ITEM_RD,
    UI_ITEM_MD
} ui_item_index_t;

esp_err_t storage_init(void);
esp_err_t save_ui_sub_item_to_nvs(const ha_entity_t *p_slot);
esp_err_t load_ui_sub_item_from_nvs(ha_entity_t *p_slot);

#ifdef __cplusplus
}
#endif