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



void setup_scr_screen_System(lv_ui *ui)
{
    //Write codes screen_System
    ui->screen_System = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_System, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_System, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_System, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_System, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_System, lv_color_hex(0x0a304f), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_System, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_System_cont_setup_main
    ui->screen_System_cont_setup_main = lv_obj_create(ui->screen_System);
    lv_obj_set_pos(ui->screen_System_cont_setup_main, 5, 49);
    lv_obj_set_size(ui->screen_System_cont_setup_main, 310, 185);
    lv_obj_set_scrollbar_mode(ui->screen_System_cont_setup_main, LV_SCROLLBAR_MODE_ACTIVE);

    //Write style for screen_System_cont_setup_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_System_cont_setup_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_System_cont_setup_main, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_System_cont_setup_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_System_cont_setup_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_System_cont_setup_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_System_cont_setup_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_System_cont_setup_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_System_cont_setup_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_System_cont_setup_top
    ui->screen_System_cont_setup_top = lv_obj_create(ui->screen_System);
    lv_obj_set_pos(ui->screen_System_cont_setup_top, 5, 5);
    lv_obj_set_size(ui->screen_System_cont_setup_top, 310, 40);
    lv_obj_set_scrollbar_mode(ui->screen_System_cont_setup_top, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_System_cont_setup_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_System_cont_setup_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_System_cont_setup_top, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_System_cont_setup_top, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_System_cont_setup_top, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_System_cont_setup_top, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_System_cont_setup_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_System_cont_setup_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_System_cont_setup_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_System_cont_setup_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_System_cont_setup_top, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_System_label_System_main
    ui->screen_System_label_System_main = lv_label_create(ui->screen_System_cont_setup_top);
    lv_obj_set_pos(ui->screen_System_label_System_main, 50, 10);
    lv_obj_set_size(ui->screen_System_label_System_main, 70, 20);
    lv_label_set_text(ui->screen_System_label_System_main, "System");
    lv_label_set_long_mode(ui->screen_System_label_System_main, LV_LABEL_LONG_WRAP);

    //Write style for screen_System_label_System_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_System_label_System_main, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_System_label_System_main, &lv_font_montserratMedium_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_System_label_System_main, 230, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_System_label_System_main, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_System_label_System_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_System_btn_System_back
    ui->screen_System_btn_System_back = lv_button_create(ui->screen_System_cont_setup_top);
    lv_obj_set_pos(ui->screen_System_btn_System_back, 5, 5);
    lv_obj_set_size(ui->screen_System_btn_System_back, 30, 30);
    ui->screen_System_btn_System_back_label = lv_label_create(ui->screen_System_btn_System_back);
    lv_label_set_text(ui->screen_System_btn_System_back_label, "←");
    lv_label_set_long_mode(ui->screen_System_btn_System_back_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_System_btn_System_back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_System_btn_System_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_System_btn_System_back_label, LV_PCT(100));

    //Write style for screen_System_btn_System_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_System_btn_System_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_System_btn_System_back, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_System_btn_System_back, LV_GRAD_DIR_VER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->screen_System_btn_System_back, lv_color_hex(0x104b7c), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui->screen_System_btn_System_back, 29, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui->screen_System_btn_System_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_System_btn_System_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_System_btn_System_back, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_System_btn_System_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_System_btn_System_back, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_System_btn_System_back, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_System_btn_System_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_System_btn_System_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_System.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_System);

    //Init events for screen.
    events_init_screen_System(ui);
}
