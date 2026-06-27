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
    char wifi_ip[16];
    char wifi_ssid[32];
} wifi_sm_t;

extern wifi_sm_t g_wifi_sm;

void wifi_init(void);
void webserver_begin(void);


#ifdef __cplusplus
}
#endif