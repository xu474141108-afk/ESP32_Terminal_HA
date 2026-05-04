#include <stdio.h>
#include "sys.h"

//#include "widgets/menu/lv_menu_private.h"
#define WIFI_CONNECTED_BIT  BIT0

TaskHandle_t xWebTaskHandle = NULL;
static const char *TAG = "app_main";
extern EventGroupHandle_t s_wifi_event_group;




void wifi_manager_task(void *pvParameters) {
    while (1) {
        // 这里的 portMAX_DELAY 让任务在没点击时完全不占 CPU
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
        ESP_LOGI(TAG, "启动 WebServer");
        webserver_begin(); // 执行你之前的启动函数
        ESP_LOGI(TAG, "WebServer 已启动，等待连接...");
    }
}


void app_main(void)
{
    bsp_display_init();
    vTaskDelay(pdMS_TO_TICKS(1000));
    wifi_init();
    ha_event_group = xEventGroupCreate();
    xEventGroupWaitBits(s_wifi_event_group, 
                       WIFI_CONNECTED_BIT, 
                       pdFALSE,          // 退出时不清除位（方便以后其它任务检查）
                       pdTRUE,           // 等待所有位
                       portMAX_DELAY);   // 无限期等待
    ESP_LOGI("MAIN", "WiFi 已就绪，");
    vTaskDelay(pdMS_TO_TICKS(2000));
    //xTaskCreate(wifi_manager_task, "wifi_task", 4096, NULL, 5, &xWebTaskHandle);

    while (1) {
        // print_ha_raw_states();
        vTaskDelay(pdMS_TO_TICKS(8000));
    }

}