#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OTA_STATUS_NEW_VERSION_AVAILABLE = 0, // 有新版本
    OTA_STATUS_NO_NEW_VERSION,            // 已是最新版本 (包括与 invalid 分区相同)
    OTA_STATUS_HTTP_FAILED
} ota_check_result_t;

ota_check_result_t check_new_version_available(void);
void write_ota_data(void * pvParameters);

#ifdef __cplusplus
}
#endif