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
static char ota_write_data[BUFFSIZE + 1] = { 0 };

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

#define REPORT_TYPE_ACTIVE   "version_report"      
// =============================================================

static int compare_version(const char *ver1, const char *ver2);
static esp_mqtt_client_handle_t global_mqtt_client = NULL; 

// 全局缓存平台下发给我们的目标升级版本号，用于进度上报时的必填参数
static char target_upgrade_version[64] = "2.0.0"; 

// OTA 状态流静态上下文变量
static esp_ota_handle_t ota_upgrade_handle = 0;
static const esp_partition_t *ota_update_partition = NULL;
static int ota_total_read_bytes = 0;
static int ota_last_reported_percentage = 0;
static int ota_http_total_length = 0;

void mqtt_report_progress(int percentage, int status_type, int error_code, const char* desc){
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

/**
 * 🌟 核心：专为免检下载定制的 HTTP 事件流拦截回调
 * 此种高级模式下，任何自定义 Header 都绝对不会丢失，且底层不容易卡死
 */
static esp_err_t ota_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "🤝 TLS 握手成功，已建立无视证书的安全通道！");
            ota_total_read_bytes = 0;
            ota_last_reported_percentage = 0;
            ota_http_total_length = 0;
            break;
            
        case HTTP_EVENT_HEADERS_SENT:
            ESP_LOGI(TAG, "📤 含有 Authorization 的鉴权头已完好无损地发出！");
            break;

        case HTTP_EVENT_ON_HEADER:
            // 捕获华为云吐回来的文件总长度
            if (strcasecmp(evt->header_key, "Content-Length") == 0) {
                ota_http_total_length = atoi(evt->header_value);
                ESP_LOGI(TAG, "📊 华为云响应：固件总大小 = %d 字节", ota_http_total_length);
            }
            break;

        case HTTP_EVENT_ON_DATA:
            if (evt->data_len > 0) {
                // 如果是接收到的第一包数据，在此处初始化 Flash 分区
                if (ota_upgrade_handle == 0) {
                    ota_update_partition = esp_ota_get_next_update_partition(NULL);
                    esp_err_t err = esp_ota_begin(ota_update_partition, OTA_SIZE_UNKNOWN, &ota_upgrade_handle);
                    if (err != ESP_OK) {
                        ESP_LOGE(TAG, "OTA Begin 失败: %s", esp_err_to_name(err));
                        return ESP_FAIL;
                    }
                    ESP_LOGI(TAG, "⚙️ OTA 写入句柄初始化成功，开始盲刷 Flash...");
                }

                // 将当前切片数据直接写入 Flash
                esp_err_t err = esp_ota_write(ota_upgrade_handle, (const void *)evt->data, evt->data_len);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Flash 写入失败: %s", esp_err_to_name(err));
                    return ESP_FAIL;
                }

                ota_total_read_bytes += evt->data_len;

                // 动态计算进度
                int percentage = 0;
                if (ota_http_total_length > 0) {
                    percentage = (ota_total_read_bytes * 100) / ota_http_total_length;
                } else {
                    percentage = (ota_total_read_bytes / 10240);
                    if (percentage > 95) percentage = 95;
                }

                if (percentage - ota_last_reported_percentage >= 5) {
                    ota_last_reported_percentage = percentage;
                    ESP_LOGI(TAG, "📥 固件已下载: %d 字节 (%d%%)", ota_total_read_bytes, percentage);
                    mqtt_report_progress(percentage, 0, 0, "Flashing...");
                }
            }
            break;

        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "🔌 华为云下载流连接已断开");
            break;

        default:
            break;
    }
    return ESP_OK;
}

/**
 * 🚀 重构后的工业级 Perform 免检下载写入任务
 */
static void native_ota_download_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🚀 华为云安全通道 [Perform 管道模式] 下载启动...");
    mqtt_report_progress(0, 0, 0, "Downloading started from HW Cloud via Perform flow");

    char *param_str = (char *)pvParameters;
    char *url_ptr = param_str;
    char *token_ptr = strchr(param_str, '|');
    
    if (token_ptr != NULL) {
        *token_ptr = '\0'; 
        token_ptr++;       
    }

    ESP_LOGI(TAG, "🔗 解析完成。目标 URL: %s", url_ptr);

    // 重置全局下载数据追踪器
    ota_upgrade_handle = 0;
    ota_update_partition = NULL;
    ota_total_read_bytes = 0;
    ota_http_total_length = 0;

    // 🌟【针对 v5.5.6 的硬核解法】显式定义底层的 TLS 彻底豁免配置（必须配合你在 menuconfig 中勾选的选项）
