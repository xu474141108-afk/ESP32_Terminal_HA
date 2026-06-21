#include "esp_websocket_client.h"
#include "esp_log.h"
#include "cJSON.h" 
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_event.h"
#include "custom.h"
#include "app_test.h"
#include "ha_http_req.h"
#include "ha_ws_client.h"
ESP_EVENT_DEFINE_BASE(HA_ACTION_EVENTS);


#define TAG "HA_WS_CLIENT"
#define MAX_WS_BUFFER_SIZE (128 * 1024) // 给 128KB，PSRAM 绰绰有余


static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
// static void process_ha_json_payload(cJSON *root);
static void* cjson_psram_malloc(size_t size);
static void cjson_psram_free(void* ptr);
static void handle_ha_message(cJSON *root);

typedef struct {
    char *data;
    size_t len;
} ws_event_data_t;


static char *ws_rx_buffer = NULL;
static int msg_id = 1; 
static int g_subscribe_trigger_id = 0; 
static int g_subscribe_entities_id = 0; 
static esp_websocket_client_handle_t client;
ha_ws_client_config_t g_ha_ws_client_config={
                .ha_ip = "192.168.1.1",
                .ha_token = ""
            };





void ws_entity_control(const char *entity_id, const char *service) {
    char domain[32];
    const char *dot = strchr(entity_id, '.');
    if (dot) {
        size_t len = dot - entity_id;
        if (len >= sizeof(domain)) len = sizeof(domain) - 1;
        strncpy(domain, entity_id, len);
        domain[len] = '\0'; // 确保字符串结束
    } else {
        strcpy(domain, "switch"); // 默认值
    }


    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id++);
    cJSON_AddStringToObject(root, "type", "call_service");
    cJSON_AddStringToObject(root, "domain", domain);
    cJSON_AddStringToObject(root, "service", service); // 例如 "turn_on" 或 "toggle"
    
    cJSON *target = cJSON_CreateObject();
    cJSON_AddStringToObject(target, "entity_id", entity_id);
    cJSON_AddItemToObject(root, "target", target);

    char *out = cJSON_PrintUnformatted(root);
    if (esp_websocket_client_is_connected(client)) {
        esp_websocket_client_send_text(client, out, strlen(out), portMAX_DELAY);
        ESP_LOGI(TAG, "已发送控制指令: %s.%s -> %s", domain, service, entity_id);
    }
    free(out);
    cJSON_Delete(root);
}

void ha_entity_subscribe_entities() {
    const char *clean_ids[4]; 
    int valid_count = 0;

    for (int i = 0; i < 4; i++) {
        const char *current_id = g_main_ui_device_data[i].entity_id;
        if (current_id[0] == '\0' || strcmp(current_id, "0") == 0) {
            continue;
        }
        bool is_duplicate = false;
        for (int j = 0; j < valid_count; j++) {
            if (strcmp(clean_ids[j], current_id) == 0) {
                is_duplicate = true;
                break;
            }
        }

        if (!is_duplicate && valid_count < 4) {
            clean_ids[valid_count++] = current_id;
        }
    }

    if (valid_count <= 0) {return;}
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id);
    cJSON_AddStringToObject(root, "type", "subscribe_entities");

    // 构建 trigger 数组
    
    cJSON *entity_array = cJSON_CreateArray();
    for (int i = 0; i < valid_count; i++) {
        cJSON_AddItemToArray(entity_array, cJSON_CreateString(clean_ids[i]));
    }
    cJSON_AddItemToObject(root, "entity_ids", entity_array);

    // 转换并发送
    char *out = cJSON_PrintUnformatted(root);
    if (out) {
        int len = strlen(out);
        if (esp_websocket_client_is_connected(client)) {
            esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
            ESP_LOGI(TAG, "已发送订阅 %d 个实体的指令, ID: %d", valid_count, msg_id);
        }
        free(out);
    }
    cJSON_Delete(root);
    g_subscribe_entities_id = msg_id;
    msg_id++; // ID 自增
}

