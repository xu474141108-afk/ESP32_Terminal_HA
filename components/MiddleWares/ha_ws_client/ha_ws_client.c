#include "esp_websocket_client.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h" 
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_event.h"
#include "custom.h"
#include "ha_ws_client.h"
ESP_EVENT_DEFINE_BASE(HA_ACTION_EVENTS);


#define TAG "HA_WS_CLIENT"
#define MAX_WS_BUFFER_SIZE (128 * 1024) // 给 128KB，PSRAM 绰绰有余



// static void process_ha_json_payload(cJSON *root);
static void* cjson_psram_malloc(size_t size);
static void cjson_psram_free(void* ptr);

typedef struct {
    char *data;
    size_t len;
} ws_event_data_t;

typedef struct {
    const char *entities[4];
    int valid_count;
} ha_ws_entities_t;

static char *ws_rx_buffer = NULL;
static int msg_id = 1; 
static int g_subscribe_trigger_id = 0; 
static int g_subscribe_entities_id = 0; 
static esp_websocket_client_handle_t client;
ha_ws_client_config_t g_ha_ws_client_config={
                .ha_ip = "192.168.1.1",
                .ha_token = ""};

ha_entity_t g_main_ui_device_data[6] =  {0};
ha_device_t g_HAdevice_ctx;


void ha_ws_subscribe_entities(int msg_id, const char **entity_ids, int count) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id);
    cJSON_AddStringToObject(root, "type", "subscribe_entities");

    cJSON *array = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
        cJSON_AddItemToArray(array, cJSON_CreateString(entity_ids[i]));
    }
    cJSON_AddItemToObject(root, "entity_ids", array);

    char *out = cJSON_PrintUnformatted(root);
    // ... 调用 esp_websocket_client_send_text 发送 out ...
    free(out);
    cJSON_Delete(root);
}


static void ha_ws_entity_control(const char *entity_id, const char *service) {
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

static ha_ws_entities_t ha_ws_entities_filtro(const char **entities_input)
{
    ha_ws_entities_t entities_ret = {0};
    if (entities_input == NULL)
    {
        ESP_LOGW(TAG, "传入实体数组为空");
        return entities_ret;
    }
    for (int i = 0; entities_input[i] != NULL && entities_ret.valid_count < 6; i++)
    {
        const char *current_id = entities_input[i];
        if (current_id == NULL || current_id[0] == '\0' || strcmp(current_id, "0") == 0)
        {
            continue;
        }
        bool is_duplicate = false;
        for (int j = 0; j < entities_ret.valid_count; j++)
        {
            if (strcmp(entities_ret.entities[j], current_id) == 0)
            {
                is_duplicate = true;
                break;
            }
        }
        if (!is_duplicate)
        {
            entities_ret.entities[entities_ret.valid_count++] = current_id;
        }
    }

    if (entities_ret.valid_count <= 0)
    {
        ESP_LOGW(TAG, "没有合法可订阅的实体ID");
    }
    return entities_ret;
}

static void ha_ws_trigger_json_send(int msg_id_num, ha_ws_entities_t entities_input)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id_num);
    cJSON_AddStringToObject(root, "type", "subscribe_trigger");

    cJSON *trigger = cJSON_CreateObject();
    cJSON_AddStringToObject(trigger, "platform", "state");

    cJSON *entity_array = cJSON_CreateArray();
    for (int i = 0; i < entities_input.valid_count; i++)
    {
        cJSON_AddItemToArray(entity_array, cJSON_CreateString(entities_input.entities[i]));
    }
    cJSON_AddItemToObject(trigger, "entity_id", entity_array);
    cJSON_AddItemToObject(root, "trigger", trigger);

    char *out = cJSON_PrintUnformatted(root);
    if (out)
    {
        int len = strlen(out);
        if (esp_websocket_client_is_connected(client))
        {
            esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
            ESP_LOGI(TAG, "已发送订阅 %d 个实体trigger指令, ID: %d", entities_input.valid_count, msg_id_num);
            for(int i = 0; i < entities_input.valid_count; i++){
                ESP_LOGI(TAG, "已发送实体id: %s", entities_input.entities[i]);
            }
        }
        free(out);
    }

    cJSON_Delete(root);
}

