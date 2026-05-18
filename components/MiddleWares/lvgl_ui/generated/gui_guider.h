/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *screen_main;
	bool screen_main_del;
	lv_obj_t *screen_main_cont_md;
	lv_obj_t *screen_main_cont_rd;
	lv_obj_t *screen_main_cont_rm;
	lv_obj_t *screen_main_cont_rt;
	lv_obj_t *screen_main_cont_setup;
	lv_obj_t *screen_main_lab_cont_setup;
	lv_obj_t *screen_main_img_cont_setup;
	lv_obj_t *screen_main_cont_weather;
	lv_obj_t *screen_main_label_weather;
	lv_obj_t *screen_setup;
	bool screen_setup_del;
	lv_obj_t *screen_setup_cont_setup_main;
	lv_obj_t *screen_setup_cont_ELSE;
	lv_obj_t *screen_setup_label_4;
	lv_obj_t *screen_setup_cont_OTA;
	lv_obj_t *screen_setup_label_3;
	lv_obj_t *screen_setup_cont_HA;
	lv_obj_t *screen_setup_label_2;
	lv_obj_t *screen_setup_cont_wifi;
	lv_obj_t *screen_setup_label_1;
	lv_obj_t *screen_setup_cont_setup_top;
	lv_obj_t *screen_setup_label_setup_main;
	lv_obj_t *screen_setup_btn_setup_back;
	lv_obj_t *screen_setup_btn_setup_back_label;
	lv_obj_t *screen_wifi;
	bool screen_wifi_del;
	lv_obj_t *screen_wifi_cont_wifi_main;
	lv_obj_t *screen_wifi_animimg_wifi;
	lv_obj_t *screen_wifi_label_wifi_info2;
	lv_obj_t *screen_wifi_label_wifi_info1;
	lv_obj_t *screen_wifi_label_wifi_state;
	lv_obj_t *screen_wifi_cont_wifi_top;
	lv_obj_t *screen_wifi_btn_wifi_ap;
	lv_obj_t *screen_wifi_btn_wifi_ap_label;
	lv_obj_t *screen_wifi_label_wifi_main;
	lv_obj_t *screen_wifi_btn_wifi_back;
	lv_obj_t *screen_wifi_btn_wifi_back_label;
	lv_obj_t *screen_HA;
	bool screen_HA_del;
	lv_obj_t *screen_HA_cont_HA_main;
	lv_obj_t *screen_HA_list_HA_show;
	lv_obj_t *screen_HA_list_HA_show_item0;
	lv_obj_t *screen_HA_cont_HA_top;
	lv_obj_t *screen_HA_btn_HA_check;
	lv_obj_t *screen_HA_btn_HA_check_label;
	lv_obj_t *screen_HA_label_HA_main;
	lv_obj_t *screen_HA_btn_HA_back;
	lv_obj_t *screen_HA_btn_HA_back_label;
	lv_obj_t *screen_OTA;
	bool screen_OTA_del;
	lv_obj_t *screen_OTA_cont_HA_main;
	lv_obj_t *screen_OTA_animimg_OTA;
	lv_obj_t *screen_OTA_label_10;
	lv_obj_t *screen_OTA_label_9;
	lv_obj_t *screen_OTA_label_8;
	lv_obj_t *screen_OTA_cont_HA_top;
	lv_obj_t *screen_OTA_btn_OTA_check;
	lv_obj_t *screen_OTA_btn_OTA_check_label;
	lv_obj_t *screen_OTA_label_OTA_main;
	lv_obj_t *screen_OTA_btn_OTA_back;
	lv_obj_t *screen_OTA_btn_OTA_back_label;
	lv_obj_t *screen_System;
	bool screen_System_del;
	lv_obj_t *screen_System_cont_setup_main;
	lv_obj_t *screen_System_cont_4;
	lv_obj_t *screen_System_label_9;
	lv_obj_t *screen_System_cont_3;
	lv_obj_t *screen_System_label_8;
	lv_obj_t *screen_System_cont_2;
	lv_obj_t *screen_System_label_7;
	lv_obj_t *screen_System_cont_1;
	lv_obj_t *screen_System_label_6;
	lv_obj_t *screen_System_cont_setup_top;
	lv_obj_t *screen_System_label_System_main;
	lv_obj_t *screen_System_btn_System_back;
	lv_obj_t *screen_System_btn_System_back_label;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_screen_main(lv_ui *ui);
void setup_scr_screen_setup(lv_ui *ui);
void setup_scr_screen_wifi(lv_ui *ui);
void setup_scr_screen_HA(lv_ui *ui);
void setup_scr_screen_OTA(lv_ui *ui);
void setup_scr_screen_System(lv_ui *ui);
LV_IMAGE_DECLARE(_setup_RGB565A8_40x40);

LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_montserratMedium_18)
LV_FONT_DECLARE(lv_font_montserratMedium_12)


#ifdef __cplusplus
}
#endif
#endif