void ha_entity_subscribe_trigger() {
    const char *clean_ids[4]; 
    int valid_count = 0;

    for (int i = 0; i < 4; i++) {
        const char *current_id = g_main_ui_device_data[i].entity_id;
        if (current_id[0] == '\0' || strcmp(current_id, "0") == 0) {
            continue;
        }
        bool is_duplicate = false;
        for (int j = 0; j < valid_count; j++) {
            if (strcmp(clean_ids[j], current_id) == 0) {
                is_duplicate = true;
                break;
            }
        }

        if (!is_duplicate && valid_count < 4) {
            clean_ids[valid_count++] = current_id;
        }
    }

    if (valid_count <= 0) {return;}
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id);
    cJSON_AddStringToObject(root, "type", "subscribe_trigger");

    // 构建 trigger 数组
    cJSON *trigger = cJSON_CreateObject();
    cJSON_AddStringToObject(trigger, "platform", "state");
    
    cJSON *entity_array = cJSON_CreateArray();
    for (int i = 0; i < valid_count; i++) {
        cJSON_AddItemToArray(entity_array, cJSON_CreateString(clean_ids[i]));
    }
    cJSON_AddItemToObject(trigger, "entity_id", entity_array);
    cJSON_AddItemToObject(root, "trigger", trigger);

    // 转换并发送
    char *out = cJSON_PrintUnformatted(root);
    if (out) {
        int len = strlen(out);
        if (esp_websocket_client_is_connected(client)) {
            esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
            ESP_LOGI(TAG, "已发送订阅 %d 个实体的指令, ID: %d", valid_count, msg_id);
        }
        free(out);
    }
    cJSON_Delete(root);
    g_subscribe_trigger_id = msg_id;
    msg_id++; // ID 自增
}

static cJSON* cJSON_GetObjectItemByPath(cJSON* root, const char* path) {
    char *path_copy = strdup(path); // 复制路径字符串以进行切割
    char *token = strtok(path_copy, ".");
    cJSON *current = root;
    
    while (token != NULL && current != NULL) {
        current = cJSON_GetObjectItem(current, token);
        token = strtok(NULL, ".");
    }
    
    free(path_copy);
    return current;
}

static void ha_auth_token_send() {
    cJSON *auth_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(auth_msg, "type", "auth");
    cJSON_AddStringToObject(auth_msg, "access_token", g_ha_ws_client_config.ha_token);
    ESP_LOGI(TAG, "发送认证: %s", g_ha_ws_client_config.ha_token);
    char *out = cJSON_PrintUnformatted(auth_msg);
    if (out) {
        int len = strlen(out);

        esp_err_t err = esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "认证信息已发送");
        } else {
            ESP_LOGE(TAG, "发送认证失败，错误码: %d", err);
        }
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


void ha_process_state_update(cJSON *root) {
    ESP_LOGI(TAG, "开始解析状态更新");
    cJSON *event = cJSON_GetObjectItem(root, "event");
    if (!event) return;
    const char *e_id = NULL;
    const char *new_state = NULL;
 
    // --- 路径判断逻辑 ---
    ESP_LOGE("update", "paso 4");
    if (cJSON_GetObjectItemByPath(root, "event.variables.trigger")) {
        ESP_LOGE("update", "这是 Trigger 模式");
        // 这是 Trigger 模式
        cJSON *id_node = cJSON_GetObjectItemByPath(root, "event.variables.trigger.entity_id");
        cJSON *state_node = cJSON_GetObjectItemByPath(root, "event.variables.trigger.to_state.state");
        if (id_node && state_node) {
            e_id = id_node->valuestring;
            new_state = state_node->valuestring;
            ESP_LOGI(TAG, "Trigger: %s %s", e_id, new_state);
        }
    } else if(cJSON_GetObjectItemByPath(root, "event.data")){
        ESP_LOGE("update", "这是 Trigger 模式");
        cJSON *id_node = cJSON_GetObjectItem(root, "event.data.entity_id");
        cJSON *state_node = cJSON_GetObjectItemByPath(root, "event.data.new_state.state");
        if (id_node && state_node) {
            e_id = id_node->valuestring;
            new_state = state_node->valuestring;
            ESP_LOGI(TAG, "Entities: %s %s", e_id, new_state);
        }
    }else{
        ESP_LOGE("update", "这是其他模式");
        return;
    }

    // --- 执行更新逻辑 ---
    for (int i = 0; i < 4; i++) {
        if (strcmp(g_main_ui_device_data[i].entity_id, e_id) == 0) {
            bool new_is_on = (strcmp(new_state, "on") == 0);
                if (g_main_ui_device_data[i].is_on != new_is_on)
            {
                g_main_ui_device_data[i].is_on = new_is_on;
                ESP_LOGI(TAG, "数组元素%d,Changed: %s -> %s",i ,e_id, new_state);
            }else{
                ESP_LOGI(TAG, "数组元素%d,No change: %s -> %s",i ,e_id, new_state);
            }
        }else{
                ESP_LOGI(TAG, "数组元素%d 与收到的id 不匹配%s",i,e_id);
        }
    }
}