static void ha_ws_entities_json_send(int msg_id_num, ha_ws_entities_t entities_input)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id_num);
    cJSON_AddStringToObject(root, "type", "subscribe_entities");

    cJSON *entity_array = cJSON_CreateArray();
    for (int i = 0; i < entities_input.valid_count; i++)
    {
        cJSON_AddItemToArray(entity_array, cJSON_CreateString(entities_input.entities[i]));
    }

    cJSON_AddItemToObject(root, "entity_ids", entity_array);

    char *out = cJSON_PrintUnformatted(root);
    if (out)
    {
        int len = strlen(out);
        if (esp_websocket_client_is_connected(client))
        {
            esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
            ESP_LOGI(TAG, "已发送订阅 %d 个实体trigger指令, ID: %d", entities_input.valid_count, msg_id_num);
            for(int i = 0; i < entities_input.valid_count; i++){
                ESP_LOGI(TAG, "已发送实体id: %s", entities_input.entities[i]);
            }
        }
        free(out);
    }

    cJSON_Delete(root);
}

static void ha_ws_convert_trigger_subsc(const ha_entity_t *entity_input)
{
    const char *temp_ptr_arr[7] = {0}; 
    int idx = 0;
    for(int i = 0; i < 6; i++)
    {
        temp_ptr_arr[idx++] = entity_input[i].entity_id;
    }
    temp_ptr_arr[idx] = NULL;
    ha_ws_entities_t entity_ids = ha_ws_entities_filtro(temp_ptr_arr);
    if (entity_ids.valid_count <= 0)
    {
        return;
    }

    ha_ws_trigger_json_send(msg_id, entity_ids);
    g_subscribe_trigger_id = msg_id;
    msg_id++;
}


