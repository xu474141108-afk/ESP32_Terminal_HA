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



void setup_scr_screen_HA(lv_ui *ui)
{
    //Write codes screen_HA
    ui->screen_HA = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_HA, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_HA, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_HA, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_HA, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_HA, lv_color_hex(0x0a304f), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_HA, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_HA_cont_HA_main
    ui->screen_HA_cont_HA_main = lv_obj_create(ui->screen_HA);
    lv_obj_set_pos(ui->screen_HA_cont_HA_main, 5, 48);
    lv_obj_set_size(ui->screen_HA_cont_HA_main, 310, 185);
    lv_obj_set_scrollbar_mode(ui->screen_HA_cont_HA_main, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_HA_cont_HA_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_HA_cont_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_HA_cont_HA_main, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_HA_cont_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_HA_cont_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_HA_cont_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_HA_cont_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_HA_cont_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_HA_cont_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_HA_list_HA_show
    ui->screen_HA_list_HA_show = lv_list_create(ui->screen_HA_cont_HA_main);
    lv_obj_set_pos(ui->screen_HA_list_HA_show, 10, 10);
    lv_obj_set_size(ui->screen_HA_list_HA_show, 290, 160);
    lv_obj_set_scrollbar_mode(ui->screen_HA_list_HA_show, LV_SCROLLBAR_MODE_OFF);
    ui->screen_HA_list_HA_show_item0 = lv_list_add_button(ui->screen_HA_list_HA_show, LV_SYMBOL_SAVE, "save_1");

    //Write style state: LV_STATE_DEFAULT for &style_screen_HA_list_HA_show_main_main_default
    static lv_style_t style_screen_HA_list_HA_show_main_main_default;
    ui_init_style(&style_screen_HA_list_HA_show_main_main_default);

    lv_style_set_pad_top(&style_screen_HA_list_HA_show_main_main_default, 5);
    lv_style_set_pad_left(&style_screen_HA_list_HA_show_main_main_default, 5);
    lv_style_set_pad_right(&style_screen_HA_list_HA_show_main_main_default, 5);
    lv_style_set_pad_bottom(&style_screen_HA_list_HA_show_main_main_default, 5);
    lv_style_set_bg_opa(&style_screen_HA_list_HA_show_main_main_default, 0);
    lv_style_set_border_width(&style_screen_HA_list_HA_show_main_main_default, 1);
    lv_style_set_border_opa(&style_screen_HA_list_HA_show_main_main_default, 111);
    lv_style_set_border_color(&style_screen_HA_list_HA_show_main_main_default, lv_color_hex(0xffffff));
    lv_style_set_border_side(&style_screen_HA_list_HA_show_main_main_default, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_screen_HA_list_HA_show_main_main_default, 3);
    lv_style_set_shadow_width(&style_screen_HA_list_HA_show_main_main_default, 0);
    lv_obj_add_style(ui->screen_HA_list_HA_show, &style_screen_HA_list_HA_show_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_HA_list_HA_show_main_scrollbar_default
    static lv_style_t style_screen_HA_list_HA_show_main_scrollbar_default;
    ui_init_style(&style_screen_HA_list_HA_show_main_scrollbar_default);

    lv_style_set_radius(&style_screen_HA_list_HA_show_main_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_screen_HA_list_HA_show_main_scrollbar_default, 255);
    lv_style_set_bg_color(&style_screen_HA_list_HA_show_main_scrollbar_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_screen_HA_list_HA_show_main_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(ui->screen_HA_list_HA_show, &style_screen_HA_list_HA_show_main_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_HA_list_HA_show_extra_btns_main_default
    static lv_style_t style_screen_HA_list_HA_show_extra_btns_main_default;
    ui_init_style(&style_screen_HA_list_HA_show_extra_btns_main_default);

    lv_style_set_pad_top(&style_screen_HA_list_HA_show_extra_btns_main_default, 5);
    lv_style_set_pad_left(&style_screen_HA_list_HA_show_extra_btns_main_default, 5);
    lv_style_set_pad_right(&style_screen_HA_list_HA_show_extra_btns_main_default, 5);
    lv_style_set_pad_bottom(&style_screen_HA_list_HA_show_extra_btns_main_default, 5);
    lv_style_set_border_width(&style_screen_HA_list_HA_show_extra_btns_main_default, 0);
    lv_style_set_text_color(&style_screen_HA_list_HA_show_extra_btns_main_default, lv_color_hex(0x0D3055));
    lv_style_set_text_font(&style_screen_HA_list_HA_show_extra_btns_main_default, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_screen_HA_list_HA_show_extra_btns_main_default, 255);
    lv_style_set_radius(&style_screen_HA_list_HA_show_extra_btns_main_default, 3);
    lv_style_set_bg_opa(&style_screen_HA_list_HA_show_extra_btns_main_default, 255);
    lv_style_set_bg_color(&style_screen_HA_list_HA_show_extra_btns_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_screen_HA_list_HA_show_extra_btns_main_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(ui->screen_HA_list_HA_show_item0, &style_screen_HA_list_HA_show_extra_btns_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_HA_list_HA_show_extra_texts_main_default
    static lv_style_t style_screen_HA_list_HA_show_extra_texts_main_default;
    ui_init_style(&style_screen_HA_list_HA_show_extra_texts_main_default);

    lv_style_set_pad_top(&style_screen_HA_list_HA_show_extra_texts_main_default, 5);
    lv_style_set_pad_left(&style_screen_HA_list_HA_show_extra_texts_main_default, 5);
    lv_style_set_pad_right(&style_screen_HA_list_HA_show_extra_texts_main_default, 5);
    lv_style_set_pad_bottom(&style_screen_HA_list_HA_show_extra_texts_main_default, 5);
    lv_style_set_border_width(&style_screen_HA_list_HA_show_extra_texts_main_default, 0);
    lv_style_set_text_color(&style_screen_HA_list_HA_show_extra_texts_main_default, lv_color_hex(0x0D3055));
    lv_style_set_text_font(&style_screen_HA_list_HA_show_extra_texts_main_default, &lv_font_montserratMedium_12);
    lv_style_set_text_opa(&style_screen_HA_list_HA_show_extra_texts_main_default, 255);
    lv_style_set_radius(&style_screen_HA_list_HA_show_extra_texts_main_default, 3);
    lv_style_set_transform_width(&style_screen_HA_list_HA_show_extra_texts_main_default, 0);
    lv_style_set_bg_opa(&style_screen_HA_list_HA_show_extra_texts_main_default, 255);
    lv_style_set_bg_color(&style_screen_HA_list_HA_show_extra_texts_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_screen_HA_list_HA_show_extra_texts_main_default, LV_GRAD_DIR_NONE);

    //Write codes screen_HA_cont_HA_top
    ui->screen_HA_cont_HA_top = lv_obj_create(ui->screen_HA);
    lv_obj_set_pos(ui->screen_HA_cont_HA_top, 5, 5);
    lv_obj_set_size(ui->screen_HA_cont_HA_top, 310, 40);
    lv_obj_set_scrollbar_mode(ui->screen_HA_cont_HA_top, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_HA_cont_HA_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_HA_cont_HA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_HA_cont_HA_top, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_HA_cont_HA_top, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_HA_cont_HA_top, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_HA_cont_HA_top, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_HA_cont_HA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_HA_cont_HA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_HA_cont_HA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_HA_cont_HA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_HA_cont_HA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_HA_btn_HA_check
    ui->screen_HA_btn_HA_check = lv_button_create(ui->screen_HA_cont_HA_top);
    lv_obj_set_pos(ui->screen_HA_btn_HA_check, 120, 5);
    lv_obj_set_size(ui->screen_HA_btn_HA_check, 180, 30);
    ui->screen_HA_btn_HA_check_label = lv_label_create(ui->screen_HA_btn_HA_check);
    lv_label_set_text(ui->screen_HA_btn_HA_check_label, "HA Checking");
    lv_label_set_long_mode(ui->screen_HA_btn_HA_check_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_HA_btn_HA_check_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_HA_btn_HA_check, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_HA_btn_HA_check_label, LV_PCT(100));

    //Write style for screen_HA_btn_HA_check, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_HA_btn_HA_check, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_HA_btn_HA_check, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_HA_btn_HA_check, LV_GRAD_DIR_VER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->screen_HA_btn_HA_check, lv_color_hex(0x104b7c), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui->screen_HA_btn_HA_check, 29, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui->screen_HA_btn_HA_check, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_HA_btn_HA_check, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_HA_btn_HA_check, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_HA_btn_HA_check, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_HA_btn_HA_check, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_HA_btn_HA_check, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_HA_btn_HA_check, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_HA_btn_HA_check, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_HA_label_HA_main
    ui->screen_HA_label_HA_main = lv_label_create(ui->screen_HA_cont_HA_top);
    lv_obj_set_pos(ui->screen_HA_label_HA_main, 50, 10);
    lv_obj_set_size(ui->screen_HA_label_HA_main, 50, 20);
    lv_label_set_text(ui->screen_HA_label_HA_main, "HA");
    lv_label_set_long_mode(ui->screen_HA_label_HA_main, LV_LABEL_LONG_WRAP);

    //Write style for screen_HA_label_HA_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_HA_label_HA_main, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_HA_label_HA_main, &lv_font_montserratMedium_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_HA_label_HA_main, 230, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_HA_label_HA_main, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_HA_label_HA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_HA_btn_HA_back
    ui->screen_HA_btn_HA_back = lv_button_create(ui->screen_HA_cont_HA_top);
    lv_obj_set_pos(ui->screen_HA_btn_HA_back, 5, 5);
    lv_obj_set_size(ui->screen_HA_btn_HA_back, 30, 30);
    ui->screen_HA_btn_HA_back_label = lv_label_create(ui->screen_HA_btn_HA_back);
    lv_label_set_text(ui->screen_HA_btn_HA_back_label, "←");
    lv_label_set_long_mode(ui->screen_HA_btn_HA_back_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_HA_btn_HA_back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_HA_btn_HA_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_HA_btn_HA_back_label, LV_PCT(100));

    //Write style for screen_HA_btn_HA_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_HA_btn_HA_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_HA_btn_HA_back, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_HA_btn_HA_back, LV_GRAD_DIR_VER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->screen_HA_btn_HA_back, lv_color_hex(0x104b7c), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui->screen_HA_btn_HA_back, 29, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui->screen_HA_btn_HA_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_HA_btn_HA_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_HA_btn_HA_back, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_HA_btn_HA_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_HA_btn_HA_back, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_HA_btn_HA_back, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_HA_btn_HA_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_HA_btn_HA_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_HA.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_HA);

    //Init events for screen.
    events_init_screen_HA(ui);
}
