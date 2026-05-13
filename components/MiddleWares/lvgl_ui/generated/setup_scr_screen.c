/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



int screen_digital_clock_1_min_value = 25;
int screen_digital_clock_1_hour_value = 17;
int screen_digital_clock_1_sec_value = 50;
void setup_scr_screen(lv_ui *ui)
{
    //Write codes screen
    ui->screen = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen, lv_color_hex(0x00fffc), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_tabview_main
    ui->screen_tabview_main = lv_tabview_create(ui->screen);
    lv_obj_set_pos(ui->screen_tabview_main, 0, 30);
    lv_obj_set_size(ui->screen_tabview_main, 320, 210);
    lv_obj_set_scrollbar_mode(ui->screen_tabview_main, LV_SCROLLBAR_MODE_OFF);
    lv_tabview_set_tab_bar_position(ui->screen_tabview_main, LV_DIR_TOP);
    lv_tabview_set_tab_bar_size(ui->screen_tabview_main, 0);
    ui->screen_tabview_main_tab_1 = lv_tabview_add_tab(ui->screen_tabview_main, "Main");
    ui->screen_tabview_main_tab_2 = lv_tabview_add_tab(ui->screen_tabview_main, "HA");
    ui->screen_tabview_main_tab_3 = lv_tabview_add_tab(ui->screen_tabview_main, " Setting");

    //Write style for screen_tabview_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_tabview_main, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_tabview_main, lv_color_hex(0xeaeff3), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_tabview_main, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_tabview_main, lv_color_hex(0x4d4d4d), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_tabview_main, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_tabview_main, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_tabview_main, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_tabview_main, 16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_tabview_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tabview_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_tabview_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_tabview_main_extra_btnm_main_default
    static lv_style_t style_screen_tabview_main_extra_btnm_main_default;
    ui_init_style(&style_screen_tabview_main_extra_btnm_main_default);

    lv_style_set_bg_opa(&style_screen_tabview_main_extra_btnm_main_default, 255);
    lv_style_set_bg_color(&style_screen_tabview_main_extra_btnm_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_screen_tabview_main_extra_btnm_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_border_width(&style_screen_tabview_main_extra_btnm_main_default, 0);
    lv_style_set_radius(&style_screen_tabview_main_extra_btnm_main_default, 0);
    for(uint32_t i = 0; i < lv_tabview_get_tab_count(ui->screen_tabview_main); i++)
    {
        lv_obj_add_style(lv_obj_get_child(lv_tabview_get_tab_bar(ui->screen_tabview_main), i), &style_screen_tabview_main_extra_btnm_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    }

    //Write style state: LV_STATE_DEFAULT for &style_screen_tabview_main_extra_btnm_items_default
    static lv_style_t style_screen_tabview_main_extra_btnm_items_default;
    ui_init_style(&style_screen_tabview_main_extra_btnm_items_default);

    lv_style_set_text_color(&style_screen_tabview_main_extra_btnm_items_default, lv_color_hex(0x4d4d4d));
    lv_style_set_text_font(&style_screen_tabview_main_extra_btnm_items_default, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_screen_tabview_main_extra_btnm_items_default, 255);
    for(uint32_t i = 0; i < lv_tabview_get_tab_count(ui->screen_tabview_main); i++)
    {
        lv_obj_add_style(lv_obj_get_child(lv_tabview_get_tab_bar(ui->screen_tabview_main), i), &style_screen_tabview_main_extra_btnm_items_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    }

    //Write style state: LV_STATE_CHECKED for &style_screen_tabview_main_extra_btnm_items_checked
    static lv_style_t style_screen_tabview_main_extra_btnm_items_checked;
    ui_init_style(&style_screen_tabview_main_extra_btnm_items_checked);

    lv_style_set_text_color(&style_screen_tabview_main_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_text_font(&style_screen_tabview_main_extra_btnm_items_checked, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_screen_tabview_main_extra_btnm_items_checked, 255);
    lv_style_set_border_width(&style_screen_tabview_main_extra_btnm_items_checked, 4);
    lv_style_set_border_opa(&style_screen_tabview_main_extra_btnm_items_checked, 255);
    lv_style_set_border_color(&style_screen_tabview_main_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_border_side(&style_screen_tabview_main_extra_btnm_items_checked, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_screen_tabview_main_extra_btnm_items_checked, 0);
    lv_style_set_bg_opa(&style_screen_tabview_main_extra_btnm_items_checked, 60);
    lv_style_set_bg_color(&style_screen_tabview_main_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_bg_grad_dir(&style_screen_tabview_main_extra_btnm_items_checked, LV_GRAD_DIR_NONE);
    for(uint32_t i = 0; i < lv_tabview_get_tab_count(ui->screen_tabview_main); i++)
    {
        lv_obj_add_style(lv_obj_get_child(lv_tabview_get_tab_bar(ui->screen_tabview_main), i), &style_screen_tabview_main_extra_btnm_items_checked, LV_PART_MAIN|LV_STATE_CHECKED);
    }

    //Write codes Main
    lv_obj_t * screen_tabview_main_tab_1_label = lv_label_create(ui->screen_tabview_main_tab_1);
    lv_label_set_text(screen_tabview_main_tab_1_label, "");

    //Write codes HA
    lv_obj_t * screen_tabview_main_tab_2_label = lv_label_create(ui->screen_tabview_main_tab_2);
    lv_label_set_text(screen_tabview_main_tab_2_label, "");

    //Write codes  Setting
    lv_obj_t * screen_tabview_main_tab_3_label = lv_label_create(ui->screen_tabview_main_tab_3);
    lv_label_set_text(screen_tabview_main_tab_3_label, "");

    //Write codes screen_tabview_set
    ui->screen_tabview_set = lv_tabview_create(ui->screen_tabview_main_tab_1);
    lv_obj_set_pos(ui->screen_tabview_set, -5, -10);
    lv_obj_set_size(ui->screen_tabview_set, 305, 190);
    lv_obj_set_scrollbar_mode(ui->screen_tabview_set, LV_SCROLLBAR_MODE_OFF);
    lv_tabview_set_tab_bar_position(ui->screen_tabview_set, LV_DIR_LEFT);
    lv_tabview_set_tab_bar_size(ui->screen_tabview_set, 50);
    ui->screen_tabview_set_tab_1 = lv_tabview_add_tab(ui->screen_tabview_set, "WIFI");
    ui->screen_tabview_set_tab_2 = lv_tabview_add_tab(ui->screen_tabview_set, "Ent.");
    ui->screen_tabview_set_tab_3 = lv_tabview_add_tab(ui->screen_tabview_set, "OTA");
    ui->screen_tabview_set_tab_4 = lv_tabview_add_tab(ui->screen_tabview_set, "Else");

    //Write style for screen_tabview_set, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_tabview_set, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_tabview_set, lv_color_hex(0x00ffe0), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_tabview_set, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_tabview_set, lv_color_hex(0x4d4d4d), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_tabview_set, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_tabview_set, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_tabview_set, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_tabview_set, 16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_tabview_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tabview_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_tabview_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_tabview_set_extra_btnm_main_default
    static lv_style_t style_screen_tabview_set_extra_btnm_main_default;
    ui_init_style(&style_screen_tabview_set_extra_btnm_main_default);

    lv_style_set_bg_opa(&style_screen_tabview_set_extra_btnm_main_default, 255);
    lv_style_set_bg_color(&style_screen_tabview_set_extra_btnm_main_default, lv_color_hex(0x00fcff));
    lv_style_set_bg_grad_dir(&style_screen_tabview_set_extra_btnm_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_border_width(&style_screen_tabview_set_extra_btnm_main_default, 0);
    lv_style_set_radius(&style_screen_tabview_set_extra_btnm_main_default, 0);
    for(uint32_t i = 0; i < lv_tabview_get_tab_count(ui->screen_tabview_set); i++)
    {
        lv_obj_add_style(lv_obj_get_child(lv_tabview_get_tab_bar(ui->screen_tabview_set), i), &style_screen_tabview_set_extra_btnm_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    }

    //Write style state: LV_STATE_DEFAULT for &style_screen_tabview_set_extra_btnm_items_default
    static lv_style_t style_screen_tabview_set_extra_btnm_items_default;
    ui_init_style(&style_screen_tabview_set_extra_btnm_items_default);

    lv_style_set_text_color(&style_screen_tabview_set_extra_btnm_items_default, lv_color_hex(0x4d4d4d));
    lv_style_set_text_font(&style_screen_tabview_set_extra_btnm_items_default, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_screen_tabview_set_extra_btnm_items_default, 255);
    for(uint32_t i = 0; i < lv_tabview_get_tab_count(ui->screen_tabview_set); i++)
    {
        lv_obj_add_style(lv_obj_get_child(lv_tabview_get_tab_bar(ui->screen_tabview_set), i), &style_screen_tabview_set_extra_btnm_items_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    }

    //Write style state: LV_STATE_CHECKED for &style_screen_tabview_set_extra_btnm_items_checked
    static lv_style_t style_screen_tabview_set_extra_btnm_items_checked;
    ui_init_style(&style_screen_tabview_set_extra_btnm_items_checked);

    lv_style_set_text_color(&style_screen_tabview_set_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_text_font(&style_screen_tabview_set_extra_btnm_items_checked, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_screen_tabview_set_extra_btnm_items_checked, 255);
    lv_style_set_border_width(&style_screen_tabview_set_extra_btnm_items_checked, 4);
    lv_style_set_border_opa(&style_screen_tabview_set_extra_btnm_items_checked, 255);
    lv_style_set_border_color(&style_screen_tabview_set_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_border_side(&style_screen_tabview_set_extra_btnm_items_checked, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_screen_tabview_set_extra_btnm_items_checked, 0);
    lv_style_set_bg_opa(&style_screen_tabview_set_extra_btnm_items_checked, 60);
    lv_style_set_bg_color(&style_screen_tabview_set_extra_btnm_items_checked, lv_color_hex(0x2195f6));
    lv_style_set_bg_grad_dir(&style_screen_tabview_set_extra_btnm_items_checked, LV_GRAD_DIR_NONE);
    for(uint32_t i = 0; i < lv_tabview_get_tab_count(ui->screen_tabview_set); i++)
    {
        lv_obj_add_style(lv_obj_get_child(lv_tabview_get_tab_bar(ui->screen_tabview_set), i), &style_screen_tabview_set_extra_btnm_items_checked, LV_PART_MAIN|LV_STATE_CHECKED);
    }

    //Write codes WIFI
    lv_obj_t * screen_tabview_set_tab_1_label = lv_label_create(ui->screen_tabview_set_tab_1);
    lv_label_set_text(screen_tabview_set_tab_1_label, "");

    //Write codes screen_btn_APConfig
    ui->screen_btn_APConfig = lv_button_create(ui->screen_tabview_set_tab_1);
    lv_obj_set_pos(ui->screen_btn_APConfig, 20, 115);
    lv_obj_set_size(ui->screen_btn_APConfig, 180, 50);
    ui->screen_btn_APConfig_label = lv_label_create(ui->screen_btn_APConfig);
    lv_label_set_text(ui->screen_btn_APConfig_label, "AP Config Mode");
    lv_label_set_long_mode(ui->screen_btn_APConfig_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_btn_APConfig_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_btn_APConfig, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_btn_APConfig_label, LV_PCT(100));

    //Write style for screen_btn_APConfig, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_btn_APConfig, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_btn_APConfig, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_btn_APConfig, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_btn_APConfig, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_btn_APConfig, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_btn_APConfig, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_btn_APConfig, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_btn_APConfig, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_btn_APConfig, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_btn_APConfig, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Ent.
    lv_obj_t * screen_tabview_set_tab_2_label = lv_label_create(ui->screen_tabview_set_tab_2);
    lv_label_set_text(screen_tabview_set_tab_2_label, "");

    //Write codes screen_list_entity
    ui->screen_list_entity = lv_list_create(ui->screen_tabview_set_tab_2);
    lv_obj_set_pos(ui->screen_list_entity, 12, 4);
    lv_obj_set_size(ui->screen_list_entity, 198, 104);
    lv_obj_set_scrollbar_mode(ui->screen_list_entity, LV_SCROLLBAR_MODE_OFF);

    //Write style state: LV_STATE_DEFAULT for &style_screen_list_entity_main_main_default
    static lv_style_t style_screen_list_entity_main_main_default;
    ui_init_style(&style_screen_list_entity_main_main_default);

    lv_style_set_pad_top(&style_screen_list_entity_main_main_default, 5);
    lv_style_set_pad_left(&style_screen_list_entity_main_main_default, 5);
    lv_style_set_pad_right(&style_screen_list_entity_main_main_default, 5);
    lv_style_set_pad_bottom(&style_screen_list_entity_main_main_default, 5);
    lv_style_set_bg_opa(&style_screen_list_entity_main_main_default, 255);
    lv_style_set_bg_color(&style_screen_list_entity_main_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_screen_list_entity_main_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_border_width(&style_screen_list_entity_main_main_default, 1);
    lv_style_set_border_opa(&style_screen_list_entity_main_main_default, 255);
    lv_style_set_border_color(&style_screen_list_entity_main_main_default, lv_color_hex(0xe1e6ee));
    lv_style_set_border_side(&style_screen_list_entity_main_main_default, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_screen_list_entity_main_main_default, 3);
    lv_style_set_shadow_width(&style_screen_list_entity_main_main_default, 0);
    lv_obj_add_style(ui->screen_list_entity, &style_screen_list_entity_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_list_entity_main_scrollbar_default
    static lv_style_t style_screen_list_entity_main_scrollbar_default;
    ui_init_style(&style_screen_list_entity_main_scrollbar_default);

    lv_style_set_radius(&style_screen_list_entity_main_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_screen_list_entity_main_scrollbar_default, 255);
    lv_style_set_bg_color(&style_screen_list_entity_main_scrollbar_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_screen_list_entity_main_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(ui->screen_list_entity, &style_screen_list_entity_main_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_list_entity_extra_btns_main_default
    static lv_style_t style_screen_list_entity_extra_btns_main_default;
    ui_init_style(&style_screen_list_entity_extra_btns_main_default);

    lv_style_set_pad_top(&style_screen_list_entity_extra_btns_main_default, 5);
    lv_style_set_pad_left(&style_screen_list_entity_extra_btns_main_default, 5);
    lv_style_set_pad_right(&style_screen_list_entity_extra_btns_main_default, 5);
    lv_style_set_pad_bottom(&style_screen_list_entity_extra_btns_main_default, 5);
    lv_style_set_border_width(&style_screen_list_entity_extra_btns_main_default, 0);
    lv_style_set_text_color(&style_screen_list_entity_extra_btns_main_default, lv_color_hex(0x0D3055));
    lv_style_set_text_font(&style_screen_list_entity_extra_btns_main_default, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_screen_list_entity_extra_btns_main_default, 255);
    lv_style_set_radius(&style_screen_list_entity_extra_btns_main_default, 3);
    lv_style_set_bg_opa(&style_screen_list_entity_extra_btns_main_default, 255);
    lv_style_set_bg_color(&style_screen_list_entity_extra_btns_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_screen_list_entity_extra_btns_main_default, LV_GRAD_DIR_NONE);

    //Write style state: LV_STATE_DEFAULT for &style_screen_list_entity_extra_texts_main_default
    static lv_style_t style_screen_list_entity_extra_texts_main_default;
    ui_init_style(&style_screen_list_entity_extra_texts_main_default);

    lv_style_set_pad_top(&style_screen_list_entity_extra_texts_main_default, 5);
    lv_style_set_pad_left(&style_screen_list_entity_extra_texts_main_default, 5);
    lv_style_set_pad_right(&style_screen_list_entity_extra_texts_main_default, 5);
    lv_style_set_pad_bottom(&style_screen_list_entity_extra_texts_main_default, 5);
    lv_style_set_border_width(&style_screen_list_entity_extra_texts_main_default, 0);
    lv_style_set_text_color(&style_screen_list_entity_extra_texts_main_default, lv_color_hex(0x0D3055));
    lv_style_set_text_font(&style_screen_list_entity_extra_texts_main_default, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_screen_list_entity_extra_texts_main_default, 255);
    lv_style_set_radius(&style_screen_list_entity_extra_texts_main_default, 3);
    lv_style_set_transform_width(&style_screen_list_entity_extra_texts_main_default, 0);
    lv_style_set_bg_opa(&style_screen_list_entity_extra_texts_main_default, 255);
    lv_style_set_bg_color(&style_screen_list_entity_extra_texts_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_screen_list_entity_extra_texts_main_default, LV_GRAD_DIR_NONE);

    //Write codes screen_btn_3
    ui->screen_btn_3 = lv_button_create(ui->screen_tabview_set_tab_2);
    lv_obj_set_pos(ui->screen_btn_3, 20, 115);
    lv_obj_set_size(ui->screen_btn_3, 180, 50);
    ui->screen_btn_3_label = lv_label_create(ui->screen_btn_3);
    lv_label_set_text(ui->screen_btn_3_label, "Entity Search");
    lv_label_set_long_mode(ui->screen_btn_3_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_btn_3_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_btn_3, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_btn_3_label, LV_PCT(100));

    //Write style for screen_btn_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_btn_3, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_btn_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_btn_3, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_btn_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_btn_3, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_btn_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes OTA
    lv_obj_t * screen_tabview_set_tab_3_label = lv_label_create(ui->screen_tabview_set_tab_3);
    lv_label_set_text(screen_tabview_set_tab_3_label, "");

    //Write codes screen_btn_ota
    ui->screen_btn_ota = lv_button_create(ui->screen_tabview_set_tab_3);
    lv_obj_set_pos(ui->screen_btn_ota, 20, 115);
    lv_obj_set_size(ui->screen_btn_ota, 180, 50);
    ui->screen_btn_ota_label = lv_label_create(ui->screen_btn_ota);
    lv_label_set_text(ui->screen_btn_ota_label, "Update Firmware");
    lv_label_set_long_mode(ui->screen_btn_ota_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_btn_ota_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_btn_ota, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_btn_ota_label, LV_PCT(100));

    ///Write codes ver_info_cont
    lv_obj_t * ver_info_cont = lv_obj_create(ui->screen_tabview_set_tab_3);
    lv_obj_set_size(ver_info_cont, 200, 100);             // 设置框的大小
    lv_obj_set_pos(ver_info_cont, 10, 10);                // 设置在页面中的位置
    lv_obj_set_style_bg_color(ver_info_cont, lv_palette_main(LV_PALETTE_GREY), 0); // 背景色
    lv_obj_set_style_bg_opa(ver_info_cont, LV_OPA_10, 0); // 浅色背景
    lv_obj_set_style_border_width(ver_info_cont, 1, 0);   // 边框宽度
    lv_obj_set_style_radius(ver_info_cont, 8, 0);         // 圆角

    lv_obj_set_flex_flow(ver_info_cont, LV_FLEX_FLOW_COLUMN);// 设置容器内的布局为“弹性布局”（垂直排列），这样文字会自动对齐
    lv_obj_set_flex_align(ver_info_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(ver_info_cont, 10, 0);       // 内边距
     ui->label_status = lv_label_create(ver_info_cont);// 2. 创建状态标签 (例如：发现新版本 / 已是最新)
    lv_label_set_text(ui->label_status, "Status: New Update!");
    lv_obj_set_style_text_color(ui->label_status, lv_palette_main(LV_PALETTE_RED), 0); // 突出显示
    ui->label_curr_ver = lv_label_create(ver_info_cont);// 3. 创建当前版本标签
    lv_label_set_text(ui->label_curr_ver, "Current: v1.0.2");
     ui->label_new_ver = lv_label_create(ver_info_cont);    // 4. 创建最新版本标签
    lv_label_set_text(ui->label_new_ver, "Latest: v1.1.0");

    //Write style for screen_btn_ota, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_btn_ota, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_btn_ota, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_btn_ota, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_btn_ota, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_btn_ota, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_btn_ota, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_btn_ota, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_btn_ota, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_btn_ota, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_btn_ota, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Else
    lv_obj_t * screen_tabview_set_tab_4_label = lv_label_create(ui->screen_tabview_set_tab_4);
    lv_label_set_text(screen_tabview_set_tab_4_label, "text");

    //Write codes screen_digital_clock_1
    static bool screen_digital_clock_1_timer_enabled = false;
    ui->screen_digital_clock_1 = lv_label_create(ui->screen);
    lv_obj_set_pos(ui->screen_digital_clock_1, 0, 0);
    lv_obj_set_size(ui->screen_digital_clock_1, 49, 30);
    lv_label_set_text(ui->screen_digital_clock_1, "17:25");
    if (!screen_digital_clock_1_timer_enabled) {
        lv_timer_create(screen_digital_clock_1_timer, 1000, NULL);
        screen_digital_clock_1_timer_enabled = true;
    }

    //Write style for screen_digital_clock_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_digital_clock_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_digital_clock_1, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_digital_clock_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_digital_clock_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_digital_clock_1, 7, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_label_1
    ui->screen_label_1 = lv_label_create(ui->screen);
    lv_obj_set_pos(ui->screen_label_1, 249, 6);
    lv_obj_set_size(ui->screen_label_1, 65, 20);
    lv_label_set_text(ui->screen_label_1, "Sin Wifi");
    lv_label_set_long_mode(ui->screen_label_1, LV_LABEL_LONG_WRAP);

    //Write style for screen_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_1, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_1, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen);

    //Init events for screen.
    events_init_screen(ui);
}
