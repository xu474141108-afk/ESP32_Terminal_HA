#include <stdio.h>
#include "lvgl.h"
#include "custom.h"
#include "esp_log.h"
#include "OTA.h"
#include "OTA_MQTT.h"
#include "app_wifi.h"
#include "storage_manager.h"
#include "OTA_MQTT.h"
#include "ha_ws_client.h"

/**********************
 *  HA new button set
 **********************/
#define TAG "Custom"

// HA设备操作界面

static int g_temp_selecting_index = -1;
//timer
lv_timer_t *wifi_timer = NULL;
lv_timer_t *ha_timer = NULL;
lv_timer_t *ota_timer = NULL;
lv_timer_t *main_timer = NULL;

static void HA_json_to_list(lv_obj_t *list_obj, ha_device_t *devices);
static void HA_select_event_show(lv_event_t * e);


void HA_state_monitor_task(lv_timer_t * timer){

    if(lv_scr_act() != guider_ui.screen_HA)
    {
        return;
    }
    u8_t p_target_slot = 5;
    switch (g_HAdevice_ctx.state_ha){
        case HA_STATE_IDLE:
            break;

        case HA_STATE_SEARCHING: 
            break;

        // util checked
        case HA_STATE_READY:
            HA_json_to_list(guider_ui.screen_HA_list_HA_show, &g_HAdevice_ctx);
            break;

        case HA_STATE_JSON_ERROR:
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;


        case HA_STATE_HTTP_ERROR:
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;

        // after check
        case HA_STATE_DOWNLOADING:
            break;

        case HA_STATE_SUCCESS :
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;

        case HA_STATE_FAILED:
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;

        case HA_STATE_SHOW_CONT:
            lv_obj_add_flag(guider_ui.screen_HA_cont_HA_main, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(guider_ui.screen_HA_cont_HA_select, LV_OBJ_FLAG_HIDDEN);  
            g_HAdevice_ctx.state_ha = HA_STATE_HOLD_CONT;
            break;
        case HA_STATE_HOLD_CONT:
            break;
        case HA_STATE_CONT_RT:
            p_target_slot = 0;
        break;

        case HA_STATE_CONT_RM:
            p_target_slot = 1;
        break;
        
        case HA_STATE_CONT_RD:
            p_target_slot = 2;
        break;

        case HA_STATE_CONT_MD:
            p_target_slot = 3;
        break;

        case HA_STATE_CLOSE_CONT:
            lv_obj_add_flag(guider_ui.screen_HA_cont_HA_select, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(guider_ui.screen_HA_cont_HA_main, LV_OBJ_FLAG_HIDDEN);
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;

        default:
            ESP_LOGE(TAG, "Unknown state");
            break;
    }

    if (p_target_slot <5) {
        if (g_temp_selecting_index >= 0) {
            g_main_ui_device_data[p_target_slot] = g_HAdevice_ctx.entity[g_temp_selecting_index];
            xQueueSend(nvs_save_queue, &p_target_slot, 0);
            g_temp_selecting_index = -1; 
        }
        g_HAdevice_ctx.state_ha = HA_STATE_CLOSE_CONT;
    }
        
}

static void HA_select_event_show(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);       
    g_temp_selecting_index = (intptr_t)lv_obj_get_user_data(obj);
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

//OTA MQTT升级界面
void OTA_MQTT_state_monitor_task(lv_timer_t * timer)
{
    if (lv_scr_act() != guider_ui.screen_OTA) {
        return;
    }
    ota_mqtt_context_t received_state = g_ota_mqtt_ctx;
    const char *s_state = "State: Unknown";
    char buf_cur[50] = "Current: ";
    char buf_las[50] = "Latest: ";
    snprintf(buf_cur, sizeof(buf_cur), "Current: %s", received_state.current_ver);
    switch (received_state.state)
    {
        case OTA_MQTT_STATE_IDLE:
            s_state = "State: IDLE"; 
            break;

        // util checked
        case OTA_MQTT_STATE_READY:
            s_state = "State: Ready to update"; 
            snprintf(buf_las, sizeof(buf_las), "Latest: %s", received_state.latest_ver);
            break;

        // after check
        case OTA_MQTT_STATE_DOWNLOADING:
            s_state = "State: Downloading";
            break;

        case OTA_MQTT_STATE_SUCCESS :
            s_state = "State: Update successful";
            break;

        case OTA_MQTT_STATE_FAILED:
            s_state = "State: Update failed";
            break;

        default:
            s_state = "State: Unknown state";
            break;
    }
    lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, s_state);
    lv_label_set_text(guider_ui.screen_OTA_label_OTA_info1, buf_cur);
    lv_label_set_text(guider_ui.screen_OTA_label_OTA_info2, buf_las);
}

void OTA_state_monitor_task(lv_timer_t * timer)
{
    if (lv_scr_act() != guider_ui.screen_OTA) {
        return;
    }


    static ota_state_t last_state = -1;
    if(g_ota_ctx.state != last_state) {
        switch (g_ota_ctx.state)
        {
            case OTA_STATE_IDLE:
                lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, "Status: IDLE");
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

void WIFI_state_monitor_task(lv_timer_t * timer){

    if(lv_scr_act() != guider_ui.screen_wifi)
    {
        return;
    }
    wifi_sm_t received_state = g_wifi_sm;
    const char *s_state = "State: Unknown";
    char buf_ssid[48] = "SSID: ";
    char buf_ip[48] = "IP: ";
    switch (received_state.wifi_FSM_state){
        case WIFI_STATE_IDLE:
            s_state = "State: Checking"; 
            break;

        case WIFI_STATE_NONVS_CONFIG:
            s_state = "State: No WiFi Config";
            break;
        case WIFI_STATE_STA_CONNECTING: 
            s_state = "State: Connecting";
            break;
        case WIFI_STATE_PROVISIONING:
            s_state = "State: Provisioning";
            snprintf(buf_ssid, sizeof(buf_ssid), "SSID: ESP32_AP");
            snprintf(buf_ip, sizeof(buf_ip), "Password: 12345678");
            break;
        case WIFI_STATE_WAIT_BEGIN_PROVISIONING:
            s_state = "State: No WiFi Config now, waiting for provisioning";
            break;
        case WIFI_STATE_CONNECTED:
            s_state = "State: Connected";
            snprintf(buf_ssid, sizeof(buf_ssid), "SSID: %s", received_state.wifi_ssid);
            snprintf(buf_ip, sizeof(buf_ip), "IP: %s", received_state.wifi_ip);
            break;
        case WIFI_STATE_DISCONNECTED:
            s_state = "State: Disconnected";
            break;
        default:
            s_state = "State: Unknown state";
            break;
    } 
    
    lv_label_set_text(guider_ui.screen_wifi_label_wifi_state, s_state);
    lv_label_set_text(guider_ui.screen_wifi_label_wifi_info1, buf_ssid);
    lv_label_set_text(guider_ui.screen_wifi_label_wifi_info2, buf_ip);
}

//
static void Main_date_monitor_task(lv_timer_t *timer)
{ 
    if (lv_scr_act() != guider_ui.screen_main) {
        return;
    }

     const char *state_str = g_main_ui_device_data[4].state;
    if (state_str == NULL || state_str[0] == '\0')
    {
        state_str = "--";
    }
    lv_label_set_text(guider_ui.screen_main_label_weather, state_str);
}

void all_timer_creat_init()
{
    wifi_timer = lv_timer_create(WIFI_state_monitor_task, 500, NULL);
    lv_timer_pause(wifi_timer);
    ota_timer = lv_timer_create(OTA_MQTT_state_monitor_task, 500, NULL);
    lv_timer_pause(ota_timer);
    ha_timer = lv_timer_create(HA_state_monitor_task, 100, NULL);
    lv_timer_pause(ha_timer);
    main_timer = lv_timer_create(Main_date_monitor_task, 1000, NULL);
    // lv_timer_pause(main_timer);
}