#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DEVICES 50

typedef enum {
    OTA_STATE_IDLE,          // 空闲
    OTA_STATE_CHECKING,      // 正在检测版本
    OTA_STATE_HTTP_ERROR,    // HTTP 请求失败
    OTA_STATE_READY,         // 发现新版本，等待用户确认
    OTA_STATE_NO_NEW,        // 已经是最新版本
    OTA_STATE_DOWNLOADING,   // 正在下载/写入
    OTA_STATE_INSTALLING,    // 正在校验/安装
    OTA_STATE_SUCCESS,       // 更新成功
    OTA_STATE_FAILED,        // 更新失败
    OTA_STATE_LEN_NOFIT      // 接收数据长度不匹配
} ha_state_t;

typedef struct {
    char domain[16];
    char integration[32];
    char unique_id[32];
    char object_id[32];
} ha_entity_t;

typedef struct {
    char entity_id[64];
    bool state;
    char friendly_name[64];
} ha_device_t;



extern ha_device_t g_device_list[MAX_DEVICES];
extern int g_device_count;


void get_ha_states_to_psram(void);

#ifdef __cplusplus
}
#endif