esp_http_client_config_t config = {
        .url = url_ptr, 
        .timeout_ms = 15000,
        .keep_alive_enable = true,
        .method = HTTP_METHOD_GET,               // 强行锁死为 GET，对齐华为云规范
        .event_handler = ota_http_event_handler, // 绑定数据流事件
        .skip_cert_common_name_check = true,     // 跳过域名/IP检查
        
        // 🌟【核心修复】在 v5.x 中，直接使用 .tls_cfg 嵌套初始化
        .tls_cfg = {
            .crt_bundle_attach = NULL,           // 显式将证书置空，触发免检
        },
    };

    esp_http_client_handle_t http_client = esp_http_client_init(&config);
    if (http_client == NULL) {
        ESP_LOGE(TAG, "无法初始化 HTTP 客户端");
        mqtt_report_progress(0, 2, 255, "HTTP init failed");
        free(pvParameters); 
        vTaskDelete(NULL);
        return;
    }

    // 🌟【绝不丢失】在此处注入 Header。调用 perform 时，此数据会伴随 GET 一同原子化发送
    if (token_ptr && strlen(token_ptr) > 0) {
        char auth_header_buf[128];
        snprintf(auth_header_buf, sizeof(auth_header_buf), "Bearer %s", token_ptr);
        esp_http_client_set_header(http_client, "Authorization", auth_header_buf);
        esp_http_client_set_header(http_client, "Content-Type", "application/json");
        ESP_LOGI(TAG, "🔑 鉴权通行令牌（Bearer Token）已成功锁定至发送缓存");
    }

    // 🌟 一键启动阻塞事务流，底层事件会自动流式分批写入 Flash
    esp_err_t perform_err = esp_http_client_perform(http_client);
    
    // 获取实际被执行返回的 HTTP 状态码
    int status_code = esp_http_client_get_status_code(http_client);
    ESP_LOGI(TAG, "📢 事务流执行完毕。HTTP 响应状态码: %d", status_code);

    if (perform_err == ESP_OK && (status_code >= 200 && status_code < 300)) {
        // 数据完全下载完毕，关闭句柄并切换 Boot 分区
        if (ota_total_read_bytes > 0 && esp_ota_end(ota_upgrade_handle) == ESP_OK) {
            esp_err_t boot_err = esp_ota_set_boot_partition(ota_update_partition);
            if (boot_err == ESP_OK) {
                ESP_LOGI(TAG, "🎉 🎉 🎉 固件完美升级成功！共下载并刷写 %d 字节。1.5秒后自动重启...", ota_total_read_bytes);
                mqtt_report_progress(100, 1, 0, "Upgrade success! Device rebooting..."); 
                vTaskDelay(pdMS_TO_TICKS(1500));
                esp_restart(); 
            } else {
                ESP_LOGE(TAG, "切换 Boot 分区失败");
                mqtt_report_progress(ota_last_reported_percentage, 2, 10, "Switch boot failed");
            }
        } else {
            ESP_LOGE(TAG, "OTA 结束校验失败，固件哈希残缺或不匹配");
            mqtt_report_progress(ota_last_reported_percentage, 2, 7, "Firmware verification failed"); 
        }
    } else {
        // 走到这说明传输异常，或者拿到了非 2xx 报错（例如 401 拦截等）
        ESP_LOGE(TAG, "HTTP Perform 事务执行失败: %s, 状态码: %d", esp_err_to_name(perform_err), status_code);
        mqtt_report_progress(ota_last_reported_percentage, 2, 7, "Network transfer or Auth gate failed");
        
        // 失败容错：如果已经开始了句柄，必须关掉释放闪存锁，防止死锁
        if (ota_upgrade_handle != 0) {
            esp_ota_end(ota_upgrade_handle);
        }
    }

    // 清理资源，打完收工
    esp_http_client_cleanup(http_client);
    free(pvParameters); 
    vTaskDelete(NULL);
}

