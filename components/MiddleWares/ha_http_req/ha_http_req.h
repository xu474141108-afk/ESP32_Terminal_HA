#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DEVICES 50

typedef enum {
    HA_STATE_IDLE,          // 空闲
    HA_STATE_SEARCHING,      // 正在检测版本
    HA_STATE_HTTP_ERROR,    // HTTP 请求失败
    HA_STATE_JSON_ERROR,    // JSON 解析失败
    HA_STATE_READY,         // 发现新版本，等待用户确认
    HA_STATE_DOWNLOADING,   // 正在写入LIST
    HA_STATE_SUCCESS,       // 更新LIST成功
    HA_STATE_SHOW_CONT,       // 显示设备详情
    HA_STATE_HOLD_CONT,
    HA_STATE_CLOSE_CONT, // 关闭设备详情
    HA_STATE_CONT_RT,
    HA_STATE_CONT_RM,
    HA_STATE_CONT_RD,
    HA_STATE_CONT_MD,
    HA_STATE_FAILED         // 更新LIST失败
} ha_state_t;

typedef struct {
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
void ha_http_control_get_states_to_psram();
#ifdef __cplusplus
}
#endif