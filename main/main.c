#include <stdio.h>
#include "sys.h"
#include "nvs_flash.h"
//#include "widgets/menu/lv_menu_private.h"
#define WIFI_CONNECTED_BIT  BIT0

TaskHandle_t xWebTaskHandle = NULL;
static const char *TAG = "app_main";
extern EventGroupHandle_t s_wifi_event_group;
void nvs_erase_wifi_config(void)
{   
    esp_err_t ret;
    nvs_close(0);

    // 3. 全盘擦除NVS分区
    ret = nvs_flash_erase();
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG,"擦除NVS失败:%s",esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG,"NVS分区全部擦除完成");

    // 4. 关键：擦完强制重新初始化NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}


void app_main(void)
{
    storage_init();
    // nvs_erase_wifi_config(); // 开发阶段清空WiFi配置，正式发布前请注释掉这行代码
    bsp_display_init();
    vTaskDelay(pdMS_TO_TICKS(1000));
    wifi_init();
    // xEventGroupWaitBits(s_wifi_event_group, 
    //                    WIFI_CONNECTED_BIT, 
    //                    pdFALSE,         
    //                    pdTRUE,           
    //                    portMAX_DELAY);   
    // ESP_LOGI(TAG, "WiFi 已就绪，");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ota_mqtt_init();
    websocket_app_start();
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(8000));

    }
}