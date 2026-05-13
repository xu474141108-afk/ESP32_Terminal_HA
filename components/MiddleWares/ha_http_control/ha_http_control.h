#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DEVICES 50

typedef enum {
    HA_STATE_IDLE,          // 空闲
    HA_STATE_CHECKING,      // 正在检测版本
    HA_STATE_HTTP_ERROR,    // HTTP 请求失败
    HA_STATE_JSON_ERROR,    // HTTP 请求失败
    HA_STATE_READY,         // 发现新版本，等待用户确认
    HA_STATE_DOWNLOADING,   // 正在下载/写入
    HA_STATE_SUCCESS,       // 更新成功
    HA_STATE_FAILED        // 更新失败
} ha_state_t;

typedef struct {
    char domain[16];
    char integration[32];
    char unique_id[32];
    char object_id[32];
    char entity_id[64];
    char friendly_name[64];
    bool is_on;
} ha_entity_t;

typedef struct {
    ha_state_t state_ha;
    ha_entity_t entity[MAX_DEVICES];
    int device_count;
} ha_device_t;



extern ha_device_t g_HAdevice_ctx;

void get_ha_states_to_psram(void);

#ifdef __cplusplus
}
#endif