static void handle_ha_message_siemple(cJSON *root){

    cJSON *type_item = cJSON_GetObjectItem(root, "type");
    const char *type = cJSON_IsString(type_item) ? type_item->valuestring : "";
    
    if (strcmp(type, "auth_required") == 0) {
        ESP_LOGI(TAG, "Recieved auth_required");
        esp_event_post(HA_ACTION_EVENTS, HA_SYSTEM_CONNECTED, NULL, 0, portMAX_DELAY); 
        ha_auth_token_send();
    } 
    else if (strcmp(type, "auth_ok") == 0) {
        ESP_LOGI(TAG, "Autorised, subscribe entities states");
        ha_entity_subscribe_trigger();
        // ha_entity_subscribe_entities();
    }
    else if (strcmp(type, "result") == 0) {
        cJSON *id_item = cJSON_GetObjectItem(root, "id");
        if (cJSON_IsNumber(id_item) && id_item->valueint == msg_id - 1) { // 确认这是我们发出的请求的响应
            ESP_LOGI(TAG, "Recieved result, ID为 %d", msg_id - 1);
        }
        else{
            int other_id = cJSON_IsNumber(id_item) ? id_item->valueint : -1;
            ESP_LOGW(TAG, "Revieved other result, ID: %d", other_id);
        }
    }
    else if (strcmp(type, "event") == 0) {
            cJSON *id_item = cJSON_GetObjectItem(root, "id");
            ESP_LOGE(TAG,"当前id: %d , t_id: %d, e_id: %d", id_item->valueint,g_subscribe_trigger_id,g_subscribe_entities_id);
            ha_process_state_update(root);
    }
    else {
        ESP_LOGW(TAG, "Non-expected message type: [%s]", type);
    }
    cJSON_Delete(root);
}


//数据处理函数
static void handle_ha_message(cJSON *root){

    cJSON *type_item = cJSON_GetObjectItem(root, "type");
    const char *type = cJSON_IsString(type_item) ? type_item->valuestring : "";
    
    if (strcmp(type, "auth_required") == 0) {
        ESP_LOGI(TAG, "Recieved auth_required");
        esp_event_post(HA_ACTION_EVENTS, HA_SYSTEM_CONNECTED, NULL, 0, portMAX_DELAY); 
        ha_auth_token_send();
    } 
    else if (strcmp(type, "auth_ok") == 0) {
        ESP_LOGI(TAG, "Autorised, subscribe entities states");
        ha_entity_subscribe_trigger();
    }
    else if (strcmp(type, "result") == 0) {
        cJSON *id_item = cJSON_GetObjectItem(root, "id");
        if (cJSON_IsNumber(id_item) && id_item->valueint == msg_id - 1) { // 确认这是我们发出的请求的响应
            ESP_LOGI(TAG, "Recieved result, ID为 %d", msg_id - 1);
        }
        else{
            int other_id = cJSON_IsNumber(id_item) ? id_item->valueint : -1;
            ESP_LOGW(TAG, "Revieved other result, ID: %d", other_id);
        }
    }
    else if (strcmp(type, "event") == 0) {
        cJSON *id_item = cJSON_GetObjectItem(root, "id");
        if (cJSON_IsNumber(id_item) && id_item->valueint == g_subscribe_trigger_id){
            ESP_LOGI(TAG, "Recieved event, ID: %d ", g_subscribe_trigger_id);
            cJSON *id_node = cJSON_GetObjectItemByPath(root, "event.variables.trigger.entity_id");
            cJSON *state_node = cJSON_GetObjectItemByPath(root, "event.variables.trigger.to_state.state");
            if (state_node && id_node) {
                const char *new_state = state_node->valuestring;
                const char *e_id = id_node->valuestring;

                for (int i = 0; i < 4; i++) {
                    if (strcmp(g_main_ui_device_data[i].entity_id, e_id) == 0) {
                        bool new_is_on = (strcmp(new_state, "on") == 0);
                         if (g_main_ui_device_data[i].is_on != new_is_on)
                        {
                            g_main_ui_device_data[i].is_on = new_is_on;
                            ESP_LOGI(TAG, "数组元素%d,Changed: %s -> %s",i ,e_id, new_state);
                        }else{
                            ESP_LOGI(TAG, "数组元素%d,No change: %s -> %s",i ,e_id, new_state);
                        }
                    }else{
                            ESP_LOGI(TAG, "数组元素%d 与收到的id 不匹配%s",i,e_id);
                    }
                }
            }
            else{
                ESP_LOGE(TAG,"new_state / e_id is null");
            }
        }
        else if (cJSON_IsNumber(id_item) && id_item->valueint == g_subscribe_entities_id){
            ESP_LOGI(TAG, "Recieved event, ID: %d ", g_subscribe_entities_id);
            cJSON *id_node = cJSON_GetObjectItemByPath(root, "event.variables.trigger.entity_id");
            cJSON *state_node = cJSON_GetObjectItemByPath(root, "event.variables.trigger.to_state.state");
            if (state_node && id_node) {
                const char *new_state = state_node->valuestring;
                const char *e_id = id_node->valuestring;

                for (int i = 0; i < 4; i++) {
                    if (strcmp(g_main_ui_device_data[i].entity_id, e_id) == 0) {
                        bool new_is_on = (strcmp(new_state, "on") == 0);
                         if (g_main_ui_device_data[i].is_on != new_is_on)
                        {
                            g_main_ui_device_data[i].is_on = new_is_on;
                            ESP_LOGI(TAG, "数组元素%d,Changed: %s -> %s",i ,e_id, new_state);
                        }else{
                            ESP_LOGI(TAG, "数组元素%d,No change: %s -> %s",i ,e_id, new_state);
                        }
                    }else{
                            ESP_LOGI(TAG, "数组元素%d 与收到的id 不匹配%s",i,e_id);
                    }
                }
            }
            else{
                ESP_LOGE(TAG,"new_state / e_id is null");
            }
        }
        else{
            int other_id = cJSON_IsNumber(id_item) ? id_item->valueint : -1;
            ESP_LOGW(TAG, "Recieved other event, ID: %d", other_id);
        }
    }
    else {
        ESP_LOGW(TAG, "Non-expected message type: [%s]", type);
    }
    cJSON_Delete(root);
}

