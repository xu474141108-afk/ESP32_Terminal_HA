#include "esp_websocket_client.h"
#include "esp_log.h"
#include "cJSON.h" 
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "ha_ws_client.h"
#include "esp_event.h"
#include "custom.h"
#include "app_test.h"

#define MAX_WS_BUFFER_SIZE (128 * 1024) // 给 128KB，PSRAM 绰绰有余
#define TAG "HA_WS_CLIENT"
#define HA_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJhYTcwZmE1OWQ2NjI0ZDFiYWVkNjFiM2MxZTU3MDdlZSIsImlhdCI6MTc3NzI5Mjk5NywiZXhwIjoyMDkyNjUyOTk3fQ.Z5T5IGwJm6M56h8j40y3HeuLgPwIlMwR1bQ0DxRIinI"
#define ha_ip "192.168.1.143"
static const char* my_target_entities[] = {
    "switch.lemesh_sw0a04_63b3_switch",  // 放入你这个具体的开关实体 ID
};
#define MY_TARGET_COUNT (sizeof(my_target_entities) / sizeof(my_target_entities[0]))

static char *ws_rx_buffer = NULL;

static int msg_id = 1; 
static esp_websocket_client_handle_t client;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
// static void process_ha_json_payload(cJSON *root);
static void send_ha_auth_token(void);
static void* cjson_psram_malloc(size_t size);
static void cjson_psram_free(void* ptr);
static void websocket_reconnect_timer_cb(void* arg);
static void handle_ha_message(cJSON *root);
static void ha_subscribe_entities_states(const char **entity_ids, int count);

static void save_state(void){
    return;
}
// 回调函数
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
   
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "网络链路已建立，等待 HA 认证要求");
            break;

        case WEBSOCKET_EVENT_DATA:
            if (ws_rx_buffer == NULL) {
                ws_rx_buffer = heap_caps_malloc(MAX_WS_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                if (ws_rx_buffer == NULL) {
                    ESP_LOGE(TAG, "PSRAM 缓冲区申请失败！");
                    return;
                }
            }

            if (data->payload_offset + data->data_len <= MAX_WS_BUFFER_SIZE) {
                memcpy(ws_rx_buffer + data->payload_offset, data->data_ptr, data->data_len);
            }

            // 判断是否接收完成：当前偏移 + 当前长度 == 总长度
            if (data->op_code == 0x01 && data->payload_offset + data->data_len == data->payload_len && data->payload_len > 0) {
                ESP_LOGI(TAG, "收到完整包，长度: %d", data->payload_len);
                cJSON *head = cJSON_ParseWithLength(ws_rx_buffer, data->payload_len);
                if (head == NULL) {
                    ESP_LOGE(TAG, "JSON 解析失败！内容预览: %.32s...", ws_rx_buffer);
                    break;
                }else {
                    handle_ha_message(head);
                }
            }         
        break;

            
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "WebSocket 已断开连接,释放接收缓冲区");
            if (ws_rx_buffer) { free(ws_rx_buffer); ws_rx_buffer = NULL; }
            break;
            
        case WEBSOCKET_EVENT_ERROR:
            if (ws_rx_buffer) { free(ws_rx_buffer); ws_rx_buffer = NULL; }
            ESP_LOGE(TAG, "WebSocket 发生错误，释放接收缓冲区");
            break;
        case WEBSOCKET_EVENT_CLOSED:
            if (ws_rx_buffer) { free(ws_rx_buffer); ws_rx_buffer = NULL; }
            ESP_LOGW(TAG, "被 HA 踢出，启动定时器准备重启，并释放接收缓冲区");
    
            // 创建一个单次定时器（10秒后执行）
            esp_timer_handle_t restart_timer;
            const esp_timer_create_args_t timer_args = {
                .callback = &websocket_reconnect_timer_cb,
                .arg = (void*)client,
                .name = "ws_restart_tmr"
            };
            esp_timer_create(&timer_args, &restart_timer);
            esp_timer_start_once(restart_timer, 10000000); // 10,000,000 微秒 = 10 秒
            break;
                    
        case WEBSOCKET_EVENT_FINISH:
            ESP_LOGD(TAG, "WebSocket 任务结束清理");
            break;
        case WEBSOCKET_EVENT_BEFORE_CONNECT:
            ESP_LOGD(TAG, "WebSocket 即将连接，准备工作...");
            break;
        case WEBSOCKET_EVENT_BEGIN:
            ESP_LOGD(TAG, "WebSocket 任务已启动，进入事件循环");
            break;
        default:
            ESP_LOGE(TAG, "其他 WebSocket 事件，ID: %d", event_id);
            break;
    }
}


