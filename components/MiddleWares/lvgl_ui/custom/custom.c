#include <stdio.h>
#include "lvgl.h"
#include "custom.h"
#include "esp_log.h"
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
        case HA_STATE_READY:
            HA_json_to_list(guider_ui.screen_HA_list_HA_show, &g_HAdevice_ctx);
            break;
        case HA_STATE_JSON_ERROR:
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;
        case HA_STATE_HTTP_ERROR:
            g_HAdevice_ctx.state_ha = HA_STATE_IDLE;
            break;
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
        return;
    }

    g_HAdevice_ctx.state_ha = HA_STATE_DOWNLOADING;
    if (!list_obj || !devices) {
        g_HAdevice_ctx.state_ha = HA_STATE_FAILED;
        return;
    }
    
    lv_obj_clean(list_obj); 
    ESP_LOGI(TAG, "Save %d dispo to ui list", devices->device_count);

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
        case OTA_MQTT_STATE_IDLE:{
            s_state = "State: IDLE"; 
            break;}
        case OTA_MQTT_STATE_READY:
            s_state = "State: Ready to update"; 
            snprintf(buf_las, sizeof(buf_las), "Latest: %s", received_state.latest_ver);
            break;
        case OTA_MQTT_STATE_DOWNLOADING:
            s_state = "State: Downloading";
            snprintf(buf_las, sizeof(buf_las), "Latest: %s", received_state.latest_ver);
            break;
        case OTA_MQTT_STATE_SUCCESS:{
            s_state = "State: Update successful";
            break;}
        case OTA_MQTT_STATE_FAILED:{
            s_state = "State: Update failed";
            break;}
        default:{
            s_state = "State: Unknown state";
            break;}
    }
    lv_label_set_text(guider_ui.screen_OTA_label_OTA_state, s_state);
    lv_label_set_text(guider_ui.screen_OTA_label_OTA_info1, buf_cur);
    lv_label_set_text(guider_ui.screen_OTA_label_OTA_info2, buf_las);
}

void custom_init(lv_ui *ui)
{
    /* Add your codes here */
}

void WIFI_state_monitor_task(lv_timer_t * timer){

    if(lv_scr_act() != guider_ui.screen_wifi)
    {return;}
    wifi_sm_t received_state = g_wifi_sm;
    const char *s_state = "State: Unknown";
    char buf_ssid[48] = "SSID: ";
    char buf_ip[48] = "IP: ";
    switch (received_state.wifi_FSM_state){
        case WIFI_STATE_IDLE:{
            s_state = "State: Checking"; 
            break;}
        case WIFI_STATE_NONVS_CONFIG:{
            s_state = "State: No WiFi Config";
            break;}
        case WIFI_STATE_STA_CONNECTING: {
            s_state = "State: Connecting";
            break;}
        case WIFI_STATE_PROVISIONING:
            s_state = "State: Provisioning";
            snprintf(buf_ssid, sizeof(buf_ssid), "SSID: ESP32_AP");
            snprintf(buf_ip, sizeof(buf_ip), "Password: 12345678");
            break;
        case WIFI_STATE_WAIT_BEGIN_PROVISIONING:{
            s_state = "State: No WiFi Config now, waiting for provisioning";
            break;}
        case WIFI_STATE_CONNECTED:
            s_state = "State: Connected";
            snprintf(buf_ssid, sizeof(buf_ssid), "SSID: %s", received_state.wifi_ssid);
            snprintf(buf_ip, sizeof(buf_ip), "IP: %s", received_state.wifi_ip);
            break;
        case WIFI_STATE_DISCONNECTED:{
            s_state = "State: Disconnected";
            break;}
        default:{
            s_state = "State: Unknown state";
            break;}
    } 
    lv_label_set_text(guider_ui.screen_wifi_label_wifi_state, s_state);
    lv_label_set_text(guider_ui.screen_wifi_label_wifi_info1, buf_ssid);
    lv_label_set_text(guider_ui.screen_wifi_label_wifi_info2, buf_ip);
}

//main screen
static lv_obj_t *device_site_get(uint8_t idx)
{
    switch (idx)
    {
        case 0:
            return guider_ui.screen_main_img_cont_rt;
        case 1:
            return guider_ui.screen_main_img_cont_rm;
        case 2:
            return guider_ui.screen_main_img_cont_rd;
        case 3:
            return guider_ui.screen_main_img_cont_md;
        default:
            return NULL;
    }
}
static void Main_date_monitor_task(lv_timer_t *timer)
{ 
    if (lv_scr_act() != guider_ui.screen_main) {return;}
    static char last_state[4][5] = {0};
    for(int i = 0; i < 4; i++)
    {
        if(strcmp(g_main_ui_device_data[i].state, last_state[i]) != 0){
            strcpy(last_state[i], g_main_ui_device_data[i].state);
            lv_obj_t *img_obj = device_site_get(i);
            if(img_obj == NULL) continue;
            if (strcmp(g_main_ui_device_data[i].state, "on") == 0)
            {
                lv_image_set_src(img_obj, &_swtich_on_RGB565A8_40x40);
            }else{
                lv_image_set_src(img_obj, &_switch_off_RGB565A8_40x40);
            }
        }
    }
    lv_label_set_text(guider_ui.screen_main_label_date, g_main_ui_device_data[4].state);
    lv_label_set_text(guider_ui.screen_main_label_temp, entity_weather_data.temp);
    lv_label_set_text(guider_ui.screen_main_label_hum, entity_weather_data.hum);
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
}