static void websocket_logic_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    switch (id) {
    case LVGL_WS_BUTTON_RT_TOGGLE:{
        ESP_LOGI(TAG, "RT BUTTON PRESSED");
        const char *entity_id = (const char *)g_main_ui_device_data[0].entity_id;
        if (entity_id == NULL || *entity_id == '\0') {
            ESP_LOGE(TAG, "非法实体ID");
            break;
        }
        ESP_LOGI(TAG, "Toggle entity: %s", entity_id);
        ws_entity_control(entity_id, "toggle");
        break;
    }
       
    
    case LVGL_WS_BUTTON_RM_TOGGLE: {
        ESP_LOGI(TAG, "RT BUTTON PRESSED");
        const char *entity_id = (const char *)g_main_ui_device_data[1].entity_id;
        if (entity_id == NULL || *entity_id == '\0') {
            ESP_LOGE(TAG, "非法实体ID");
            break;
        }
        ESP_LOGI(TAG, "Toggle entity: %s", entity_id);
        ws_entity_control(entity_id, "toggle");
        break;
    }
       

    case LVGL_WS_BUTTON_RD_TOGGLE:{
        ESP_LOGI(TAG, "RT BUTTON PRESSED");
        const char *entity_id = (const char *)g_main_ui_device_data[2].entity_id;
        if (entity_id == NULL || *entity_id == '\0') {
            ESP_LOGE(TAG, "非法实体ID");
            break;
        }
        ESP_LOGI(TAG, "Toggle entity: %s", entity_id);
        ws_entity_control(entity_id, "toggle");
        break;
    }
       

    case LVGL_WS_BUTTON_MD_TOGGLE:{
        ESP_LOGI(TAG, "RT BUTTON PRESSED");
        const char *entity_id = (const char *)g_main_ui_device_data[3].entity_id;
        if (entity_id == NULL || *entity_id == '\0') {
            ESP_LOGE(TAG, "非法实体ID");
            break;
        }
        ESP_LOGI(TAG, "Toggle entity: %s", entity_id);
        ws_entity_control(entity_id, "toggle");
        break;
    }
       

    case HA_DATA_RECEIVED:
        ws_event_data_t *ha_data = (ws_event_data_t *)event_data; 
        cJSON *head = cJSON_ParseWithLength(ha_data->data, ha_data->len);
        if (head == NULL) {
            ESP_LOGE(TAG, "JSON anal failed: %.32s...", ws_rx_buffer);
            break;
        }else {
            ESP_LOGI(TAG, "--- 即将解析数据 (Len: %d) ---", ha_data->len);
            handle_ha_message_siemple(head);
        }   
    break;

    case HA_SYSTEM_CONNECTED: 
        // ha_auth_token_send();
        break;
    case HA_WS_STATE_KICKED_OUT:
        ESP_LOGI(TAG, "Restart");

        if (client) {
            esp_websocket_client_stop(client);
            esp_websocket_client_destroy(client);
            client = NULL;
        }
        websocket_app_start(); 
    break;   

    case HA_TOKEN_IP_UPDATE:
        ESP_LOGI(TAG, "HA_TOKEN_IP_UPDATE, Restart");
        if (client) {
            esp_websocket_client_stop(client);
            esp_websocket_client_destroy(client);
            client = NULL;
        }
        websocket_app_start(); // 直接调用初始化
    break;
    case LVGL_REQ_ALL_DATA:
        ESP_LOGI(TAG,"LVGL_REQ_ALL_DATA");
    }
}

