
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"

#define WIFI_CONNECTION_ERROR              (1 << 0)





static char new_version_str[32] = {0};



void check_new_version_available(void)
{
    // 通过 HTTP 请求获取到了服务器的新版本号
    // strcpy(new_version_str, "v1.2.0"); 
    
    // 比较新版本和当前运行的版本
    esp_app_desc_t running_app_info;
    const esp_partition_t *running = esp_ota_get_running_partition();
    
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI("OTA", "当前版本: %s", running_app_info.version);
        //设置为1
    }
        //标志位设置为0
}
