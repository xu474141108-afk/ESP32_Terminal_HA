#include <stdio.h>
#include "lvgl.h"
#include "custom.h"
#include "esp_log.h"
#include "ha_http_control.h"
#include "OTA.h"

/**********************
 *  HA new button set
 **********************/
#define TAG "Custom"
// HA设备操作界面
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

//OTA升级界面


static void btn_start_update_event_cb(lv_event_t * e)
{
    lv_obj_t * mbox = lv_event_get_user_data(e);
    g_ota_ctx.state = OTA_STATE_DOWNLOADING;
    if(mbox){
        lv_msgbox_close(mbox);
    }

    ESP_LOGI(TAG, "用户确认更新，启动下载任务...");
    //xTaskCreate(write_ota_data, "write_ota_data", 8192, NULL, 5, NULL);
}



static void btn_cancel_update_event_cb(lv_event_t * e)
{ 
    lv_obj_t * mbox = lv_event_get_user_data(e);
    
    
    if(mbox){
        lv_obj_t * content_obj = lv_msgbox_get_content(mbox);
        if (content_obj) {
            ESP_LOGI(TAG, "用户取消更新，提示内容为: %s", lv_label_get_text(lv_obj_get_child(content_obj, 0)));
        }
        lv_msgbox_close(mbox);
    }
    g_ota_ctx.state = OTA_STATE_IDLE;
}

void task_OTA_state_monitor(lv_timer_t * timer)
{
    static lv_obj_t *mbox = NULL;
    if (!lv_obj_is_valid(mbox)) {
        mbox = NULL;
    }

    if(guider_ui.label_status) {
        switch (g_ota_ctx.state)
        {
            case OTA_STATE_IDLE:
                break;

            case OTA_STATE_CHECKING:  
                break;

            case OTA_STATE_READY:
                lv_label_set_text(guider_ui.label_status, "Status: New Update!");
                break;

            case OTA_STATE_NO_NEW:
                lv_label_set_text(guider_ui.label_status, "Status: No new version");
                break;

            case OTA_STATE_HTTP_ERROR:
                lv_label_set_text(guider_ui.label_status, "Status: HTTP error");
                break;

            case OTA_STATE_DOWNLOADING:
                lv_label_set_text(guider_ui.label_status, "Status: Downloading update...");
                break;

            case OTA_STATE_SUCCESS :
                lv_label_set_text(guider_ui.label_status, "Status: Update successful! Restarting...");
                break;

            case OTA_STATE_FAILED:
                lv_label_set_text(guider_ui.label_status, "Status: Update failed");
                break;

            case OTA_STATE_LEN_NOFIT:
                lv_label_set_text(guider_ui.label_status, "Status: Received data mismatch");
                break;

            default:
                break;
        }
    }
    if(guider_ui.label_curr_ver&&guider_ui.label_new_ver) {
        lv_label_set_text_fmt(guider_ui.label_curr_ver, "Current: %s", g_ota_ctx.current_ver);
        lv_label_set_text_fmt(guider_ui.label_new_ver, "Latest: %s", g_ota_ctx.latest_ver);
    }
    
}



void custom_init(lv_ui *ui)
{
    /* Add your codes here */
}

