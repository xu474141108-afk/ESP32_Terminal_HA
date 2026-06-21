/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "gui_guider.h"



extern lv_timer_t *wifi_timer;
extern lv_timer_t *ha_timer;
extern lv_timer_t *ota_timer;


void custom_init(lv_ui *ui);
//OTA升级界面
void OTA_state_monitor_task(lv_timer_t * timer);
void OTA_MQTT_state_monitor_task(lv_timer_t * timer);
//HA设备操作界面
void HA_state_monitor_task(lv_timer_t * timer);

void WIFI_state_monitor_task(lv_timer_t * timer);


void all_timer_creat_init();
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
