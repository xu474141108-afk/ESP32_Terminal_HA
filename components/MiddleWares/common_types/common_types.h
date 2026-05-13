#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#define HA_DATA_BIT     BIT0    
#define HA_DATA_READY_BIT    BIT1
#define HA_DATA_UPDATED_BIT  BIT2
#define HA_DATA_REMOVED_BIT  BIT3
#define HA_DATA_CHANGED_BIT  BIT4

#define TASK_NIEVL_LVGL_FLUSH 5
#define TASK_NIVEL_OTA_DOWNLOAD 4
#define TASK_NIVEL_OTA_CHECK 2

extern EventGroupHandle_t ha_event_group;
extern EventGroupHandle_t s_wifi_event_group;

#ifdef __cplusplus
}
#endif