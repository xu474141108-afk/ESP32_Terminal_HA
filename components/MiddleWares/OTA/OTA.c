#include "lvgl.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"

#define WIFI_CONNECTION_ERROR              (1 << 0)


#define OTA_VERSION_AVAILABLE_FLAG         (1 << 0) // 标志位：新版本可用
#define OTA_UPDATE_STARTED_FLAG            (1 << 1) // 标志位：OTA 更新开始
#define OTA_UPDATE_CANCELLED_FLAG          (1 << 2) // 标志位：OTA 更新取消         
#define OTA_EVENT_SUCCESS                  (1 << 3) // 更新成功
#define OTA_EVENT_FAILED                   (1 << 4) // 更新失败
#define OTA_EVENT_REBOOT                   (1 << 5) // 请求重启


static char new_version_str[32] = {0};



// 检查版本的逻辑（这里简化模拟，您可以调用 HTTP API 获取版本号）
void check_new_version_available(void)
{
    // 假设您的逻辑通过 HTTP 请求获取到了服务器的新版本号
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
