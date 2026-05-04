#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define MAX_DEVICES 50
#define HA_DATA_READY_BIT  BIT0

#define HA_URL "http://192.168.1.137:8123/api/services/light/turn_on"
#define HA_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJhYTcwZmE1OWQ2NjI0ZDFiYWVkNjFiM2MxZTU3MDdlZSIsImlhdCI6MTc3NzI5Mjk5NywiZXhwIjoyMDkyNjUyOTk3fQ.Z5T5IGwJm6M56h8j40y3HeuLgPwIlMwR1bQ0DxRIinI"

typedef struct {
    char domain[16];
    char integration[32];
    char unique_id[32];
    char object_id[32];
} ha_entity_t;

typedef struct {
    ha_entity_t entity_id;
    bool state;
    char friendly_name[64];
} ha_device_t_borrar;

typedef struct {
    char entity_id[64];
    bool state;
    char friendly_name[64];
} ha_device_t;

extern ha_device_t g_device_list[MAX_DEVICES];
extern int g_device_count;
extern EventGroupHandle_t ha_event_group;
extern EventGroupHandle_t s_wifi_event_group;


#ifdef __cplusplus
}
#endif