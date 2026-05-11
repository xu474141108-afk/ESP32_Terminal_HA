#include <stdio.h>
#include "sys.h"

//#include "widgets/menu/lv_menu_private.h"
#define WIFI_CONNECTED_BIT  BIT0

TaskHandle_t xWebTaskHandle = NULL;
static const char *TAG = "app_main";
extern EventGroupHandle_t s_wifi_event_group;




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
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(8000));
    }

}