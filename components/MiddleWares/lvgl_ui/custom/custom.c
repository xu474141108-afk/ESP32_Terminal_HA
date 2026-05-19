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



static void HA_json_to_list(lv_obj_t *list_obj, ha_device_t *devices) ;
static void HA_button_event_register(lv_event_t * e);
static void HA_button_event_cb(lv_event_t * e) ;

void task_HA_state_monitor(lv_timer_t * timer)
{
    // static lv_obj_t *mbox = NULL;
    // if (!lv_obj_is_valid(mbox)) {
    //     mbox = NULL;
    // }
    
    // switch (g_HAdevice_ctx.state_ha)
    // {
    //     case HA_STATE_IDLE:
    //         break;

    //     case HA_STATE_CHECKING:  
    //         break;

    //     // util checked
    //     case HA_STATE_READY:
    //         ESP_LOGI(TAG, "HA设备状态: READY，准备将设备添加到 UI 列表");
    //         HA_json_to_list(guider_ui.screen_list_entity, &g_HAdevice_ctx);
    //         break;

    //     case HA_STATE_JSON_ERROR:
    //         ESP_LOGE(TAG, "JSON 解析失败");
    //         g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
    //         break;


    //     case HA_STATE_HTTP_ERROR:
    //         ESP_LOGE(TAG, "HTTP 请求失败");
    //         g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
    //         break;

    //     // after check
    //     case HA_STATE_DOWNLOADING:
    //         ESP_LOGI(TAG, "正在将设备添加到 UI 列表...");
    //         break;

    //     case HA_STATE_SUCCESS :
    //         ESP_LOGI(TAG, "设备已成功添加到 UI 列表");
    //         g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
    //         break;

    //     case HA_STATE_FAILED:
    //         ESP_LOGE(TAG, "设备添加到 UI 列表失败");
    //         g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
    //         break;

    //     default:
    //         break;
    // }
}

static void HA_button_action(lv_event_t * e)
{
    ESP_LOGI(TAG, "执行设备操作");
}

static void HA_button_event_register(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);       
    int index = (intptr_t)lv_obj_get_user_data(obj);
    ha_device_t * devices = (ha_device_t *)lv_event_get_user_data(e);

    if (devices) {
        ESP_LOGI("UI_EVENT", "点击了: %s, 状态: %s", devices->entity[index].friendly_name, devices->entity[index].is_on ? "ON" : "OFF");

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

static void HA_button_event_cb(lv_event_t * e)
{

    // lv_obj_t * btn = lv_event_get_target(e);
    // lv_obj_t * mbox = (lv_obj_t *)lv_event_get_user_data(e);
    
    // const char * txt = lv_label_get_text(lv_obj_get_child(btn, 0));

    // if (strcmp(txt, "Sure") == 0) {
    //     intptr_t index = (intptr_t)lv_obj_get_user_data(mbox); 
    //     ESP_LOGI(TAG, "用户确认添加设备 %s", g_HAdevice_ctx.entity[index].friendly_name);

    //     lv_obj_t * new_btn = lv_btn_create(guider_ui.screen_tabview_main_tab_2);
        
    //     lv_obj_set_size(new_btn, 90, 40);

    //     lv_obj_t * label = lv_label_create(new_btn);
    //     lv_label_set_text(label, g_HAdevice_ctx.entity[index].friendly_name);
    //     lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
    //     lv_obj_set_style_anim_duration(label, 4000, LV_PART_MAIN);
    //     lv_obj_set_width(label, 75);
    //     lv_obj_center(label);

    //     lv_obj_add_event_cb(new_btn, HA_button_action, LV_EVENT_CLICKED, (void *)index);
    //     // save_devices_to_nvs(); 
    // }
    // lv_msgbox_close(mbox);
}

static void HA_json_to_list(lv_obj_t *list_obj, ha_device_t *devices) 
{
    // g_HAdevice_ctx.state_ha = HA_STATE_DOWNLOADING;
    // if (!list_obj || !devices) {
    //     g_HAdevice_ctx.state_ha = HA_STATE_FAILED;
    //     ESP_LOGE(TAG, "无效的列表对象或设备数据");
    //     return;
    // }


    // lv_obj_clean(list_obj); 
    // ESP_LOGI(TAG, "正在将 %d 个设备添加到 UI 列表...", devices->device_count);

    // for (int i = 0; i < devices->device_count; i++) {
    //     lv_obj_t * btn = lv_list_add_btn(guider_ui.screen_list_entity, 
    //                                     LV_SYMBOL_SETTINGS, 
    //                                     devices->entity[i].friendly_name);


    //     lv_obj_set_user_data(btn, (void *)(intptr_t)i);
    //     lv_obj_add_event_cb(btn, HA_button_event_register, LV_EVENT_CLICKED, devices); 
    // }
    // devices->state_ha = HA_STATE_SUCCESS;
}

//OTA升级界面


void task_OTA_state_monitor(lv_timer_t * timer)
{
    if (lv_scr_act() != guider_ui.screen_OTA) {
        return;
    }

    if (guider_ui.screen_OTA_label_OTA_state == NULL || guider_ui.screen_OTA_label_OTA_info1 == NULL || guider_ui.screen_OTA_label_OTA_info2 == NULL) {
        ESP_LOGI("UI_EVENT", "OTA界面标签未初始化，无法更新状态显示");
        return;
    }

    //逻辑更改 进入页面后才开始http请求检查版本，避免不必要的请求和状态更新
    static ota_state_t last_state = -1;
    if(g_ota_ctx.state != last_state) {
        switch (g_ota_ctx.state)
        {
            case OTA_STATE_IDLE:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: Ready to update!");
                break;

            case OTA_STATE_CHECKING: 
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: Checking for updates...");
                break;
            // util checked
            case OTA_STATE_READY:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: Ready to update!");
                lv_label_set_text_fmt(guider_ui.screen_OTA_label_OTA_info2, "Latest: %s", g_ota_ctx.latest_ver);
                break;

            case OTA_STATE_NO_NEW:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: No new version");
                g_ota_ctx.state = OTA_STATE_IDLE; 
                break;

            case OTA_STATE_HTTP_ERROR:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: HTTP error");
                g_ota_ctx.state = OTA_STATE_IDLE; 
                break;

            // after check
            case OTA_STATE_DOWNLOADING:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: Downloading update...");
                break;

            case OTA_STATE_SUCCESS :
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: Update successful! Restarting...");
                break;

            case OTA_STATE_FAILED:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: Update failed");
                g_ota_ctx.state = OTA_STATE_IDLE; 
                break;

            case OTA_STATE_LEN_NOFIT:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: Received data mismatch");
                break;

            default:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: Unknown state");
                break;
        }
        last_state = g_ota_ctx.state; 
        lv_label_set_text_fmt(guider_ui.screen_OTA_label_OTA_info1, "Current: %s", g_ota_ctx.current_ver);
    }
}

void custom_init(lv_ui *ui)
{
    /* Add your codes here */
}

