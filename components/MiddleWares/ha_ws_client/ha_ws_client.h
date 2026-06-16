#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char ha_ip[32];
    char ha_token[256];
} ha_ws_client_config_t;
extern ha_ws_client_config_t g_ha_ws_client_config;

void websocket_app_start();
void ha_get_states_request(void);

#ifdef __cplusplus
}
#endif