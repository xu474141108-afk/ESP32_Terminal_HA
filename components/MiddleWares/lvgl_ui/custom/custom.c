#include <stdio.h>
#include "lvgl.h"
#include "custom.h"
#include "esp_log.h"
#include "OTA.h"


/**********************
 *  HA new button set
 **********************/
#define TAG "Custom"
// HA设备操作界面

UI_Main_cout_item_t g_ui_main_data = {0};
static int index = 0;

static void HA_json_to_list(lv_obj_t *list_obj, ha_device_t *devices);


void task_HA_state_monitor(lv_timer_t * timer){

    if(lv_scr_act() != guider_ui.screen_HA)
    {
        return;
    }

    ha_entity_t * p_target_slot = NULL;
    switch (g_HAdevice_ctx.state_ha){
        case HA_STATE_IDLE:
            break;

        case HA_STATE_SEARCHING: 
            ESP_LOGI(TAG, "HA设备状态: SEARCHING，正在搜索设备...");
            break;

        // util checked
        case HA_STATE_READY:
            ESP_LOGI(TAG, "HA设备状态: READY，准备将设备添加到 UI 列表");
            HA_json_to_list(guider_ui.screen_HA_list_HA_show, &g_HAdevice_ctx);
            break;

        case HA_STATE_JSON_ERROR:
            ESP_LOGE(TAG, "JSON 解析失败");
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;


        case HA_STATE_HTTP_ERROR:
            ESP_LOGE(TAG, "HTTP 请求失败");
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;

        // after check
        case HA_STATE_DOWNLOADING:
            ESP_LOGI(TAG, "正在将设备添加到UI列表...");
            break;

        case HA_STATE_SUCCESS :
            ESP_LOGI(TAG, "设备已成功添加到UI列表");
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;

        case HA_STATE_FAILED:
            ESP_LOGE(TAG, "设备添加到UI列表失败");
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;

        case HA_STATE_SHOW_CONT:
            lv_obj_add_flag(guider_ui.screen_HA_cont_HA_main, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(guider_ui.screen_HA_cont_HA_select, LV_OBJ_FLAG_HIDDEN);  
            ESP_LOGI(TAG, "显示设备详情");
            g_HAdevice_ctx.state_ha = HA_STATE_HOLD_CONT;
            break;
        case HA_STATE_HOLD_CONT:
            ESP_LOGI(TAG, "等待选择");
            break;
        case HA_STATE_CONT_RT:
            p_target_slot = &g_ui_main_data.cont_rt;
        break;

        case HA_STATE_CONT_RM:
            p_target_slot = &g_ui_main_data.cont_rm;
        break;
        
        case HA_STATE_CONT_RD:
            p_target_slot = &g_ui_main_data.cont_rd;
        break;

        case HA_STATE_CONT_MD:
            p_target_slot = &g_ui_main_data.cont_md;
        break;

        case HA_STATE_CLOSE_CONT:
            lv_obj_add_flag(guider_ui.screen_HA_cont_HA_select, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(guider_ui.screen_HA_cont_HA_main, LV_OBJ_FLAG_HIDDEN);
            ESP_LOGI(TAG, "关闭设备详情");
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;

        default:
            ESP_LOGE(TAG, "未知状态");
            break;
    }

    if (p_target_slot != NULL) {
        ESP_LOGI("FSM", "状态机捕捉到位置状态 [%d]，开始统一盖章...", g_HAdevice_ctx.state_ha);
        u8_t g_temp_selecting_index = 1;

        if (g_temp_selecting_index >= 0) {
            *p_target_slot = g_HAdevice_ctx.entity[g_temp_selecting_index];       
            ESP_LOGI("FSM", "中转指针中的数据id: [%s]， name:[%s]", p_target_slot->entity_id, p_target_slot->friendly_name);
            // g_temp_selecting_index = -1; // 擦除临时记事本
        }



        g_HAdevice_ctx.state_ha = HA_STATE_CLOSE_CONT;
    }
        
}





static void HA_select_event_show(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);       
    int index = (intptr_t)lv_obj_get_user_data(obj);

    g_HAdevice_ctx.state_ha = HA_STATE_SHOW_CONT;
}

static void HA_json_to_list(lv_obj_t *list_obj, ha_device_t *devices) {

    if(lv_scr_act() != guider_ui.screen_HA)
    {
        ESP_LOGI(TAG,"非HA界面");
        return;
    }

    g_HAdevice_ctx.state_ha = HA_STATE_DOWNLOADING;
    if (!list_obj || !devices) {
        g_HAdevice_ctx.state_ha = HA_STATE_FAILED;
        ESP_LOGE(TAG, "无效的列表对象或设备数据");
        return;
    }
    
    lv_obj_clean(list_obj); 
    ESP_LOGI(TAG, "正在将 %d 个设备添加到UI列表...", devices->device_count);

    for (int i = 0; i < devices->device_count; i++) {
        lv_obj_t * btn = lv_list_add_btn(guider_ui.screen_HA_list_HA_show, LV_SYMBOL_SETTINGS, devices->entity[i].friendly_name);
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        lv_obj_add_event_cb(btn, HA_select_event_show, LV_EVENT_CLICKED, devices); 
    }
    devices->state_ha = HA_STATE_SUCCESS;

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