static void handle_ha_message(cJSON *root){

        cJSON *type_item = cJSON_GetObjectItem(root, "type");
        const char *type = cJSON_IsString(type_item) ? type_item->valuestring : "";
        
        if (strcmp(type, "auth_required") == 0) {
            ESP_LOGI(TAG, "收到 auth_required，开始身份验证...");
            vTaskDelay(pdMS_TO_TICKS(100));
            send_ha_auth_token(); 
        } 
        else if (strcmp(type, "auth_ok") == 0) {
            ESP_LOGI(TAG, "认证成功！请求实体列表...");
            ha_subscribe_entities_states(my_target_entities, MY_TARGET_COUNT); // 订阅所有实体的状态更新
        }
        else if (strcmp(type, "result") == 0) {
            cJSON *id_item = cJSON_GetObjectItem(root, "id");
            if (cJSON_IsNumber(id_item) && id_item->valueint == msg_id - 1) { // 确认这是我们发出的请求的响应
                ESP_LOGI(TAG, "收到 ID为 %d 的结果", msg_id - 1);
                save_state();
            }
            else{
                int other_id = cJSON_IsNumber(id_item) ? id_item->valueint : -1;
                ESP_LOGW(TAG, "收到其他的 result，ID 为: %d", other_id);
            }
        }
        else if (strcmp(type, "event") == 0) {
            cJSON *id_item = cJSON_GetObjectItem(root, "id");
            if (cJSON_IsNumber(id_item) && id_item->valueint == msg_id - 1) { // 确认这是我们发出的请求的响应
                ESP_LOGI(TAG, "收到 ID为 %d 的evet", msg_id - 1);
                save_state();
            }
            else{
                int other_id = cJSON_IsNumber(id_item) ? id_item->valueint : -1;
                ESP_LOGW(TAG, "收到其他的 event，ID 为: %d", other_id);
            }
        }
        else {
            ESP_LOGW(TAG, "收到未处理的消息类型: [%s]", type);
        }
}

static void ha_get_all_states_request() {
    char buf[128];
    // 使用 snprintf 构建 JSON，避免使用动态内存
    int len = snprintf(buf, sizeof(buf), "{\"id\":%d,\"type\":\"get_states\"}", msg_id);
    
    if (esp_websocket_client_is_connected(client)) {
        esp_websocket_client_send_text(client, buf, len, portMAX_DELAY);
        ESP_LOGI(TAG, "已发送获取列表指令 ID: %d", msg_id);
    }
    msg_id++; // 每次调用后 ID 自增，保持唯一性
}

static void ha_subscribe_entities_states(const char **entity_ids, int count) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id);
    cJSON_AddStringToObject(root, "type", "subscribe_trigger");

    // 构建 trigger 数组
    cJSON *trigger = cJSON_CreateObject();
    cJSON_AddStringToObject(trigger, "platform", "state");
    
    cJSON *entity_array = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
        cJSON_AddItemToArray(entity_array, cJSON_CreateString(entity_ids[i]));
    }
    cJSON_AddItemToObject(trigger, "entity_id", entity_array);
    cJSON_AddItemToObject(root, "trigger", trigger);

    // 转换并发送
    char *out = cJSON_PrintUnformatted(root);
    if (out) {
        int len = strlen(out);
        if (esp_websocket_client_is_connected(client)) {
            esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
            ESP_LOGI(TAG, "已发送订阅 %d 个实体的指令，ID: %d", count, msg_id);
        }
        free(out);
    }
    cJSON_Delete(root);
    msg_id++; // ID 自增
}

