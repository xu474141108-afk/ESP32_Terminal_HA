#include <stdio.h>
#include "sys.h"
#include "OTA.h"
//#include "widgets/menu/lv_menu_private.h"
#define WIFI_CONNECTED_BIT  BIT0

TaskHandle_t xWebTaskHandle = NULL;
static const char *TAG = "app_main";
extern EventGroupHandle_t s_wifi_event_group;
static SemaphoreHandle_t ha_poll_mutex = NULL;

static void ha_poll_task(void *arg)
{
    // 互斥锁：防止上一次HTTP没跑完又触发下一次
    ha_poll_mutex = xSemaphoreCreateMutex();
    if (!ha_poll_mutex) {
        ESP_LOGE("ha_poll_task", "Mutex create fail");
        vTaskDelete(NULL);
    }

    while (1)
    {
        // 拿到锁才允许执行采集
        if (xSemaphoreTake(ha_poll_mutex, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI("ha_poll_task", "Start HA poll cycle");
            // 调用你现成的HTTP+PSRAM采集函数
            ha_http_control_get_states_to_psram();

            xSemaphoreGive(ha_poll_mutex);
        }

        // 固定100ms周期延时
        vTaskDelay(pdMS_TO_TICKS(100));
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
                       pdFALSE,         
                       pdTRUE,           
                       portMAX_DELAY);   
    ESP_LOGI(TAG, "WiFi 已就绪，");
    vTaskDelay(pdMS_TO_TICKS(2000));
    xTaskCreate(OTA_autoscan_task, "ota_auto_scan_task", 8192, NULL, TASK_NIVEL_OTA_CHECK, NULL);
    xTaskCreate(ha_poll_task, "ha_poll_task", 4096, NULL, 5, NULL);
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(8000));

    }

}