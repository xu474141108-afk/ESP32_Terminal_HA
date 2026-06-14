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

#define MQTT_TOPIC_EVENTS_UP   "$oc/devices/" HUAWEI_BASE_DEVICE_ID "/sys/events/up"   // 设备上报（版本/进度）
#define MQTT_TOPIC_EVENTS_DOWN "$oc/devices/" HUAWEI_BASE_DEVICE_ID "/sys/events/down" // 平台下发（升级通知）
    

static esp_mqtt_client_handle_t global_mqtt_client = NULL; 
static char target_upgrade_version[64] = "2.0.0"; 
static char ota_write_data[BUFFSIZE + 1] = { 0 };


static void http_cleanup(esp_http_client_handle_t client)
{
    if (client != NULL) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        client = NULL;
    }
}

static void mqtt_report_progress(int percentage, int status_type, int error_code, const char* desc){
    if (global_mqtt_client == NULL) return;
    
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "object_device_id", HUAWEI_BASE_DEVICE_ID); 
    
    cJSON *services = cJSON_CreateArray();
    cJSON *service_item = cJSON_CreateObject();
    cJSON_AddStringToObject(service_item, "service_id", "$ota");
    cJSON_AddStringToObject(service_item, "event_type", "upgrade_progress_report"); 
    
    cJSON *paras = cJSON_CreateObject();
    cJSON_AddNumberToObject(paras, "progress", percentage);     
    cJSON_AddStringToObject(paras, "version", target_upgrade_version); // 必须是平台期望的目标新版本号
    
    // 根据状态动态组装符合官方定义的 JSON
    if (status_type == 1) {
        cJSON_AddNumberToObject(paras, "result_code", 0); // 成功
    } else if (status_type == 2) {
        cJSON_AddNumberToObject(paras, "result_code", error_code); // 失败
    }
    
    if (desc) {
        cJSON_AddStringToObject(paras, "description", desc);
    }
    
    cJSON_AddItemToObject(service_item, "paras", paras);
    cJSON_AddItemToArray(services, service_item);
    cJSON_AddItemToObject(root, "services", services);
    
    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str) {
        esp_mqtt_client_publish(global_mqtt_client, MQTT_TOPIC_EVENTS_UP, json_str, 0, 1, 0);
        free(json_str);
    }
    cJSON_Delete(root);
}


static void OTA_mqtt_download_task(void *pvParameters)
{
    char *task_param = (char *)pvParameters;
    char *url_ptr = NULL;
    char *token_ptr = NULL;
    char *save_ptr = NULL;
    if (task_param == NULL)
    {
        ESP_LOGE(TAG, "OTA入参指针为空");
        vTaskDelete(NULL);
        return;
    }

    url_ptr = strtok_r(task_param, "|",&save_ptr);
    if (url_ptr == NULL)
    {
        ESP_LOGE(TAG, "参数解析失败，无URL");
        free(task_param); // 释放堆内存
        vTaskDelete(NULL);
        return;
    }
    token_ptr = strtok_r(NULL, "|",&save_ptr); // 第二段是token，可能为NULL


    // if(g_ota_ctx.state != OTA_STATE_DOWNLOADING) return;

    int binary_file_length = 0;
    esp_ota_handle_t update_handle = 0 ;
    g_ota_ctx.update_partition = esp_ota_get_next_update_partition(NULL);

    esp_http_client_config_t http_config = {
        .url = url_ptr,
        // .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = 10000,
        .keep_alive_enable = true,
    };
    esp_http_client_handle_t http_OTA_Download_client = esp_http_client_init(&http_config);    
    char auth_header[512] = {0};
    // 严格按照文档格式拼接：Bearer + 空格 + 你的 token
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
    err = esp_ota_begin(g_ota_ctx.update_partition, OTA_SIZE_UNKNOWN, &update_handle);
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
    err = esp_ota_set_boot_partition(g_ota_ctx.update_partition);
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


static void mqtt_report_version_generic(esp_mqtt_client_handle_t client, const char* version)
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
        ESP_LOGI(TAG, "端到服务器上报版本: %s", ver_buf);
        free(json_str);
    }
    cJSON_Delete(root);
}

/**
 * 解析并自动应答华为云物模型事件
 */
static void process_ota_json(const char *json_str)
{
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) return;

    cJSON *services = cJSON_GetObjectItem(root, "services");
    if (cJSON_IsArray(services)) {
        cJSON *service_item = cJSON_GetArrayItem(services, 0);
        if (service_item) {
            cJSON *event_type = cJSON_GetObjectItem(service_item, "event_type");
            if (cJSON_IsString(event_type)) {
                // 1. 处理平台下发的 "version_query" 指令
                if (strcmp(event_type->valuestring, "version_query") == 0) {
                    ESP_LOGI(TAG, "收到华为云 version_query 指令，触发应答机制...");
                    const esp_app_desc_t *app_desc = esp_app_get_description();
                    if (global_mqtt_client) {
                        mqtt_report_version_generic(global_mqtt_client, app_desc->version);
                    }
                }
                
                // 2. 处理平台下发的 "firmware_upgrade" 升级包
                else if (strcmp(event_type->valuestring, "firmware_upgrade") == 0) {
                    cJSON *paras = cJSON_GetObjectItemCaseSensitive(service_item, "paras");
                    if (paras) {
                        cJSON *ver = cJSON_GetObjectItem(paras, "version");     
                        cJSON *url = cJSON_GetObjectItem(paras, "url");
                        cJSON *token = cJSON_GetObjectItem(paras, "access_token"); 

                        if (cJSON_IsString(ver) && cJSON_IsString(url)) {
                            ESP_LOGI(TAG, "📥 收到华为云升级指令！目标新版本: %s", ver->valuestring);
                                // 动态分配大缓冲区，用 '|' 拼接 URL 和 Token 传递给任务头
                                char *task_param = malloc(1024);
                                if (task_param != NULL) {
                                    memset(task_param, 0, 1024);
                                    if (token && cJSON_IsString(token)) {
                                        snprintf(task_param, 1024, "%s|%s", url->valuestring, token->valuestring);
                                    } else {
                                        snprintf(task_param, 1024, "%s|", url->valuestring);
                                    }
                                    
                                    ESP_LOGI(TAG, "🚀 成功打包鉴权参数，正在拉起独立固件下载任务...");
                                    BaseType_t res = xTaskCreate(OTA_mqtt_download_task, "ota_download", 1024 * 8, (void*)task_param,3, NULL);
                                    
                                    if (res != pdPASS) {
                                        ESP_LOGE(TAG, "无法创建 OTA 下载任务！");
                                        free(task_param);
                                    }
                                } else {
                                    ESP_LOGE(TAG, "堆内存不足，无法分配下载链接缓冲区！");
                                }
                        }
                    }
                }
            }
        }
    }
    cJSON_Delete(root);
}

/**
 * MQTT 事件监听回调
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            global_mqtt_client = client; 
            ESP_LOGI(TAG, "成功接入华为云物联网平台！");
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_EVENTS_DOWN, 1);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "与华为云断开连接");
            break;

        case MQTT_EVENT_DATA:
            if (strncmp(event->topic, MQTT_TOPIC_EVENTS_DOWN, event->topic_len) == 0) {
                int copy_len = (event->data_len < BUFFSIZE) ? event->data_len : BUFFSIZE;
                memcpy(ota_write_data, event->data, copy_len);
                ota_write_data[copy_len] = '\0'; 
                process_ota_json(ota_write_data);
            }
            break;

        default:
            break;
    }
}

void ota_mqtt_init(void)
{
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
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

