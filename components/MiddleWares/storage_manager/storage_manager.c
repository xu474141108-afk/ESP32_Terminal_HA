#include "storage_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>
#include "ha_http_req.h"
#include "custom.h"


#define TAG "STORAGE_MGR"
#define NVS_STORAGE_NAMESPACE "ui_items"

esp_err_t storage_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

static const char* get_nvs_key_by_pointer(const ha_entity_t *p_slot)
{
    if (p_slot == &g_ui_main_data.cont_rt) return "item_rt";
    if (p_slot == &g_ui_main_data.cont_rm) return "item_rm";
    if (p_slot == &g_ui_main_data.cont_rd) return "item_rd";
    if (p_slot == &g_ui_main_data.cont_md) return "item_md";
    return NULL;
}

esp_err_t save_ui_sub_item_to_nvs(const ha_entity_t *p_slot) 
{
    if (p_slot == NULL) return ESP_ERR_INVALID_ARG;

    // 1. 自动寻找 Key
    const char *nvs_key = get_nvs_key_by_pointer(p_slot);
    if (nvs_key == NULL) {
        ESP_LOGE(TAG, "错误：传入的指针不属于 g_ui_main_data 的任何有效成员！");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // 2. 写入数据
    err = nvs_set_blob(my_handle, nvs_key, p_slot, sizeof(ha_entity_t));
    if (err == ESP_OK) {
        err = nvs_commit(my_handle);
        ESP_LOGI(TAG, "成功保存数据到 NVS 键: %s", nvs_key);
    }

    nvs_close(my_handle);
    return err;
}

/**
 * @brief 通过传入的子项指针，自动从 NVS 恢复对应数据
 */
esp_err_t load_ui_sub_item_from_nvs(ha_entity_t *p_slot)
{
    if (p_slot == NULL) return ESP_ERR_INVALID_ARG;

    const char *nvs_key = get_nvs_key_by_pointer(p_slot);
    if (nvs_key == NULL) return ESP_ERR_INVALID_ARG;

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    size_t required_size = 0;
    err = nvs_get_blob(my_handle, nvs_key, NULL, &required_size);
    
    if (err == ESP_OK && required_size == sizeof(ha_entity_t)) {
        err = nvs_get_blob(my_handle, nvs_key, p_slot, &required_size);
        ESP_LOGI(TAG, "成功从 NVS 键 %s 恢复数据", nvs_key);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "NVS 中未找到键 %s 的数据，保持默认值", nvs_key);
    } else {
        err = ESP_ERR_NVS_INVALID_LENGTH;
    }

    nvs_close(my_handle);
    return err;
}
