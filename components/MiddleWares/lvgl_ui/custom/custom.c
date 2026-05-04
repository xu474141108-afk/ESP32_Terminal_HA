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
    // 获取被点击的按钮对象
    lv_obj_t * btn = lv_event_get_target(e);
    // 从事件参数中获取弹窗对象（由我们在 add_event_cb 时作为 user_data 传入）
    lv_obj_t * mbox = (lv_obj_t *)lv_event_get_user_data(e);
    
    // 获取按钮上的文本
    const char * txt = lv_label_get_text(lv_obj_get_child(btn, 0));

    if (strcmp(txt, "Sure") == 0) {
        intptr_t index = (intptr_t)lv_obj_get_user_data(mbox); // 从弹窗中取回索引
        ESP_LOGI(TAG, "用户确认添加设备 %s", g_device_list[index].friendly_name);

        // --- 动态创建一个新按钮 ---
        lv_obj_t * new_btn = lv_btn_create(guider_ui.screen_tabview_main_tab_2);
        
        lv_obj_set_size(new_btn, 90, 40);

        // 给新按钮添加文字标签
        lv_obj_t * label = lv_label_create(new_btn);
        lv_label_set_text(label, g_device_list[index].friendly_name);
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_obj_set_style_anim_duration(label, 4000, LV_PART_MAIN);
        lv_obj_set_width(label, 75);
        lv_obj_center(label);

        // --- 为这个“动态生成的按钮”绑定功能 ---
        lv_obj_add_event_cb(new_btn, HA_button_action, LV_EVENT_CLICKED, (void *)index);
        // save_devices_to_nvs(); 
    }

    // 关闭删除弹窗
    lv_msgbox_close(mbox);
}

static void HA_button_event_register(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);        // 获取触发事件的列表按钮
    int index = (intptr_t)lv_obj_get_user_data(obj);
    ha_device_t * devices = (ha_device_t *)lv_event_get_user_data(e);

    if (devices) {
        // 现在我们通过局部指针访问数据，完全不依赖外部的 g_device_list 名字
        ESP_LOGI("UI_EVENT", "点击了: %s, 状态: %s", 
                 devices[index].friendly_name, 
                 devices[index].state ? "ON" : "OFF");

        // 创建弹窗
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


/**
 * @param list_obj 目标列表控件
 * @param devices  设备数组的首地址
 * @param count    要显示的设备数量
 */
void HA_json_to_list(lv_obj_t *list_obj, ha_device_t *devices, int count) 
{
    if (!list_obj || !devices) return;

    lv_obj_clean(list_obj); // 清空旧 UI 元素
    ESP_LOGI(TAG, "正在将 %d 个设备添加到 UI 列表...", count);

    for (int i = 0; i < count; i++) {
        lv_obj_t * btn = lv_list_add_btn(guider_ui.screen_list_entity, 
                                        LV_SYMBOL_SETTINGS, 
                                        g_device_list[i].friendly_name);


        // 关键点：将索引 i 绑定到按钮
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        // 绑定回调
        lv_obj_add_event_cb(btn, HA_button_event_register, LV_EVENT_CLICKED, devices); 
    }
}









void custom_init(lv_ui *ui)
{
    /* Add your codes here */
}