// 回调函数
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
   
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "等待HA认证");
            break;

        case WEBSOCKET_EVENT_DATA:
            if (ws_rx_buffer == NULL) {
                ws_rx_buffer = heap_caps_malloc(MAX_WS_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                if (ws_rx_buffer == NULL) {
                    return;
                }
            }
            if (data->payload_offset + data->data_len <= MAX_WS_BUFFER_SIZE) {
                memcpy(ws_rx_buffer + data->payload_offset, data->data_ptr, data->data_len);
            }
            // 判断是否接收完成：当前偏移 + 当前长度 == 总长度
            if (data->op_code == 0x01 && data->payload_offset + data->data_len == data->payload_len && data->payload_len > 0) {
                ws_event_data_t event_payload = {
                    .data = ws_rx_buffer,
                    .len = data->payload_len
                };
                esp_event_post(HA_ACTION_EVENTS, HA_DATA_RECEIVED, &event_payload, sizeof(ws_event_data_t), portMAX_DELAY);
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
            esp_event_post(HA_ACTION_EVENTS, HA_WS_STATE_KICKED_OUT, NULL, 0, portMAX_DELAY);
            break;
                    
        case WEBSOCKET_EVENT_FINISH:
            break;
        case WEBSOCKET_EVENT_BEFORE_CONNECT:
            break;
        case WEBSOCKET_EVENT_BEGIN:
            break;
        default:
            ESP_LOGE(TAG, "其他 WebSocket 事件, ID: %d", event_id);
            break;
    }
}

// 初始化并注册回调
void websocket_app_start() {
    vTaskDelay(pdMS_TO_TICKS(2000));

    static bool handler_registered = false;
    if (!handler_registered) {
        esp_event_handler_register(HA_ACTION_EVENTS, ESP_EVENT_ANY_ID, websocket_logic_handler, NULL);
        handler_registered = true;
    }

    cJSON_Hooks hooks = {
        .malloc_fn = cjson_psram_malloc,
        .free_fn = cjson_psram_free
    };
    cJSON_InitHooks(&hooks);

    if (client != NULL)
    {
        // 停止WebSocket连接
        esp_websocket_client_stop(client);
        // 注销事件监听（可选，规范操作）
        esp_websocket_unregister_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler);
        // 销毁客户端，释放内存
        esp_websocket_client_destroy(client);
        client = NULL;
    }
    char ws_uri_buffer[64] = {0};
    // 2. 动态拼装 WebSocket URL
    snprintf(ws_uri_buffer, sizeof(ws_uri_buffer), "ws://%s:8123/api/websocket", g_ha_ws_client_config.ha_ip);
    ESP_LOGI(TAG, "正在启动 WebSocket 客户端，连接至: %s", ws_uri_buffer);

    const esp_websocket_client_config_t ws_cfg = {
        .uri = ws_uri_buffer,
        .buffer_size = 60000, // 关键点：缓冲区必须大于 42kb，否则大数据包会被截断报错
        .reconnect_timeout_ms = 10000, // 断线重连间隔，10秒
        .network_timeout_ms = 20000,   // 网络超时时间，10秒
        .pingpong_timeout_sec = 120,
    };
    client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}

static void* cjson_psram_malloc(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

static void cjson_psram_free(void* ptr) {
    heap_caps_free(ptr);
}