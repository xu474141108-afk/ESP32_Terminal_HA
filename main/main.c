#include <stdio.h>
#include "sys.h"
#include "nvs_flash.h"
//#include "widgets/menu/lv_menu_private.h"

static const char *TAG = "app_main";


void app_main(void)
{
    storage_init();
    // all_nvs_erase();
    all_date_load_init();
    bsp_display_init();
    vTaskDelay(pdMS_TO_TICKS(1000));
    wifi_init();
    vTaskDelay(pdMS_TO_TICKS(4000));
    //ota_mqtt_init();
    websocket_app_start();
    //ws_ha_test_init();
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_event_post(HA_ACTION_EVENTS, HA_WS_TEST, NULL, 0, portMAX_DELAY);
    }
}