#pragma once
#include "esp_event.h"
#ifdef __cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(HA_ACTION_EVENTS);
enum {
    HA_SYSTEM_CONNECTED,
    HA_DATA_RECEIVED,      // 在这里定义你的事件 ID
    LVGL_WS_BUTTON_RT_TOGGLE,
    LVGL_WS_BUTTON_RM_TOGGLE,
    LVGL_WS_BUTTON_RD_TOGGLE,
    LVGL_WS_BUTTON_MD_TOGGLE,
    HA_WS_STATE_KICKED_OUT,
    HA_TOKEN_IP_UPDATE,
    LVGL_REQ_ALL_DATA,
};


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
    HA_STATE_FIGURA_SHOW,
    HA_STATE_FAILED         // 更新LIST失败
} ha_state_t;

typedef struct {
    char ha_ip[32];
    char ha_token[256];
} ha_ws_client_config_t;

typedef struct {
    char entity_id[64];
    char friendly_name[64];
    char state[64];
} ha_entity_t;


typedef struct {
    ha_state_t state_ha;
    ha_entity_t entity[50];
    int device_count;
} ha_device_t;

extern ha_device_t g_HAdevice_ctx;
extern ha_entity_t g_main_ui_device_data[6];
extern ha_ws_client_config_t g_ha_ws_client_config;


void get_ha_states_to_psram(void);
void websocket_app_start();


#ifdef __cplusplus
}
#endif