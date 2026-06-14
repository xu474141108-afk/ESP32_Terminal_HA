#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WIFI_STATE_IDLE,
    WIFI_STATE_NONVS_CONFIG,
    WIFI_STATE_STA_CONNECTING,
    WIFI_STATE_WAIT_BEGIN_PROVISIONING, 
    WIFI_STATE_PROVISIONING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_DISCONNECTED,
} wifi_state_t;


typedef struct {
    wifi_state_t wifi_FSM_state;
    int retry_progress;      // 重连进度（例如当前是第几次 1/5）
    char connected_ip[16];   // 连上后获取到的 IP 地址
    char ap_ssid[32];        // 自身发射热点的名称（供扫码/手动连）
} wifi_sm_t;

extern wifi_sm_t g_wifi_sm;

void wifi_init(void);
void webserver_begin(void);
extern EventGroupHandle_t s_wifi_event_group;
extern QueueHandle_t g_ui_status_queue;


#ifdef __cplusplus
}
#endif