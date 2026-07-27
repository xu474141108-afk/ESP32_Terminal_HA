#include "storage_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>
#include "custom.h"
#include "ha_ws_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#define TAG "STORAGE_MGR"
#define NVS_STORAGE_NAMESPACE "ui&ha_items"

QueueHandle_t nvs_save_queue = NULL;

static esp_err_t main_ui_item_save(ha_entity_t *p_slot, uint8_t key_index);
static void nvs_task(void *pvParameters) {
    uint8_t slot_to_save;
    while (1) {
        if (xQueueReceive(nvs_save_queue, &slot_to_save, portMAX_DELAY) == pdPASS) {
            main_ui_item_save(&g_main_ui_device_data[slot_to_save], slot_to_save);      
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t storage_init(void) {
    nvs_save_queue = xQueueCreate(5, sizeof(uint8_t));
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    BaseType_t res = xTaskCreatePinnedToCore(nvs_task, "nvs_worker", 4096,  NULL, 2, NULL,1);
    if (res != pdPASS) {
        ESP_LOGE("NVS_TASK", "Creat failed");
    }
    return err;
}

static const char* cont_nvs_key_get2(uint8_t key_index)
{
     switch(key_index){
        case 0:
            return "item_rt";
        case 1:
            return "item_rm";
        case 2:
            return "item_rd";
        case 3:
            return "item_md";
        case 4:
            return "item_time";
        case 5:
            return "item_temp";

        default:
            return NULL;
    }
}

static esp_err_t main_ui_item_save(ha_entity_t *p_slot, uint8_t key_index) 
{
    if (p_slot == NULL) return ESP_ERR_INVALID_ARG;
   
    const char *nvs_key = cont_nvs_key_get2(key_index);
    if (nvs_key == NULL) {
        ESP_LOGE(TAG, " Input invalid");
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(my_handle, nvs_key, p_slot, sizeof(ha_entity_t));
    if (err == ESP_OK) {
        err = nvs_commit(my_handle);
        ESP_LOGI(TAG, "Save to NVS, key: %s", nvs_key);
    }

    nvs_close(my_handle);
    return err;
}

static esp_err_t main_ui_item_load(ha_entity_t *p_slot, uint8_t key_index)
{
    if (p_slot == NULL) return ESP_ERR_INVALID_ARG;
    memset(p_slot, 0, sizeof(ha_entity_t));
    const char *nvs_key = cont_nvs_key_get2(key_index);
    if (nvs_key == NULL) return ESP_ERR_INVALID_ARG;

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    size_t required_size = 0;
    err = nvs_get_blob(my_handle, nvs_key, NULL, &required_size);
    
    
    if (err == ESP_OK && required_size == sizeof(ha_entity_t)) {
        err = nvs_get_blob(my_handle, nvs_key, p_slot, &required_size);
        if(err == ESP_OK){
            ESP_LOGE(TAG, "Load entity OK, key: %s, entity_id: %s, name: %s",nvs_key, p_slot->entity_id, p_slot->friendly_name);
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "No find key: %s en NVS", nvs_key);
    } else {
        err = ESP_ERR_NVS_INVALID_LENGTH;
    }

    nvs_close(my_handle);
    return err;
}




//HA_TOKEN, HA_IP Save and load
static const char* ha_nvs_key_get(const char *p_buf)
{
    if (p_buf == g_ha_ws_client_config.ha_ip){return "ha_ip";}
    if (p_buf == g_ha_ws_client_config.ha_token){return "ha_token";}
    return NULL;
}

esp_err_t ha_data_item_save(const char *p_buf)
{
    if (p_buf == NULL)
    {
        ESP_LOGE(TAG, "Input invalid");
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGI(TAG, "Ready to save string to NVS, content: [%s]", p_buf);
    const char *nvs_key = ha_nvs_key_get(p_buf);
    if (nvs_key == NULL)
    {
        ESP_LOGE(TAG, "Input invalid");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_str(my_handle, nvs_key, p_buf);
    if (err == ESP_OK)
    {
        err = nvs_commit(my_handle);
        ESP_LOGI(TAG, "Save to NVS, key: %s", nvs_key);
    }

    nvs_close(my_handle);
    return err;
}

esp_err_t ha_data_item_load(char *p_buf)
{
    if (p_buf == NULL)
    {
        ESP_LOGE(TAG, "Input invalid");
        return ESP_ERR_INVALID_ARG;
    }

    const char *nvs_key = ha_nvs_key_get(p_buf);
    if (nvs_key == NULL)
    {
        ESP_LOGE(TAG, "Input invalid");
        return ESP_ERR_INVALID_ARG;
    }

    size_t buf_max_len = (p_buf == g_ha_ws_client_config.ha_ip) ? sizeof(g_ha_ws_client_config.ha_ip) : sizeof(g_ha_ws_client_config.ha_token);
    memset(p_buf, 0, buf_max_len);

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Key %s not found in NVS", nvs_key);
        return err;
    }

    size_t str_len = 0;
    nvs_get_str(my_handle, nvs_key, NULL, &str_len);
    if (str_len > buf_max_len)
    {
        str_len = buf_max_len;
    }
    err = nvs_get_str(my_handle, nvs_key, p_buf,  &str_len);

    ESP_LOGE(TAG, "Load HA DATA OK, key: %s, val: %s", nvs_key, p_buf);
    nvs_close(my_handle);
    return err;
}


//All data Save and load
void all_data_load_init()
{
    ha_data_item_load(g_ha_ws_client_config.ha_ip);
    ha_data_item_load(g_ha_ws_client_config.ha_token); 
    for(int i=0;i<6;i++)
    {
        main_ui_item_load(&g_main_ui_device_data[i],i);
    }
}

void all_nvs_erase(void)
{   
    esp_err_t ret;
    nvs_close(0);
    ret = nvs_flash_erase();
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG,"擦除NVS失败:%s",esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG,"NVS分区全部擦除完成");
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}



void test_main_ui_item_save(){
    // strcpy(g_main_ui_device_data[4].entity_id, "sensor.date_time");
    // strcpy(g_main_ui_device_data[4].friendly_name, "DateTime");
    // strcpy(g_main_ui_device_data[4].state, "0");
    // strcpy(g_main_ui_device_data[5].entity_id, "weather.forecast_wo_de_jia");
    // strcpy(g_main_ui_device_data[5].friendly_name, "Myhome");
    // strcpy(g_main_ui_device_data[5].state, "0");
    
    // main_ui_item_save(&g_main_ui_device_data[4],4);
    // main_ui_item_save(&g_main_ui_device_data[5],5);
    
}
