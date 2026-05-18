#pragma once
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    OTA_STATE_IDLE,          // 空闲
    OTA_STATE_OTA_SCREEN,    // 正在检测版本
    OTA_STATE_CHECKING,      // 正在检测版本
    OTA_STATE_HTTP_ERROR,    // HTTP 请求失败
    OTA_STATE_READY,         // 发现新版本，等待用户确认
    OTA_STATE_NO_NEW,        // 已经是最新版本
    OTA_STATE_DOWNLOADING,   // 正在下载/写入
    OTA_STATE_INSTALLING,    // 正在校验/安装
    OTA_STATE_SUCCESS,       // 更新成功
    OTA_STATE_FAILED,        // 更新失败
    OTA_STATE_LEN_NOFIT      // 接收数据长度不匹配
} ota_state_t;

typedef struct {
    ota_state_t state;
    const esp_partition_t *update_partition;
    int data_read;
    char download_url[256];
    char current_ver[32]; 
    char latest_ver[32];
} ota_context_t;



extern ota_context_t g_ota_ctx;

void OTA_download_task(void *pvParameters);
void OTA_autoscan_task(void *pvParameters);

#ifdef __cplusplus
}
#endif