#pragma once
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OTA_STATUS_NEW_VERSION_AVAILABLE = 0, // 有新版本
    OTA_STATUS_NO_NEW_VERSION,            // 已是最新版本 (包括与 invalid 分区相同)
    OTA_STATUS_HTTP_FAILED
} ota_check_result_t;

typedef enum {
    OTA_STATE_IDLE,          // 空闲
    OTA_STATE_CHECKING,      // 正在检测版本
    OTA_STATE_HTTP_ERROR,    // HTTP 请求失败
    OTA_STATE_READY,         // 发现新版本，等待用户确认
    OTA_STATE_NO_NEW,        // 已经是最新版本
    OTA_STATE_DOWNLOADING,   // 正在下载/写入
    OTA_STATE_INSTALLING,    // 正在校验/安装
    OTA_STATE_SUCCESS,       // 更新成功
    OTA_STATE_FAILED         // 更新失败
} ota_state_t;

typedef struct {
    ota_state_t state;
    esp_http_client_handle_t http_client;
    esp_ota_handle_t update_handle;
    const esp_partition_t *update_partition;
    int data_read;
} ota_context_t;



extern ota_context_t g_ota_ctx;

void OTA_version_check_task(void *pvParameters);
void write_ota_data(ota_context_t *ctx);


#ifdef __cplusplus
}
#endif