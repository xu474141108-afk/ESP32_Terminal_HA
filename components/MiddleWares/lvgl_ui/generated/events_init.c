/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

#include "custom.h"
#include "app_wifi.h"
#include "ha_ws_client.h"
#include "esp_log.h"

static void screen_main_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui * ui = (lv_ui *)lv_event_get_user_data(e);
        lv_label_set_text(ui->screen_main_lab_cont_rt, g_main_ui_device_data[0].friendly_name);
        lv_label_set_text(ui->screen_main_lab_cont_rm, g_main_ui_device_data[1].friendly_name);
        lv_label_set_text(ui->screen_main_lab_cont_rd, g_main_ui_device_data[2].friendly_name);
        lv_label_set_text(ui->screen_main_lab_cont_md, g_main_ui_device_data[3].friendly_name);
        lv_label_set_text(guider_ui.screen_main_label_date, g_main_ui_device_data[4].state);
        break;
    }
    default:
        break;
    }
}

static void screen_main_cont_md_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ESP_LOGI("EVENTS","MD ID: %s Name: %s",g_main_ui_device_data[3].entity_id,g_main_ui_device_data[3].friendly_name);
        esp_event_post(HA_ACTION_EVENTS, HA_WS_LVGL_BUTTON_MD_TOGGLE, NULL, 0, portMAX_DELAY);
        break;
    }
    default:
        break;
    }
}

static void screen_main_cont_rd_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ESP_LOGI("EVENTS","RD ID: %s Name: %s",g_main_ui_device_data[2].entity_id,g_main_ui_device_data[2].friendly_name);
        esp_event_post(HA_ACTION_EVENTS, HA_WS_LVGL_BUTTON_RD_TOGGLE, NULL, 0, portMAX_DELAY);
        break;
    }
    default:
        break;
    }
}

static void screen_main_cont_rm_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ESP_LOGI("EVENTS","RM ID: %s Name: %s",g_main_ui_device_data[1].entity_id,g_main_ui_device_data[1].friendly_name);
        esp_event_post(HA_ACTION_EVENTS, HA_WS_LVGL_BUTTON_RM_TOGGLE, NULL, 0, portMAX_DELAY);
        break;
    }
    default:
        break;
    }
}

static void screen_main_cont_rt_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ESP_LOGI("EVENTS","RT ID: %s Name: %s",g_main_ui_device_data[0].entity_id,g_main_ui_device_data[0].friendly_name);
        esp_event_post(HA_ACTION_EVENTS, HA_WS_LVGL_BUTTON_RT_TOGGLE, NULL, 0, portMAX_DELAY);
        break;
    }
    default:
        break;
    }
}

static void screen_main_cont_setup_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_main_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 200, true, true);
        if(main_timer != NULL){
            lv_timer_pause(main_timer);
        }
        is_first_load=true;
        // lv_timer_pause(main_weather_timer);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_main (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_main, screen_main_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_main_cont_md, screen_main_cont_md_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_main_cont_rd, screen_main_cont_rd_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_main_cont_rm, screen_main_cont_rm_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_main_cont_rt, screen_main_cont_rt_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_main_cont_setup, screen_main_cont_setup_event_handler, LV_EVENT_ALL, ui);
}

static void screen_setup_cont_ELSE_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_System, guider_ui.screen_System_del, &guider_ui.screen_setup_del, setup_scr_screen_System, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void screen_setup_cont_OTA_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_OTA, guider_ui.screen_OTA_del, &guider_ui.screen_setup_del, setup_scr_screen_OTA, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 200, true, true);
        lv_timer_resume(ota_timer);
        break;
    }
    default:
        break;
    }
}

static void screen_setup_cont_HA_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_HA, guider_ui.screen_HA_del, &guider_ui.screen_setup_del, setup_scr_screen_HA, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 200, true, true);
        lv_timer_resume(ha_timer);
        break;
    }
    default:
        break;
    }
}

static void screen_setup_cont_wifi_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_wifi, guider_ui.screen_wifi_del, &guider_ui.screen_setup_del, setup_scr_screen_wifi, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 200, true, true);
        lv_timer_resume(wifi_timer);
        is_first_load_wifi = true;
        break;
    }
    default:
        break;
    }
}

static void screen_setup_btn_setup_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_main, guider_ui.screen_main_del, &guider_ui.screen_setup_del, setup_scr_screen_main, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 200, 200, true, true);
        if(main_timer != NULL){
            lv_timer_resume(main_timer);
        }

        // lv_timer_resume(main_weather_timer);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_setup (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_setup_cont_ELSE, screen_setup_cont_ELSE_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_setup_cont_OTA, screen_setup_cont_OTA_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_setup_cont_HA, screen_setup_cont_HA_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_setup_cont_wifi, screen_setup_cont_wifi_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_setup_btn_setup_back, screen_setup_btn_setup_back_event_handler, LV_EVENT_ALL, ui);
}

static void screen_wifi_btn_wifi_ap_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        webserver_begin();
        break;
    }
    default:
        break;
    }
}

static void screen_wifi_btn_wifi_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_wifi_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 200, 200, true, true);
        lv_timer_pause(wifi_timer);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_wifi (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_wifi_btn_wifi_ap, screen_wifi_btn_wifi_ap_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_wifi_btn_wifi_back, screen_wifi_btn_wifi_back_event_handler, LV_EVENT_ALL, ui);
}

static void screen_HA_btn_HA_check_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ha_rest_get_states_to_psram();
        break;
    }
    default:
        break;
    }
}

static void screen_HA_btn_HA_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_HA_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, true, true);
        lv_timer_pause(ha_timer);
        break;
    }
    default:
        break;
    }
}

static void screen_HA_cont_rt_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        g_HAdevice_ctx.state_ha = HA_STATE_CONT_RT;
        break;
    }
    default:
        break;
    }
}

static void screen_HA_cont_rm_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        g_HAdevice_ctx.state_ha = HA_STATE_CONT_RM;
        break;
    }
    default:
        break;
    }
}

static void screen_HA_cont_md_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        g_HAdevice_ctx.state_ha = HA_STATE_CONT_MD;
        break;
    }
    default:
        break;
    }
}

static void screen_HA_cont_rd_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        g_HAdevice_ctx.state_ha = HA_STATE_CONT_RD;
        break;
    }
    default:
        break;
    }
}

void events_init_screen_HA (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_HA_btn_HA_check, screen_HA_btn_HA_check_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_HA_btn_HA_back, screen_HA_btn_HA_back_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_HA_cont_rt, screen_HA_cont_rt_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_HA_cont_rm, screen_HA_cont_rm_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_HA_cont_md, screen_HA_cont_md_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_HA_cont_rd, screen_HA_cont_rd_event_handler, LV_EVENT_ALL, ui);
}

static void screen_OTA_btn_OTA_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_OTA_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, true, true);
        lv_timer_pause(ota_timer);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_OTA (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_OTA_btn_OTA_back, screen_OTA_btn_OTA_back_event_handler, LV_EVENT_ALL, ui);
}

static void screen_System_btn_System_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_System_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_System (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_System_btn_System_back, screen_System_btn_System_back_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
