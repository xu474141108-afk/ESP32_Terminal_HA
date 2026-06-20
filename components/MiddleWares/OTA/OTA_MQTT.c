#include <string.h>
#include "esp_log.h"
#include "esp_app_desc.h"
#include "cJSON.h"
#include "mqtt_client.h"
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "OTA.h"

#define BUFFSIZE 1024          


#define TAG "HW_OTA_MQTT"

// ==================== IoTDA Configuration ====================
#define HW_MQTT_BROKER_URL  "mqtt://bc2f867476.st1.iotda-device.me-east-1.myhuaweicloud.com"
#define HW_MQTT_PORT        1883
#define HW_MQTTS_PORT       8883
#define HW_CLIENT_ID        "6a2db2637ce3a4387e2d5e3c_ESP32_HA_TERMINAL_TEST_01_0_0_2026061319"
#define HW_USERNAME         "6a2db2637ce3a4387e2d5e3c_ESP32_HA_TERMINAL_TEST_01"
#define HW_PASSWORD         "6b71a8896b7966c8215cd3bc0a0d2aad70becfdcd24ddef030bf379ae1dc6c83"
#define HUAWEI_BASE_DEVICE_ID "6a2db2637ce3a4387e2d5e3c_ESP32_HA_TERMINAL_TEST_01"

#define MQTT_TOPIC_EVENTS_UP   "$oc/devices/" HUAWEI_BASE_DEVICE_ID "/sys/events/up"   // Device Publish to Platform 
#define MQTT_TOPIC_EVENTS_DOWN "$oc/devices/" HUAWEI_BASE_DEVICE_ID "/sys/events/down" // Platform Publish to Device 
    

static esp_mqtt_client_handle_t global_mqtt_client = NULL; 
static char target_upgrade_version[64] = "2.0.0"; 
static char ota_write_data[BUFFSIZE + 1] = { 0 };
static const esp_partition_t *update_partition;

ota_context_t g_ota_ctx = {
    .state = OTA_STATE_IDLE,
    .current_ver = "1.1.1",
    .latest_ver = {0} 
};


static void http_cleanup(esp_http_client_handle_t client)
{
    if (client != NULL) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        client = NULL;
    }
}

static void ota_mqtt_download_task(void *pvParameters)
{
    char *task_param = (char *)pvParameters;
    char *url_ptr = NULL;
    char *token_ptr = NULL;
    char *save_ptr = NULL;
    if (task_param == NULL)
    {
        ESP_LOGE(TAG, "OTA task parameter is NULL");
        vTaskDelete(NULL);
        return;
    }

    url_ptr = strtok_r(task_param, "|",&save_ptr);
    if (url_ptr == NULL)
    {
        ESP_LOGE(TAG, "Failed to parse URL");
        free(task_param); // 释放堆内存
        vTaskDelete(NULL);
        return;
    }
    token_ptr = strtok_r(NULL, "|",&save_ptr); // 第二段是token，可能为NULL


    int binary_file_length = 0;
    esp_ota_handle_t update_handle = 0 ;
    update_partition = esp_ota_get_next_update_partition(NULL);

    esp_http_client_config_t http_config = {
        .url = url_ptr,
        .timeout_ms = 10000,
        .keep_alive_enable = true,
    };
    esp_http_client_handle_t http_OTA_Download_client = esp_http_client_init(&http_config);    
    char auth_header[512] = {0};

    // Bearer + " " + token
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", token_ptr);
    // 将其塞入 HTTP 请求头中
    esp_http_client_set_header(http_OTA_Download_client, "Authorization", auth_header);
    esp_http_client_set_header(http_OTA_Download_client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_open(http_OTA_Download_client, 0);
        ESP_LOGI(TAG, "开始下载固件，URL: %s", url_ptr);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        http_cleanup(http_OTA_Download_client);
        // g_ota_ctx.state = OTA_STATE_HTTP_ERROR;
        vTaskDelete(NULL); 
        return;
    }
    esp_http_client_fetch_headers(http_OTA_Download_client);
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin 失败: %s", esp_err_to_name(err));
        http_cleanup(http_OTA_Download_client);
        esp_ota_abort(update_handle);
        // g_ota_ctx.state = OTA_STATE_FAILED;
        vTaskDelete(NULL); 
    }
    
    int data_read = 0;
    while(1){
        data_read = esp_http_client_read(http_OTA_Download_client, ota_write_data, BUFFSIZE);   
        if(data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            http_cleanup(http_OTA_Download_client);
            esp_ota_abort(update_handle);
            // g_ota_ctx.state = OTA_STATE_FAILED;
            vTaskDelete(NULL); 
        } else if (data_read > 0) {
            err = esp_ota_write(update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_write 失败: %s", esp_err_to_name(err));
                http_cleanup(http_OTA_Download_client);
                esp_ota_abort(update_handle);
                // g_ota_ctx.state = OTA_STATE_FAILED;
                vTaskDelete(NULL); 
            }
            binary_file_length += data_read;
        } else if (data_read == 0) {
            if (esp_http_client_is_complete_data_received(http_OTA_Download_client)) {
                ESP_LOGI(TAG, "下载完成");
                http_cleanup(http_OTA_Download_client);
                // g_ota_ctx.state = OTA_STATE_INSTALLING;
                break;
            } else {
                ESP_LOGE(TAG, "连接意外中断");
                http_cleanup(http_OTA_Download_client);
                esp_ota_abort(update_handle);
                // g_ota_ctx.state = OTA_STATE_FAILED;
                vTaskDelete(NULL); 
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "固件校验失败 (OTA End): %s", esp_err_to_name(err));
        http_cleanup(http_OTA_Download_client);
        esp_ota_abort(update_handle);
        // g_ota_ctx.state = OTA_STATE_FAILED;
        vTaskDelete(NULL); 
    }
    // 设置下次启动的分区
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "设置启动分区失败: %s", esp_err_to_name(err));
        http_cleanup(http_OTA_Download_client);
        // g_ota_ctx.state = OTA_STATE_FAILED;
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "OTA 成功！系统将在 10 秒后重启...");
    g_ota_ctx.state = OTA_STATE_SUCCESS;
    vTaskDelay(pdMS_TO_TICKS(1*1000));
    esp_restart(); 
}

