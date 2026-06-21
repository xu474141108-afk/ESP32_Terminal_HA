#pragma once
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    OTA_MQTT_STATE_IDLE,          // Idle state
    OTA_MQTT_STATE_READY,         // New version found, waiting for user confirmation
    OTA_MQTT_STATE_DOWNLOADING,   // Downloading & writing firmware
    OTA_MQTT_STATE_SUCCESS,       // OTA update succeeded
    OTA_MQTT_STATE_FAILED,        // OTA update failed
} ota_mqtt_state_t;

typedef struct {
    ota_mqtt_state_t state;
    char current_ver[32]; 
    char latest_ver[32];
} ota_mqtt_context_t;
extern ota_mqtt_context_t g_ota_mqtt_ctx;

void ota_mqtt_init(void);
#ifdef __cplusplus
}
#endif