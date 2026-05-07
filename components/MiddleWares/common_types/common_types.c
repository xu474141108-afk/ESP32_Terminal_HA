#include "common_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


// 全局变量：设备列表和事件组
ha_device_t g_device_list[MAX_DEVICES];
int g_device_count = 0;
EventGroupHandle_t ha_event_group;

EventGroupHandle_t s_wifi_event_group = NULL;
EventGroupHandle_t ota_event_group = NULL;