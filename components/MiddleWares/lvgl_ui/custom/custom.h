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
#include "common_types.h"
#include "gui_guider.h"

void custom_init(lv_ui *ui);
void HA_json_to_list(lv_obj_t *list_obj, ha_device_t *devices, int count) ;
void OTA_check_new_version_available();
void task_OTA_state_monitor(lv_timer_t * timer);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
