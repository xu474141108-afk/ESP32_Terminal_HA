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



void setup_scr_screen_OTA(lv_ui *ui)
{
    //Write codes screen_OTA
    ui->screen_OTA = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_OTA, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_OTA, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_OTA, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_OTA, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_OTA, lv_color_hex(0x0a304f), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_OTA, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_OTA_cont_OTA_main
    ui->screen_OTA_cont_OTA_main = lv_obj_create(ui->screen_OTA);
    lv_obj_set_pos(ui->screen_OTA_cont_OTA_main, 5, 48);
    lv_obj_set_size(ui->screen_OTA_cont_OTA_main, 310, 185);
    lv_obj_set_scrollbar_mode(ui->screen_OTA_cont_OTA_main, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_OTA_cont_OTA_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_OTA_cont_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_OTA_cont_OTA_main, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_OTA_cont_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_OTA_cont_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_OTA_cont_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_OTA_cont_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_OTA_cont_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_OTA_cont_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_OTA_animimg_OTA
    ui->screen_OTA_animimg_OTA = lv_animimg_create(ui->screen_OTA_cont_OTA_main);
    lv_obj_set_pos(ui->screen_OTA_animimg_OTA, 13, 29);
    lv_obj_set_size(ui->screen_OTA_animimg_OTA, 100, 100);
    lv_animimg_set_src(ui->screen_OTA_animimg_OTA, (const void **) screen_OTA_animimg_OTA_imgs, 0);
    lv_animimg_set_duration(ui->screen_OTA_animimg_OTA, 30*0);
    lv_animimg_set_repeat_count(ui->screen_OTA_animimg_OTA, LV_ANIM_REPEAT_INFINITE);
    lv_image_set_src(ui->screen_OTA_animimg_OTA, screen_OTA_animimg_OTA_imgs[0]);

    //Write codes screen_OTA_label_OTA_info2
    ui->screen_OTA_label_OTA_info2 = lv_label_create(ui->screen_OTA_cont_OTA_main);
    lv_obj_set_pos(ui->screen_OTA_label_OTA_info2, 120, 110);
    lv_obj_set_size(ui->screen_OTA_label_OTA_info2, 180, 22);
    lv_label_set_text(ui->screen_OTA_label_OTA_info2, "Latest: ");
    lv_label_set_long_mode(ui->screen_OTA_label_OTA_info2, LV_LABEL_LONG_WRAP);

    //Write style for screen_OTA_label_OTA_info2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_OTA_label_OTA_info2, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_OTA_label_OTA_info2, 177, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_OTA_label_OTA_info2, lv_color_hex(0x2F92DA), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_OTA_label_OTA_info2, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_OTA_label_OTA_info2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_OTA_label_OTA_info2, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_OTA_label_OTA_info2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_OTA_label_OTA_info2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_OTA_label_OTA_info2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_OTA_label_OTA_info1
    ui->screen_OTA_label_OTA_info1 = lv_label_create(ui->screen_OTA_cont_OTA_main);
    lv_obj_set_pos(ui->screen_OTA_label_OTA_info1, 120, 70);
    lv_obj_set_size(ui->screen_OTA_label_OTA_info1, 180, 22);
    lv_label_set_text(ui->screen_OTA_label_OTA_info1, "Current:");
    lv_label_set_long_mode(ui->screen_OTA_label_OTA_info1, LV_LABEL_LONG_WRAP);

    //Write style for screen_OTA_label_OTA_info1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_OTA_label_OTA_info1, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_OTA_label_OTA_info1, 177, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_OTA_label_OTA_info1, lv_color_hex(0x2F92DA), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_OTA_label_OTA_info1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_OTA_label_OTA_info1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_OTA_label_OTA_info1, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_OTA_label_OTA_info1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_OTA_label_OTA_info1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_OTA_label_OTA_info1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_OTA_label_OTA_state
    ui->screen_OTA_label_OTA_state = lv_label_create(ui->screen_OTA_cont_OTA_main);
    lv_obj_set_pos(ui->screen_OTA_label_OTA_state, 120, 30);
    lv_obj_set_size(ui->screen_OTA_label_OTA_state, 180, 22);
    lv_label_set_text(ui->screen_OTA_label_OTA_state, "State:");
    lv_label_set_long_mode(ui->screen_OTA_label_OTA_state, LV_LABEL_LONG_WRAP);

    //Write style for screen_OTA_label_OTA_state, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_OTA_label_OTA_state, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_OTA_label_OTA_state, 177, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_OTA_label_OTA_state, lv_color_hex(0x2F92DA), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_OTA_label_OTA_state, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_OTA_label_OTA_state, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_OTA_label_OTA_state, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_OTA_label_OTA_state, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_OTA_label_OTA_state, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_OTA_label_OTA_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_OTA_btn_OTA_download
    ui->screen_OTA_btn_OTA_download = lv_button_create(ui->screen_OTA_cont_OTA_main);
    lv_obj_set_pos(ui->screen_OTA_btn_OTA_download, 71, 147);
    lv_obj_set_size(ui->screen_OTA_btn_OTA_download, 180, 30);
    ui->screen_OTA_btn_OTA_download_label = lv_label_create(ui->screen_OTA_btn_OTA_download);
    lv_label_set_text(ui->screen_OTA_btn_OTA_download_label, "Download");
    lv_label_set_long_mode(ui->screen_OTA_btn_OTA_download_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_OTA_btn_OTA_download_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_OTA_btn_OTA_download, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_OTA_btn_OTA_download_label, LV_PCT(100));

    //Write style for screen_OTA_btn_OTA_download, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_OTA_btn_OTA_download, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_OTA_btn_OTA_download, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_OTA_btn_OTA_download, LV_GRAD_DIR_VER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->screen_OTA_btn_OTA_download, lv_color_hex(0x104b7c), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui->screen_OTA_btn_OTA_download, 29, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui->screen_OTA_btn_OTA_download, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_OTA_btn_OTA_download, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_OTA_btn_OTA_download, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_OTA_btn_OTA_download, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_OTA_btn_OTA_download, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_OTA_btn_OTA_download, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_OTA_btn_OTA_download, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_OTA_btn_OTA_download, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_OTA_cont_OTA_top
    ui->screen_OTA_cont_OTA_top = lv_obj_create(ui->screen_OTA);
    lv_obj_set_pos(ui->screen_OTA_cont_OTA_top, 5, 5);
    lv_obj_set_size(ui->screen_OTA_cont_OTA_top, 310, 40);
    lv_obj_set_scrollbar_mode(ui->screen_OTA_cont_OTA_top, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_OTA_cont_OTA_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_OTA_cont_OTA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_OTA_cont_OTA_top, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_OTA_cont_OTA_top, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_OTA_cont_OTA_top, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_OTA_cont_OTA_top, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_OTA_cont_OTA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_OTA_cont_OTA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_OTA_cont_OTA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_OTA_cont_OTA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_OTA_cont_OTA_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_OTA_label_OTA_main
    ui->screen_OTA_label_OTA_main = lv_label_create(ui->screen_OTA_cont_OTA_top);
    lv_obj_set_pos(ui->screen_OTA_label_OTA_main, 50, 10);
    lv_obj_set_size(ui->screen_OTA_label_OTA_main, 50, 20);
    lv_label_set_text(ui->screen_OTA_label_OTA_main, "OTA");
    lv_label_set_long_mode(ui->screen_OTA_label_OTA_main, LV_LABEL_LONG_WRAP);

    //Write style for screen_OTA_label_OTA_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_OTA_label_OTA_main, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_OTA_label_OTA_main, &lv_font_montserratMedium_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_OTA_label_OTA_main, 230, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_OTA_label_OTA_main, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_OTA_label_OTA_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_OTA_btn_OTA_back
    ui->screen_OTA_btn_OTA_back = lv_button_create(ui->screen_OTA_cont_OTA_top);
    lv_obj_set_pos(ui->screen_OTA_btn_OTA_back, 5, 5);
    lv_obj_set_size(ui->screen_OTA_btn_OTA_back, 30, 30);
    ui->screen_OTA_btn_OTA_back_label = lv_label_create(ui->screen_OTA_btn_OTA_back);
    lv_label_set_text(ui->screen_OTA_btn_OTA_back_label, "←");
    lv_label_set_long_mode(ui->screen_OTA_btn_OTA_back_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_OTA_btn_OTA_back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_OTA_btn_OTA_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_OTA_btn_OTA_back_label, LV_PCT(100));

    //Write style for screen_OTA_btn_OTA_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_OTA_btn_OTA_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_OTA_btn_OTA_back, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_OTA_btn_OTA_back, LV_GRAD_DIR_VER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->screen_OTA_btn_OTA_back, lv_color_hex(0x104b7c), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui->screen_OTA_btn_OTA_back, 29, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui->screen_OTA_btn_OTA_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_OTA_btn_OTA_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_OTA_btn_OTA_back, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_OTA_btn_OTA_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_OTA_btn_OTA_back, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_OTA_btn_OTA_back, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_OTA_btn_OTA_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_OTA_btn_OTA_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_OTA.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_OTA);

    //Init events for screen.
    events_init_screen_OTA(ui);
}
