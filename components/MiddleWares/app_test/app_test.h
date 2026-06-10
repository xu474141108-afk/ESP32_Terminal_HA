#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_IDLE,
    STATE_ERROR,
    STATE_STA_CONNECTING,
    STATE_PROVISIONING,
    STATE_CONNECTED,
    STATE_DISCONNECTED,
    STATE_NEWWIFI_SETTING
} wifi_state_t;

// 新增：传递给 LVGL UI 的数据载荷
typedef struct {
    int retry_progress;       // 重连进度（例如当前是第几次 1/5）
    char connected_ip[16];   // 连上后获取到的 IP 地址
    char ap_ssid[32];        // 自身发射热点的名称（供扫码/手动连）
} wifi_ui_data_t;

typedef struct {
    wifi_state_t current_state;
    int retry_count;
    bool has_nvs_record;
    wifi_ui_data_t ui_data;   // 状态机内嵌的 UI 数据源
} wifi_sm_t;

static wifi_sm_t g_wifi_sm;

void wifi_init(void);
void webserver_begin(void);
extern EventGroupHandle_t s_wifi_event_group;



#ifdef __cplusplus
}
#endif