static void send_ha_auth_token() {
    // 1. 使用 cJSON 构建，确保格式绝对符合标准
    cJSON *auth_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(auth_msg, "type", "auth");
    cJSON_AddStringToObject(auth_msg, "access_token", HA_TOKEN);

    char *out = cJSON_PrintUnformatted(auth_msg);
    if (out) {
        int len = strlen(out);
        esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
        // ESP_LOGI(TAG, "发送认证消息: %s", out); // 调试：确认 JSON 结构是否正确
        free(out);
    }
    cJSON_Delete(auth_msg);
}

// static void process_ha_json_payload(cJSON *root) {
//     // 1. 解析大 JSON (确保你的任务栈或堆空间足够)
//     cJSON *result = cJSON_GetObjectItem(root, "result");
//     if (!cJSON_IsArray(result)) {
//         char *raw_json = cJSON_Print(root);
//             ESP_LOGW(TAG, "消息中不包含有效的 result 数组。原始数据如下：");
//         if (raw_json != NULL) {
//             ESP_LOGW(TAG, "%s", raw_json);
//             free(raw_json); // 别忘了释放 Print 出来的内存
//         }
//         cJSON_Delete(root);
//         return;
//     }
//     ESP_LOGI(TAG, "成功解析 JSON，开始过滤设备...");
//     int size = cJSON_GetArraySize(result);
//     g_device_count = 0; 
//     for (int i = 0; i < size; i++) {
//         cJSON *item = cJSON_GetArrayItem(result, i);      
//         // 安全提取 entity_id
//         cJSON *eid_obj = cJSON_GetObjectItem(item, "entity_id");
//         if (!cJSON_IsString(eid_obj)) continue;
//         const char *eid = eid_obj->valuestring;
//         // 过滤逻辑
//         if (strstr(eid, "light.") || strstr(eid, "switch.")) {
//             if (g_device_count >= 50) {
//                 ESP_LOGW(TAG, "目标设备过多，数组已满，跳过后续目标设备...");
//                 break;
//             }
//             cJSON *attrs = cJSON_GetObjectItem(item, "attributes");
//             const char *fname = eid;
//             if (cJSON_IsObject(attrs)) {
//                 cJSON *fname_obj = cJSON_GetObjectItem(attrs, "friendly_name");
//                 // 如果没有 friendly_name，则回退显示 eid
//                 if (cJSON_IsString(fname_obj)) fname = fname_obj->valuestring;
//             }             
//                // --- 核心存储逻辑 ---
//             strncpy(g_device_list[g_device_count].entity_id, eid, 63);
//             strncpy(g_device_list[g_device_count].friendly_name, fname, 63);
//             g_device_count++;        
//             ESP_LOGI(TAG, "已存入数组: %s", fname);
//             }
//         }
//     // 2. 彻底释放 cJSON 树占用的堆内存
//     cJSON_Delete(root); 
//     ESP_LOGI(TAG, "解析完成，内存已回收。剩余堆内存: %ld", esp_get_free_heap_size());
// }

static void* cjson_psram_malloc(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

static void cjson_psram_free(void* ptr) {
    heap_caps_free(ptr);
}

// 初始化并注册回调
void websocket_app_start() {
    cJSON_Hooks hooks = {
        .malloc_fn = cjson_psram_malloc,
        .free_fn = cjson_psram_free
    };
    cJSON_InitHooks(&hooks);
    char ws_uri_buffer[64] = {0};
    // 2. 动态拼装 WebSocket URL
    snprintf(ws_uri_buffer, sizeof(ws_uri_buffer), "ws://%s:8123/api/websocket", ha_ip);
    ESP_LOGI(TAG, "正在启动 WebSocket 客户端，连接至: %s", ws_uri_buffer);

    const esp_websocket_client_config_t ws_cfg = {
        .uri = ws_uri_buffer,
        .buffer_size = 60080, // 关键点：缓冲区必须大于 42kb，否则大数据包会被截断报错
        .reconnect_timeout_ms = 5000, // 断线重连间隔，10秒
        .network_timeout_ms = 20000,   // 网络超时时间，10秒
        .pingpong_timeout_sec = 60,
    };
    client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}

static void websocket_reconnect_timer_cb(void* arg) {
    esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)arg;
    ESP_LOGI("RECONNECT", "正在从外部任务执行强制重启...");
    esp_websocket_client_stop(client);
    vTaskDelay(pdMS_TO_TICKS(1000)); 
    esp_websocket_client_start(client);
}