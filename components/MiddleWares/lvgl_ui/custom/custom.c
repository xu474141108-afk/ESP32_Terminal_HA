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
    if(mbox){
        lv_msgbox_close(mbox);
    }

    ESP_LOGI(TAG, "用户确认更新，启动下载任务...");
    //xTaskCreate(write_ota_data, "write_ota_data", 8192, NULL, 5, NULL);
}

static void btn_cancel_update_event_cb(lv_event_t * e)
{ 
    lv_obj_t * mbox = lv_event_get_user_data(e);
    g_ota_ctx.state = OTA_STATE_IDLE;
    ESP_LOGI(TAG, "用户取消更新 %s", lv_label_get_text(mbox));
    if(mbox){
        lv_msgbox_close(mbox);
    }

}

void task_OTA_state_monitor()
{
    lv_obj_t *mbox = NULL;
    lv_obj_t *checking_mbox = NULL;
    switch (g_ota_ctx.state)
    {
        case OTA_STATE_IDLE:
            break;

        case OTA_STATE_CHECKING:  
            ESP_LOGI(TAG, "正在检查新版本...");
            if(checking_mbox == NULL){
                checking_mbox = lv_msgbox_create(NULL);
                lv_msgbox_add_title(checking_mbox, "OTA");
                lv_msgbox_add_text(checking_mbox,  "Checking for new version...");
            }
            break;

        case OTA_STATE_READY:
            if(checking_mbox){
                lv_msgbox_close(checking_mbox);
                checking_mbox = NULL;
            }
            ESP_LOGI(TAG, "有新版本");
            if (mbox == NULL)
            {
                mbox = lv_msgbox_create(NULL);
                lv_msgbox_add_title(mbox, "OTA");
                lv_msgbox_add_text(mbox,  "New version available");
                // 添加按钮        
                lv_obj_t * btn_v_ud = lv_msgbox_add_footer_button(mbox, "Update");
                lv_obj_add_event_cb(btn_v_ud, btn_start_update_event_cb, LV_EVENT_CLICKED, mbox);
                lv_obj_t * btn_v_cc = lv_msgbox_add_footer_button(mbox, "Cancel");
                lv_obj_add_event_cb(btn_v_cc, btn_cancel_update_event_cb, LV_EVENT_CLICKED, mbox);
            }
        break;           
            
        case OTA_STATE_NO_NEW:
            ESP_LOGI(TAG, "无新版本");
            if(checking_mbox){
                lv_msgbox_close(checking_mbox);
                checking_mbox = NULL;
            }
            mbox = lv_msgbox_create(NULL);
            
            lv_msgbox_add_title(mbox, "OTA");
            lv_msgbox_add_text(mbox,  "No new version.");
            lv_obj_t * btn_n_cc = lv_msgbox_add_footer_button(mbox, "Cancel");
            lv_obj_add_event_cb(btn_n_cc, btn_cancel_update_event_cb, LV_EVENT_CLICKED, mbox);
            break;

        case OTA_STATE_HTTP_ERROR:
            ESP_LOGE(TAG, "检查新版本时发生HTTP错误");
            if(checking_mbox){
                lv_msgbox_close(checking_mbox);
                checking_mbox = NULL;
            }
            g_ota_ctx.state = OTA_STATE_IDLE;
            mbox = lv_msgbox_create(NULL);
            lv_msgbox_add_title(mbox, "ERROR");
            lv_msgbox_add_text(mbox,  "HTTP Failed.");      
            lv_obj_t * btn_h_cc = lv_msgbox_add_footer_button(mbox, "Cancel");
            lv_obj_add_event_cb(btn_h_cc, btn_cancel_update_event_cb, LV_EVENT_CLICKED, mbox);
            break;

        default:
            if(checking_mbox){
                lv_msgbox_close(checking_mbox);
                checking_mbox = NULL;
            }
            ESP_LOGE(TAG, "未知的OTA状态");
            g_ota_ctx.state = OTA_STATE_IDLE;
            mbox = lv_msgbox_create(NULL);
            lv_msgbox_add_title(mbox, "ERROR");
            lv_msgbox_add_text(mbox,  "Unknown OTA status.");     
            lv_obj_t * btn_e_cc = lv_msgbox_add_footer_button(mbox, "Cancel");
            lv_obj_add_event_cb(btn_e_cc, btn_cancel_update_event_cb, LV_EVENT_CLICKED, mbox);
            break;
    }
}



void custom_init(lv_ui *ui)
{
    /* Add your codes here */
}

