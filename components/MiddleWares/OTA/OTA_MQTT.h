#pragma once
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    OTA_STATE_IDLE,          // Idle state
    OTA_STATE_OTA_SCREEN,    // OTA page displayed
    OTA_STATE_CHECKING,      // Checking firmware version
    OTA_STATE_HTTP_ERROR,    // HTTP request failed
    OTA_STATE_READY,         // New version found, waiting for user confirmation
    OTA_STATE_NO_NEW,        // Already the latest version
    OTA_STATE_DOWNLOADING,   // Downloading & writing firmware
    OTA_STATE_INSTALLING,    // Verifying and installing firmware
    OTA_STATE_SUCCESS,       // OTA update succeeded
    OTA_STATE_FAILED,        // OTA update failed
    OTA_STATE_LEN_NOFIT      // Mismatch of received data length
} ota_state_t;

typedef struct {
    ota_state_t state;
    char current_ver[32]; 
    char latest_ver[32];
} ota_context_t;
extern ota_context_t g_ota_ctx;

void ota_mqtt_init(void);
#ifdef __cplusplus
}
#endif