static void mqtt_report_version_generic(esp_mqtt_client_handle_t client, const char* version, const char* report_type)
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
    cJSON_AddStringToObject(service_item, "event_type", report_type); 
    
    cJSON *paras = cJSON_CreateObject();
    cJSON_AddStringToObject(paras, "fw_version", ver_buf); 
    cJSON_AddStringToObject(paras, "sw_version", ver_buf); 
    
    cJSON_AddItemToObject(service_item, "paras", paras);
    cJSON_AddItemToArray(services, service_item);
    cJSON_AddItemToObject(root, "services", services);
    
    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str) {
        esp_mqtt_client_publish(client, MQTT_TOPIC_EVENTS_UP, json_str, 0, 1, 0);
        ESP_LOGI(TAG, "【物模型业务: %s】对齐标准发包, 版本: %s", report_type, ver_buf);
        free(json_str);
    }
    cJSON_Delete(root);
}

/**
 * 📥 解析并自动应答华为云物模型事件
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
                
                // 💡 1. 收到平台的 "version_query" 
                if (strcmp(event_type->valuestring, "version_query") == 0) {
                    ESP_LOGI(TAG, "🤖 收到华为云 version_query 指令，触发应答机制...");
                    const esp_app_desc_t *app_desc = esp_app_get_description();
                    if (global_mqtt_client) {
                        mqtt_report_version_generic(global_mqtt_client, app_desc->version, REPORT_TYPE_ACTIVE);
                    }
                }
                
                // 💡 2. 处理平台下发的 "firmware_upgrade" 升级包
                else if (strcmp(event_type->valuestring, "firmware_upgrade") == 0) {
                    cJSON *paras = cJSON_GetObjectItemCaseSensitive(service_item, "paras");
                    if (paras) {
                        cJSON *ver = cJSON_GetObjectItem(paras, "version");     
                        cJSON *url = cJSON_GetObjectItem(paras, "url");
                        cJSON *token = cJSON_GetObjectItem(paras, "access_token"); 

                        if (cJSON_IsString(ver) && cJSON_IsString(url)) {
                            const esp_app_desc_t *app_desc = esp_app_get_description();
                            
                            strncpy(target_upgrade_version, ver->valuestring, sizeof(target_upgrade_version) - 1);
                            ESP_LOGI(TAG, "📥 收到华为云升级指令！目标新版本: %s", ver->valuestring);
                            
                            if (compare_version(ver->valuestring, app_desc->version) > 0) {
                                
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
                                    BaseType_t res = xTaskCreate(native_ota_download_task, "ota_download", 1024 * 8, (void*)task_param, 5, NULL);
                                    
                                    if (res != pdPASS) {
                                        ESP_LOGE(TAG, "无法创建 OTA 下载任务！");
                                        free(task_param);
                                    }
                                } else {
                                    ESP_LOGE(TAG, "内存不足，无法分配下载链接缓冲区！");
                                }
                            } else {
                                ESP_LOGI(TAG, "下发版本不高于当前版本，放弃升级。");
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
            
            const esp_app_desc_t *app_desc = esp_app_get_description();
            mqtt_report_version_generic(client, app_desc->version, REPORT_TYPE_ACTIVE);
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

static int compare_version(const char *v1, const char *v2) {
    int v1_major = 0, v1_minor = 0, v1_patch = 0;
    int v2_major = 0, v2_minor = 0, v2_patch = 0;
    
    if (*v1 == 'V' || *v1 == 'v') v1++;
    if (*v2 == 'V' || *v2 == 'v') v2++;

    sscanf(v1, "%d.%d.%d", &v1_major, &v1_minor, &v1_patch);
    sscanf(v2, "%d.%d.%d", &v2_major, &v2_minor, &v2_patch);
    
    if (v1_major != v2_major) return (v1_major > v2_major) ? 1 : -1;
    if (v1_minor != v2_minor) return (v1_minor > v2_minor) ? 1 : -1;
    if (v1_patch != v2_patch) return (v1_patch > v2_patch) ? 1 : -1;
    return 0; 
}