static void ota_mqtt_report_version_generic(esp_mqtt_client_handle_t client, const char* version)
{
    char ver_buf[64];
    if (*version != 'v' && *version != 'V') {
        snprintf(ver_buf, sizeof(ver_buf), "v%s", version);
    } else {
        strncpy(ver_buf, version, sizeof(ver_buf) - 1);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "object_device_id", HUAWEI_BASE_DEVICE_ID); 
    
    cJSON *services = cJSON_CreateArray();
    cJSON *service_item = cJSON_CreateObject();
    cJSON_AddStringToObject(service_item, "service_id", "$ota");
    cJSON_AddStringToObject(service_item, "event_type",  "version_report"); 
    
    cJSON *paras = cJSON_CreateObject();
    cJSON_AddStringToObject(paras, "fw_version", ver_buf); 
    cJSON_AddStringToObject(paras, "sw_version", ver_buf); 
    
    cJSON_AddItemToObject(service_item, "paras", paras);
    cJSON_AddItemToArray(services, service_item);
    cJSON_AddItemToObject(root, "services", services);
    
    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str) {
        esp_mqtt_client_publish(client, MQTT_TOPIC_EVENTS_UP, json_str, 0, 1, 0);
        free(json_str);
    }
    cJSON_Delete(root);
}

static void ota_json_process(const char *json_str)
{
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) return;

    cJSON *services = cJSON_GetObjectItem(root, "services");
    if (cJSON_IsArray(services)) {
        cJSON *service_item = cJSON_GetArrayItem(services, 0);
        if (service_item) {
            cJSON *event_type = cJSON_GetObjectItem(service_item, "event_type");
            if (cJSON_IsString(event_type)) {
                // process "version_query" 
                if (strcmp(event_type->valuestring, "version_query") == 0) {
                    ESP_LOGI(TAG, "Recieved version_query");
                    if (global_mqtt_client) {
                        ota_mqtt_report_version_generic(global_mqtt_client, g_ota_ctx.current_ver);
                    }
                }
                
                //process "firmware_upgrade" 
                else if (strcmp(event_type->valuestring, "firmware_upgrade") == 0) {
                    cJSON *paras = cJSON_GetObjectItemCaseSensitive(service_item, "paras");
                    if (paras) {
                        cJSON *ver = cJSON_GetObjectItem(paras, "version");     
                        cJSON *url = cJSON_GetObjectItem(paras, "url");
                        cJSON *token = cJSON_GetObjectItem(paras, "access_token"); 
                        
                        if (cJSON_IsString(ver) && cJSON_IsString(url)) {
                            strlcpy(g_ota_ctx.latest_ver, ver->valuestring, sizeof(g_ota_ctx.latest_ver));
                            ESP_LOGI(TAG, "Recieved firmware_upgrade, version: %s", ver->valuestring);
                                char *task_param = malloc(1024);
                                if (task_param != NULL) {
                                    memset(task_param, 0, 1024);
                                    if (token && cJSON_IsString(token)) {
                                        snprintf(task_param, 1024, "%s|%s", url->valuestring, token->valuestring);
                                    } else {
                                        snprintf(task_param, 1024, "%s|", url->valuestring);
                                    }
                                    
                                    BaseType_t res = xTaskCreate(ota_mqtt_download_task, "ota_download", 1024 * 8, (void*)task_param,3, NULL);
                                    
                                    if (res != pdPASS) {
                                        ESP_LOGE(TAG, "Failed to create OTA download task!");
                                        free(task_param);
                                    }
                                } else {
                                    ESP_LOGE(TAG, "Insufficient heap memory, failed to allocate download link buffer!");
                                }
                        }
                    }
                }
            }
        }
    }
    cJSON_Delete(root);
}

static void ota_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            global_mqtt_client = client; 
            ESP_LOGI(TAG, "Connected to Huawei Cloud IoTDA MQTT broker");
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_EVENTS_DOWN, 1);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Disconnected from Huawei Cloud IoTDA MQTT broker");
            break;

        case MQTT_EVENT_DATA:
            if (strncmp(event->topic, MQTT_TOPIC_EVENTS_DOWN, event->topic_len) == 0) {
                int copy_len = (event->data_len < BUFFSIZE) ? event->data_len : BUFFSIZE;
                memcpy(ota_write_data, event->data, copy_len);
                ota_write_data[copy_len] = '\0'; 
                ota_json_process(ota_write_data);
            }
            break;

        default:
            break;
    }
}

void ota_mqtt_init(void)
{

    const esp_app_desc_t *app_desc = esp_app_get_description();
    strncpy(g_ota_ctx.current_ver, app_desc->version, sizeof(g_ota_ctx.current_ver) - 1);
    g_ota_ctx.current_ver[sizeof(g_ota_ctx.current_ver) - 1] = '\0';
    
    char broker_uri[256];
    snprintf(broker_uri, sizeof(broker_uri), "%s:%d", HW_MQTT_BROKER_URL, HW_MQTT_PORT);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_uri, 
        .credentials.client_id = HW_CLIENT_ID,
        .credentials.username = HW_USERNAME,
        .credentials.authentication.password = HW_PASSWORD,
        .network.reconnect_timeout_ms = 5000,
        .network.disable_auto_reconnect = false,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, ota_mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

