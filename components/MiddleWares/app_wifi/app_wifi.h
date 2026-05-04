#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

void wifi_init(void);
void webserver_begin(void);
extern EventGroupHandle_t s_wifi_event_group;

#ifdef __cplusplus
}
#endif