static void ha_ws_convert_entities_subsc(const ha_entity_t *entity_input){
    const char *temp_ptr_arr[7] = {0}; 
    int idx = 0;
    for(int i = 0; i < 6; i++)
    {
        temp_ptr_arr[idx++] = entity_input[i].entity_id;
    }
    temp_ptr_arr[idx] = NULL;
    ha_ws_entities_t entity_ids = ha_ws_entities_filtro(temp_ptr_arr);
    if (entity_ids.valid_count <= 0)
    {
        return;
    }

    ha_ws_entities_json_send(msg_id, entity_ids);
    g_subscribe_trigger_id = msg_id;
    msg_id++;
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

void ha_ws_message_update_handle(cJSON *root) {
    if (!cJSON_GetObjectItem(root, "event")) return;

    cJSON *id_item = cJSON_GetObjectItem(root, "id");
    ESP_LOGE(TAG,"当前id: %d , t_id: %d, e_id: %d", id_item->valueint,g_subscribe_trigger_id,g_subscribe_entities_id);
    const char *e_id = NULL;
    const char *new_state = NULL;
 
    if (cJSON_GetObjectItemByPath(root, "event.variables.trigger")) {
        cJSON *id_node = cJSON_GetObjectItemByPath(root, "event.variables.trigger.entity_id");
        cJSON *state_node = cJSON_GetObjectItemByPath(root, "event.variables.trigger.to_state.state");
        if (id_node && state_node) {
            e_id = id_node->valuestring;
            new_state = state_node->valuestring;
            ESP_LOGI(TAG, "Trigger: %s %s", e_id, new_state);
        }
    } else if(cJSON_GetObjectItemByPath(root, "event.data")){
        cJSON *id_node = cJSON_GetObjectItem(root, "event.data.entity_id");
        cJSON *state_node = cJSON_GetObjectItemByPath(root, "event.data.new_state.state");
        if (id_node && state_node) {
            e_id = id_node->valuestring;
            new_state = state_node->valuestring;
            ESP_LOGI(TAG, "Entities: %s %s", e_id, new_state);
        }
        
    } else if(cJSON_GetObjectItemByPath(root, "event.c")){
        cJSON *target_node = cJSON_GetObjectItemByPath(root, "event.c");
        if (target_node) {
            cJSON *item = target_node->child;
            while (item) {
                e_id = item->string;
                cJSON *plus_node = cJSON_GetObjectItem(item, "+");
                if (plus_node) {
                    cJSON *state_node = cJSON_GetObjectItem(plus_node, "s");
                    new_state = state_node->valuestring;
                    if (new_state) {
                        ESP_LOGI(TAG, "实体[%s] 状态更新为: %s", e_id, new_state);
                    }
                }
                item = item->next;
            }
        }
    }else if(cJSON_GetObjectItemByPath(root, "event.a")){
        cJSON *target_node = cJSON_GetObjectItemByPath(root, "event.a");
        if (target_node) {
            cJSON *item = target_node->child;
            while (item) {
                e_id = item->string;
                cJSON *state_node = cJSON_GetObjectItem(item, "s");
                new_state =  state_node->valuestring;
                if (new_state) {
                    ESP_LOGI(TAG, "实体[%s] 状态更新为: %s", e_id, new_state);
                }

                item = item->next;
            }
        }
    }else{
        ESP_LOGI(TAG, "Error parsing JSON");
        return;
    }

    for (int i = 0; i < 6; i++) {
        if (strcmp(g_main_ui_device_data[i].entity_id, e_id) == 0) {
            strncpy(g_main_ui_device_data[i].state, new_state, sizeof(g_main_ui_device_data[i].state) - 1);
            g_main_ui_device_data[i].state[sizeof(g_main_ui_device_data[i].state) - 1] = '\0';
            ESP_LOGI(TAG, "更新实体状态: %s -> %s",g_main_ui_device_data[i].entity_id,g_main_ui_device_data[i].state);
        }else{
            ESP_LOGI(TAG, "数组元素%d 与收到的id 不匹配%s",i,e_id);
        }
    }

}

static void ha_ws_message_handle(cJSON *root){

    cJSON *type_item = cJSON_GetObjectItem(root, "type");
    const char *type = cJSON_IsString(type_item) ? type_item->valuestring : "";
    
    if (strcmp(type, "auth_required") == 0) {
        ESP_LOGI(TAG, "Recieved auth_required");
        esp_event_post(HA_ACTION_EVENTS, HA_SYSTEM_CONNECTED, NULL, 0, portMAX_DELAY); 
        vTaskDelay(pdMS_TO_TICKS(100));
        ha_auth_token_send();
    } 
    else if (strcmp(type, "auth_ok") == 0) {
        ESP_LOGI(TAG, "Autorised, subscribe entities states");      
        // ha_ws_entity_convert_subsc(g_main_ui_device_data);
        ha_ws_convert_entities_subsc(g_main_ui_device_data);
    }
    else if (strcmp(type, "auth_invalid") == 0) {
        ESP_LOGE(TAG, "Autorisation failed, will be closed");
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
            ha_ws_message_update_handle(root);
    }
    else {
        ESP_LOGW(TAG, "Non-expected message type: [%s]", type);
    }
    cJSON_Delete(root);
}

static void ha_ws_logic_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    switch (id) {
    case LVGL_WS_BUTTON_RT_TOGGLE:{
        ESP_LOGI(TAG, "RT BUTTON PRESSED");
        const char *entity_id = (const char *)g_main_ui_device_data[0].entity_id;
        if (entity_id == NULL || *entity_id == '\0') {
            ESP_LOGE(TAG, "非法实体ID");
            break;
        }
        ESP_LOGI(TAG, "Toggle entity: %s", entity_id);
        ha_ws_entity_control(entity_id, "toggle");
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
        ha_ws_entity_control(entity_id, "toggle");
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
        ha_ws_entity_control(entity_id, "toggle");
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
        ha_ws_entity_control(entity_id, "toggle");
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
            ha_ws_message_handle(head);
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
static void ha_ws_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
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
        esp_event_handler_register(HA_ACTION_EVENTS, ESP_EVENT_ANY_ID, ha_ws_logic_handler, NULL);
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
        esp_websocket_unregister_events(client, WEBSOCKET_EVENT_ANY, ha_ws_event_handler);
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
        .buffer_size = 60000, 
        .reconnect_timeout_ms = 15000, // 断线重连间
        .network_timeout_ms = 20000,   // 网络超时时间
        .pingpong_timeout_sec = 120,
    };
    client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, ha_ws_event_handler, (void *)client);
    esp_websocket_client_start(client);
}

