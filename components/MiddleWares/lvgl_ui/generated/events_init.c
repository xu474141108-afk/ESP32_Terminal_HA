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


static void screen_main_cont_setup_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_main_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_main (lv_ui *ui)
{
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

static void screen_wifi_btn_wifi_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_wifi_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_wifi (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_wifi_btn_wifi_back, screen_wifi_btn_wifi_back_event_handler, LV_EVENT_ALL, ui);
}

static void screen_HA_btn_HA_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_HA_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_HA (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_HA_btn_HA_back, screen_HA_btn_HA_back_event_handler, LV_EVENT_ALL, ui);
}

static void screen_OTA_btn_OTA_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_setup, guider_ui.screen_setup_del, &guider_ui.screen_OTA_del, setup_scr_screen_setup, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, true, true);
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
