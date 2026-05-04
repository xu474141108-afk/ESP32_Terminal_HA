#include "storage_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>
#include "common_types.h"

#define TAG "STORAGE_MGR"

esp_err_t storage_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}


void Write_devices_to_nvs(){
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("ha_store", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "无法打开 NVS 句柄");
        return;
    }
}

void save_devices_to_nvs(void) {
    nvs_handle_t my_handle;
   esp_err_t err = nvs_open("ha_store", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "无法打开 NVS 句柄");
        return;
    }

    nvs_set_i32(my_handle, "dev_count", g_device_count);
    nvs_set_blob(my_handle, "device_list", g_device_list, sizeof(g_device_list));
    
    nvs_commit(my_handle);
    nvs_close(my_handle);
    ESP_LOGI(TAG, "NVS 保存成功");
} 

void load_devices_from_nvs(void) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("ha_store", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "未找到已保存的 NVS 数据，跳过读取。");
        return;
    }

    int32_t count = 0;
    nvs_get_i32(my_handle, "dev_count", &count);
    g_device_count = (int)count;

    size_t required_size = sizeof(g_device_list);
    nvs_get_blob(my_handle, "device_list", g_device_list, &required_size);

    nvs_close(my_handle);
    ESP_LOGI(TAG, "NVS 读取成功，设备数: %d", g_device_count);
}