static void* cjson_psram_malloc(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

static void cjson_psram_free(void* ptr) {
    heap_caps_free(ptr);
}



void get_ha_states_to_psram() {
    g_HAdevice_ctx.state_ha = HA_STATE_SEARCHING;
    cJSON_Hooks hooks = {
        .malloc_fn = cjson_psram_malloc,
        .free_fn = cjson_psram_free
    };
    cJSON_InitHooks(&hooks);
    char api_uri_buffer[64] = {0};
    snprintf(api_uri_buffer, sizeof(api_uri_buffer), "http://%s:8123/api/states", g_ha_ws_client_config.ha_ip);
    esp_http_client_config_t config = {
        .url = api_uri_buffer,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 8000, 
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t http_HA_client = esp_http_client_init(&config);

    char auth_header[512]; 
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", g_ha_ws_client_config.ha_token);
    esp_http_client_set_header(http_HA_client, "Authorization", auth_header);
    esp_http_client_set_header(http_HA_client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_open(http_HA_client,0);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "连接失败: %s", esp_err_to_name(err));
        if (client != NULL) {
            esp_http_client_close(http_HA_client);
            esp_http_client_cleanup(http_HA_client);
            client = NULL;
        }
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }

    int content_length = esp_http_client_fetch_headers(http_HA_client);
    if (content_length <= 0 ||content_length > 500 * 1024) {
        ESP_LOGE(TAG, "内容长度超出合理范围 %d",content_length);
        if (client != NULL) {
            esp_http_client_close(http_HA_client);
            esp_http_client_cleanup(http_HA_client);
            client = NULL;
        }
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }
    
    // 直接在 PSRAM 中申请长度对应的内存
    char *buffer = heap_caps_malloc(content_length + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "PSRAM 申请失败！");
        if (client != NULL) {
            esp_http_client_close(http_HA_client);
            esp_http_client_cleanup(http_HA_client);
            client = NULL;
        }
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }

    int read_len = esp_http_client_read(http_HA_client, buffer, content_length);
    buffer[read_len] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    if (root && cJSON_IsArray(root)) {
        int array_size = cJSON_GetArraySize(root);
        ESP_LOGI(TAG, "JSON 解析成功，实体总数: %d", array_size);
        
        g_HAdevice_ctx.device_count = 0;

        for (int i = 0; i < array_size; i++) {
            cJSON *item = cJSON_GetArrayItem(root, i);
            cJSON *eid_obj = cJSON_GetObjectItem(item, "entity_id");

            if (!cJSON_IsString(eid_obj)) continue;
            const char *eid = eid_obj->valuestring;

            // 筛选条件：只取 light 和 switch 设备
            if (strstr(eid, "light.") || strstr(eid, "switch.")||strstr(eid, "input_button.")) {
                if (g_HAdevice_ctx.device_count >= 50) break;

                cJSON *attrs = cJSON_GetObjectItem(item, "attributes");
                const char *fname = eid;

                if (cJSON_IsObject(attrs)) {
                    cJSON *fname_obj = cJSON_GetObjectItem(attrs, "friendly_name");
                    if (cJSON_IsString(fname_obj)) fname = fname_obj->valuestring;
                }

                // 存入全局数组
                strncpy(g_HAdevice_ctx.entity[g_HAdevice_ctx.device_count].entity_id, eid, 63);
                strncpy(g_HAdevice_ctx.entity[g_HAdevice_ctx.device_count].friendly_name, fname, 63);
                g_HAdevice_ctx.device_count++;
            }
        }
        g_HAdevice_ctx.state_ha = HA_STATE_READY;
        
    }
    else{
        ESP_LOGI(TAG, "JSON 解析失败");
        g_HAdevice_ctx.state_ha = HA_STATE_JSON_ERROR;
    }

    if(root){
           cJSON_Delete(root); 
    }
    heap_caps_free(buffer); 
    if (client != NULL) {
        esp_http_client_close(http_HA_client);
        esp_http_client_cleanup(http_HA_client);
        client = NULL;
    }
}