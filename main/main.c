#include <stdio.h>
#include "sys.h"
#include "OTA.h"
#include "app_test.h"
//#include "widgets/menu/lv_menu_private.h"
#define WIFI_CONNECTED_BIT  BIT0

TaskHandle_t xWebTaskHandle = NULL;
static const char *TAG = "app_main";
extern EventGroupHandle_t s_wifi_event_group;
static SemaphoreHandle_t ha_poll_mutex = NULL;

static void ha_poll_task(void *arg)
{
    int64_t cycle_start = esp_timer_get_time();
    int64_t ha_work_time_us = 0;

    while (1)
    {
        int64_t t0 = esp_timer_get_time();
        if (ha_poll_mutex != NULL) 
        {
            // 拿到锁才允许执行采集
            if (xSemaphoreTake(ha_poll_mutex, portMAX_DELAY) == pdTRUE)
            {
                ha_http_control_get_states_to_psram();
                xSemaphoreGive(ha_poll_mutex);
            }
        }

        int64_t t1 = esp_timer_get_time();
        ha_work_time_us += (t1 - t0); // 累加 HTTP 任务真正的纯工作时间

        // 3. 结算逻辑
        int64_t now = esp_timer_get_time();
        if (now - cycle_start >= 10000000) { // 每 10 秒算一次
            int ha_cpu = (int)(((double)ha_work_time_us / (double)(now - cycle_start)) * 100.0);
            ESP_LOGI("HA_CPU", "HTTP 任务单独占用 Core 0 算力: %d%%", ha_cpu);
            
            // 联动：如果这个任务自己就吃掉了 80% 的算力，主动减速
            if (ha_cpu > 80) { /* 调整逻辑 */ }

            ha_work_time_us = 0;
            cycle_start = now;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    // bsp_display_init();
    ha_poll_mutex = xSemaphoreCreateMutex();
    if (ha_poll_mutex == NULL) {
        ESP_LOGE(TAG, "全局互斥锁创建失败！系统挂起");
        while(1) vTaskDelay(100);
    }
    wifi_init();
    ha_event_group = xEventGroupCreate();
    xEventGroupWaitBits(s_wifi_event_group, 
                       WIFI_CONNECTED_BIT, 
                       pdFALSE,         
                       pdTRUE,           
                       portMAX_DELAY);   
    ESP_LOGI(TAG, "WiFi 已就绪，");
    vTaskDelay(pdMS_TO_TICKS(2000));
    // xTaskCreate(OTA_autoscan_task, "ota_auto_scan_task", 8192, NULL, TASK_NIVEL_OTA_CHECK, NULL);
    xTaskCreate(ha_poll_task, "ha_poll_task", 4096, NULL, 5, NULL);
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(8000));

    }

}