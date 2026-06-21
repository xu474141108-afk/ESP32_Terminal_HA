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
typedef struct {
    char ha_ip[32];
    char ha_token[256];
} ha_ws_client_config_t;
extern ha_ws_client_config_t g_ha_ws_client_config;

void websocket_app_start();


#ifdef __cplusplus
}
#endif