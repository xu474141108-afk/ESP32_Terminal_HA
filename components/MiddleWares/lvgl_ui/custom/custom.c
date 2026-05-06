#include <stdio.h>
#include "lvgl.h"
#include "custom.h"
#include "esp_log.h"
#include "ha_http_control.h"


/**********************
 *  HA new button set
 **********************/
#define TAG "Custom"

static void HA_button_action(lv_event_t * e)
{
    ESP_LOGI(TAG, "执行设备操作");
}

static void HA_button_event_cb(lv_event_t * e)
{

    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * mbox = (lv_obj_t *)lv_event_get_user_data(e);
    
    const char * txt = lv_label_get_text(lv_obj_get_child(btn, 0));

    if (strcmp(txt, "Sure") == 0) {
        intptr_t index = (intptr_t)lv_obj_get_user_data(mbox); 
        ESP_LOGI(TAG, "用户确认添加设备 %s", g_device_list[index].friendly_name);

        lv_obj_t * new_btn = lv_btn_create(guider_ui.screen_tabview_main_tab_2);
        
        lv_obj_set_size(new_btn, 90, 40);

        lv_obj_t * label = lv_label_create(new_btn);
        lv_label_set_text(label, g_device_list[index].friendly_name);
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_obj_set_style_anim_duration(label, 4000, LV_PART_MAIN);
        lv_obj_set_width(label, 75);
        lv_obj_center(label);

        lv_obj_add_event_cb(new_btn, HA_button_action, LV_EVENT_CLICKED, (void *)index);
        // save_devices_to_nvs(); 
    }
    lv_msgbox_close(mbox);
}

static void HA_button_event_register(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);       
    int index = (intptr_t)lv_obj_get_user_data(obj);
    ha_device_t * devices = (ha_device_t *)lv_event_get_user_data(e);

    if (devices) {
        ESP_LOGI("UI_EVENT", "点击了: %s, 状态: %s", 
                 devices[index].friendly_name, 
                 devices[index].state ? "ON" : "OFF");

        lv_obj_t * mbox = lv_msgbox_create(NULL);
    
        lv_obj_set_user_data(mbox, (void *)index);
        lv_msgbox_add_title(mbox, "To sure");
        lv_msgbox_add_text(mbox, "Are you sure?");

        lv_obj_t * btn_confirm = lv_msgbox_add_footer_button(mbox, "Sure");
        lv_obj_t * btn_cancel = lv_msgbox_add_footer_button(mbox, "Cancel");
        lv_obj_set_user_data(mbox, (void *)index);

        lv_obj_add_event_cb(btn_confirm, HA_button_event_cb, LV_EVENT_CLICKED, mbox);
        lv_obj_add_event_cb(btn_cancel, HA_button_event_cb, LV_EVENT_CLICKED, mbox);
    }
}


void HA_json_to_list(lv_obj_t *list_obj, ha_device_t *devices, int count) 
{
    if (!list_obj || !devices) return;

    lv_obj_clean(list_obj); // 清空旧UI
    ESP_LOGI(TAG, "正在将 %d 个设备添加到 UI 列表...", count);

    for (int i = 0; i < count; i++) {
        lv_obj_t * btn = lv_list_add_btn(guider_ui.screen_list_entity, 
                                        LV_SYMBOL_SETTINGS, 
                                        g_device_list[i].friendly_name);


        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        lv_obj_add_event_cb(btn, HA_button_event_register, LV_EVENT_CLICKED, devices); 
    }
}









void custom_init(lv_ui *ui)
{
    /* Add your codes here */
}

