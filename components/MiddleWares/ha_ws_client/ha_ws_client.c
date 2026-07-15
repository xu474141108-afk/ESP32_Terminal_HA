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
#include "esp_wifi.h"
ESP_EVENT_DEFINE_BASE(HA_ACTION_EVENTS);


#define TAG "HA_WS_CLIENT"
#define MAX_WS_BUFFER_SIZE (128 * 1024) 
#define MAX_COUT 6
//TEST
#define SAMPLE_NUM 100
#define PYTHON_SERVER_URL "http://192.168.1.136:8000/upload" 
static void ws_start_delay_test_packet(void);
//TEST
// static void process_ha_json_payload(cJSON *root);
static void* cjson_psram_malloc(size_t size);
static void cjson_psram_free(void* ptr);
static void trigger_reconnect_timer(esp_websocket_client_handle_t ws_client);

typedef struct {
    char *data;
    size_t len;
} ws_event_data_t;

typedef struct {
    const char *entities[4];
    int valid_count;
} ha_ws_entities_t;

// test
float single_delay_buf[SAMPLE_NUM];
int rssi_buf[SAMPLE_NUM];
uint8_t test_scene = 1;         //1=近距离无遮挡 2=隔墙远距离
bool test_finish_flag = false;  //采集完成标志
uint16_t sample_index = 0;
int64_t send_ts_us;             //测试包发送时间戳
// test

static char *ws_rx_buffer = NULL;
static int msg_id = 1; 
static int g_subscribe_entities_id = 0; 
static esp_websocket_client_handle_t client;
ha_ws_client_config_t g_ha_ws_client_config={
                .ha_ip = "192.168.1.1",
                .ha_token = ""};

ha_entity_t g_main_ui_device_data[MAX_COUT] =  {0};
ha_device_t g_HAdevice_ctx;
weather_data_t entity_weather_data = {.temp = "0", .hum = "0"};


static void ha_ws_subscribe_all_events(int msg_id_num)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id_num);
    cJSON_AddStringToObject(root, "type", "subscribe_events");
    char *out = cJSON_PrintUnformatted(root);
    if (esp_websocket_client_is_connected(client)) {
        esp_websocket_client_send_text(client, out, strlen(out), portMAX_DELAY);
    }
    free(out);
    cJSON_Delete(root);
}


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
        domain[len] = '\0'; 
    } else {
        strcpy(domain, "switch"); //default
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id++);
    cJSON_AddStringToObject(root, "type", "call_service");
    cJSON_AddStringToObject(root, "domain", domain);
    cJSON_AddStringToObject(root, "service", service); 
    cJSON *target = cJSON_CreateObject();
    cJSON_AddStringToObject(target, "entity_id", entity_id);
    cJSON_AddItemToObject(root, "target", target);
    char *out = cJSON_PrintUnformatted(root);
    if (esp_websocket_client_is_connected(client)) {
        esp_websocket_client_send_text(client, out, strlen(out), portMAX_DELAY);
    }
    free(out);
    cJSON_Delete(root);
}

static ha_ws_entities_t ha_ws_entities_filtro(const char **entities_input)
{
    ha_ws_entities_t entities_ret = {0};
    if (entities_input == NULL)
    {
        ESP_LOGW(TAG, "entities_input is NULL");
        return entities_ret;
    }
    for (int i = 0; entities_input[i] != NULL && entities_ret.valid_count < MAX_COUT; i++)
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
        ESP_LOGW(TAG, "no entities found");
    }
    ESP_LOGI(TAG, "entities_ret.valid_count: %d", entities_ret.valid_count);
    return entities_ret;
}

static void ha_ws_entities_json_send(int msg_id_num, ha_ws_entities_t entities_input)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", msg_id_num);
    cJSON_AddStringToObject(root, "type", "subscribe_entities");
    cJSON *entity_array = cJSON_CreateArray();
    ESP_LOGE("ERROR", "entities_input count: %d", entities_input.valid_count);
    for (int i = 0; i < entities_input.valid_count-1; i++)
    {
        ESP_LOGE("ERROR", "ERROR SITIO 3: %s,%d", entities_input.entities[i],i);
        cJSON_AddItemToArray(entity_array, cJSON_CreateString(entities_input.entities[i]));
    }
    ESP_LOGE("ERROR", "ERROR SITIO 4");
    cJSON_AddItemToObject(root, "entity_ids", entity_array);
    ESP_LOGE("ERROR", "ERROR SITIO 5");
    char *out = cJSON_PrintUnformatted(root);
    if (out)
    {
        int len = strlen(out);
        if (esp_websocket_client_is_connected(client))
        {
            esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
            for(int i = 0; i < entities_input.valid_count; i++){
            }
        }
        free(out);
    }

    cJSON_Delete(root);
}


static void ha_ws_convert_entities_subsc(const ha_entity_t *entity_input){
    const char *temp_ptr_arr[MAX_COUT+1] = {0}; 
    int idx = 0;
    for(int i = 0; i < MAX_COUT; i++)
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
    g_subscribe_entities_id = msg_id;
    msg_id++;
}

static cJSON* cJSON_GetObjectItemByPath(cJSON* root, const char* path) {
    char *path_copy = strdup(path); 
    char *token = strtok(path_copy, ".");
    cJSON *current = root;
    
    while (token != NULL && current != NULL) {
        current = cJSON_GetObjectItem(current, token);
        token = strtok(NULL, ".");
    }
    
    free(path_copy);
    return current;
}

static void ha_auth_token_send(void)
{
    if (!esp_websocket_client_is_connected(client))
    {
        ESP_LOGE(TAG, "WebSocket disconnected");
        return;
    }

    cJSON *auth_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(auth_msg, "type", "auth");
    cJSON_AddStringToObject(auth_msg, "access_token", g_ha_ws_client_config.ha_token);

    char *out = cJSON_PrintUnformatted(auth_msg);
    if (out == NULL)
    {
        cJSON_Delete(auth_msg);
        return;
    }

    ESP_LOGI(TAG, "HA AUTH JSON: %s", out);
    int send_len = strlen(out);
    esp_err_t err = esp_websocket_client_send_text(client, out, send_len, portMAX_DELAY);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error code: %d", err);
    }

    free(out);
    cJSON_Delete(auth_msg);
}

void ha_ws_weather_data_handle(cJSON *item){
    if (item == NULL) return;
    cJSON *temp_data = cJSON_GetObjectItemByPath(item, "a.temperature");
    cJSON *humi_data = cJSON_GetObjectItemByPath(item, "a.humidity");
   if (cJSON_IsNumber(temp_data)) {
        snprintf(entity_weather_data.temp, sizeof(entity_weather_data.temp), "%.1f", temp_data->valuedouble);
    }else {
        ESP_LOGW(TAG, "invalid temp data");
    }

    // 湿度字段安全处理
   if (cJSON_IsNumber(humi_data)) {
        snprintf(entity_weather_data.hum, sizeof(entity_weather_data.hum), "%d", humi_data->valueint);
    } else {
        ESP_LOGW(TAG, "invalid hum data");
    }
    ESP_LOGI(TAG, "Temp=%s, hum=%s", entity_weather_data.temp, entity_weather_data.hum);
}

void ha_ws_message_update_handle(cJSON *root) {
    if (!cJSON_GetObjectItem(root, "event")) return;
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
    } 
    else if(cJSON_GetObjectItemByPath(root, "event.c")){
        cJSON *target_node = cJSON_GetObjectItemByPath(root, "event.c");
        if (target_node) {
            cJSON *item = target_node->child;
            while (item) {
                cJSON *state_node = cJSON_GetObjectItemByPath(item, "+.s");
                cJSON *all_node = cJSON_GetObjectItemByPath(item, "+.a");
                if (state_node) {
                    e_id = item->string;
                    new_state = state_node->valuestring;
                    for (int i = 0; i < 5; i++) {
                        if (strcmp(g_main_ui_device_data[i].entity_id, e_id) == 0) {
                            strncpy(g_main_ui_device_data[i].state, new_state, sizeof(g_main_ui_device_data[i].state) - 1);
                            ESP_LOGI(TAG, "entity_id: %s, new_state: %s", e_id, new_state);
                        }
                    }
                }else if (all_node) {
                    e_id = item->string;
                    new_state = state_node->valuestring;
                    if (strcmp(g_main_ui_device_data[5].entity_id, e_id) == 0) {
                        strncpy(g_main_ui_device_data[5].state, new_state, sizeof(g_main_ui_device_data[5].state) - 1);
                        ESP_LOGI(TAG, "entity_id: %s, new_state: %s", e_id, new_state);
                        ha_ws_weather_data_handle(item);
                    }
                }else{
                    ESP_LOGI(TAG, "not found entity type");
                }         
                item = item->next;
            }
            return;
        }
    }
    else if(cJSON_GetObjectItemByPath(root, "event.a")){
        cJSON *target_node = cJSON_GetObjectItemByPath(root, "event.a");
        if (target_node) {
            cJSON *item = target_node->child;
            while (item) {
                cJSON *state_node = cJSON_GetObjectItem(item, "s");
                if (state_node && cJSON_IsString(state_node) ) {
                    e_id = item->string;
                    new_state = state_node->valuestring;
                    for (int i = 0; i < MAX_COUT; i++) {
                        if (strcmp(g_main_ui_device_data[i].entity_id, e_id) == 0) {
                            strncpy(g_main_ui_device_data[i].state, new_state, sizeof(g_main_ui_device_data[i].state) - 1);
                            ESP_LOGI(TAG, "entity_id: %s, new_state: %s", e_id, new_state);
                            if (i == 5) { 
                                ha_ws_weather_data_handle(item);
                            }
                        }
                    }
                }
                item = item->next;
            }
            return;
        }
    }else{
        ESP_LOGI(TAG, "Error parsing JSON");
        return;
    }

    if (e_id != NULL){
        for (int i = 0; i < MAX_COUT; i++) {
            if (strcmp(g_main_ui_device_data[i].entity_id,e_id)==0){
                strncpy(g_main_ui_device_data[i].state, new_state, sizeof(g_main_ui_device_data[i].state) - 1);
                g_main_ui_device_data[i].state[sizeof(g_main_ui_device_data[i].state) - 1] = '\0';
                ESP_LOGI(TAG, "Update state: %s -> %s",g_main_ui_device_data[i].entity_id,g_main_ui_device_data[i].state);
            }
        }
    }else{
        ESP_LOGD(TAG,"entity_id empty");
    }
}

static void ha_ws_message_handle(cJSON *root){
    cJSON *type_item = cJSON_GetObjectItem(root, "type");
    const char *type = cJSON_IsString(type_item) ? type_item->valuestring : "";
    if (strcmp(type, "auth_required") == 0) {
        ESP_LOGI(TAG, "Recieved auth_required");
        esp_event_post(HA_ACTION_EVENTS, HA_WS_SYSTEM_CONNECTED, NULL, 0, portMAX_DELAY); 
    } 
    else if (strcmp(type, "auth_ok") == 0) {
        ESP_LOGI(TAG, "Autorised, subscribe entities states");     
        ha_ws_convert_entities_subsc(g_main_ui_device_data);
        ha_ws_subscribe_all_events(msg_id++);
    }
    else if (strcmp(type, "auth_invalid") == 0) {
        ESP_LOGE(TAG, "Autorisation failed, will be closed");
    }
    else if (strcmp(type, "result") == 0) {
        cJSON *id_item = cJSON_GetObjectItem(root, "id");
        // char *result_dump = cJSON_PrintUnformatted(root);
        // ESP_LOGW(TAG, "result: %s", result_dump); // 改成D日志减少刷屏
        // free(result_dump);

        if (cJSON_IsNumber(id_item) && id_item->valueint == msg_id - 1) {
            ESP_LOGD(TAG, "Recieved result, ID: %d", msg_id - 1);
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
    case HA_WS_LVGL_BUTTON_RT_TOGGLE:{
        ESP_LOGI(TAG, "RT BUTTON PRESSED");
        const char *entity_id = (const char *)g_main_ui_device_data[0].entity_id;
        if (entity_id == NULL || *entity_id == '\0') {
            ESP_LOGE(TAG, "ID illegale");
            break;
        }
        ESP_LOGI(TAG, "Toggle entity: %s", entity_id);
        ha_ws_entity_control(entity_id, "toggle");
        break;
    }
    case HA_WS_LVGL_BUTTON_RM_TOGGLE: {
        ESP_LOGI(TAG, "RT BUTTON PRESSED");
        const char *entity_id = (const char *)g_main_ui_device_data[1].entity_id;
        if (entity_id == NULL || *entity_id == '\0') {
            ESP_LOGE(TAG, "ID illegale");
            break;
        }
        ESP_LOGI(TAG, "Toggle entity: %s", entity_id);
        ha_ws_entity_control(entity_id, "toggle");
        break;
    }
    case HA_WS_LVGL_BUTTON_RD_TOGGLE:{
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
    case HA_WS_LVGL_BUTTON_MD_TOGGLE:{
        ESP_LOGI(TAG, "RT BUTTON PRESSED");
        const char *entity_id = (const char *)g_main_ui_device_data[3].entity_id;
        if (entity_id == NULL || *entity_id == '\0') {
            ESP_LOGE(TAG, "ID illegale");
            break;
        }
        ha_ws_entity_control(entity_id, "toggle");
        break;
    }   
    case HA_WS_DATA_RECEIVED:
        ws_event_data_t *ha_data = (ws_event_data_t *)event_data; 
        cJSON *head = cJSON_ParseWithLength(ha_data->data, ha_data->len);
        if (head == NULL) {
            break;
        }else {
            ha_ws_message_handle(head);
        }   
    break;
    case HA_WS_SYSTEM_CONNECTED: 
        ha_auth_token_send();
    break;
    case HA_WS_STATE_KICKED_OUT:{
        trigger_reconnect_timer(client); }
    break;   
    case HA_WS_TOKEN_IP_UPDATE:{
        websocket_app_start();
    break;}
    case HA_WS_LVGL_REQ_ALL_DATA:{
    break;}
    case HA_WS_TEST:{
        ws_start_delay_test_packet();
    }
    }
}

// callback function for websocket events
static void ha_ws_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:{
            ESP_LOGI(TAG, "Wait HA Auth Request");
            break;}
        case WEBSOCKET_EVENT_DATA:
            if (ws_rx_buffer == NULL) {
                ws_rx_buffer = heap_caps_malloc(MAX_WS_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                if (ws_rx_buffer == NULL) {return;}
            }
            if (data->payload_offset + data->data_len <= MAX_WS_BUFFER_SIZE) {
                memcpy(ws_rx_buffer + data->payload_offset, data->data_ptr, data->data_len);
            }
            //  Check  if the message is complete
            if (data->op_code == 0x01 && data->payload_offset + data->data_len == data->payload_len && data->payload_len > 0) {
                if(strstr(ws_rx_buffer, "\"event_type\":\"delay_test\"") && !test_finish_flag)
                {
                    int64_t recv_ts = esp_timer_get_time();
                    float rtt_ms = (recv_ts - send_ts_us) / 1000.0f;
                    ESP_LOGI(TAG, "匹配到测试包，发包ts:%lld，收包ts:%lld，单程延迟：%.2fms", send_ts_us, recv_ts, rtt_ms/2);
                    float single_delay = rtt_ms / 2.0f;

                    int rssi_val = 0;
                    esp_wifi_sta_get_rssi(&rssi_val);

                    single_delay_buf[sample_index] = single_delay;
                    rssi_buf[sample_index] = rssi_val;
                    sample_index++;
                    if(sample_index >= SAMPLE_NUM)
                    {
                        test_finish_flag = true;
                        ESP_LOGI(TAG, "已采集满100组时延样本，等待后台上传Python");
                    }
                }


                ws_event_data_t event_payload = {
                    .data = ws_rx_buffer,
                    .len = data->payload_len
                };
                esp_event_post(HA_ACTION_EVENTS, HA_WS_DATA_RECEIVED, &event_payload, sizeof(ws_event_data_t), portMAX_DELAY);
            }        
        break;   
        case WEBSOCKET_EVENT_CLOSED:
            if (ws_rx_buffer) { free(ws_rx_buffer); ws_rx_buffer = NULL; }
            ESP_LOGW(TAG, "WebSocket Closed, wait to restart");
            esp_event_post(HA_ACTION_EVENTS, HA_WS_STATE_KICKED_OUT, NULL, 0, portMAX_DELAY);
            break; 
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "WebSocket Disconnected");
            if (ws_rx_buffer) { free(ws_rx_buffer); ws_rx_buffer = NULL; }
            break;
        case WEBSOCKET_EVENT_ERROR:
            if (ws_rx_buffer) { free(ws_rx_buffer); ws_rx_buffer = NULL; }
            ESP_LOGE(TAG, "WebSocket Error");
            break;    
        case WEBSOCKET_EVENT_FINISH:
            break;
        case WEBSOCKET_EVENT_BEFORE_CONNECT:
            break;
        case WEBSOCKET_EVENT_BEGIN:
            break;
        default:
            ESP_LOGE(TAG, "other WebSocket event, ID: %d", event_id);
            break;
    }
}



// init
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
        esp_websocket_client_stop(client);
        esp_websocket_unregister_events(client, WEBSOCKET_EVENT_ANY, ha_ws_event_handler);
        esp_websocket_client_destroy(client);
        client = NULL;
    }
    char ws_uri_buffer[64] = {0};
    //WebSocket URL
    snprintf(ws_uri_buffer, sizeof(ws_uri_buffer), "ws://%s:8123/api/websocket", g_ha_ws_client_config.ha_ip);
    ESP_LOGI(TAG, "Start WebSocket Client, connect to: %s", ws_uri_buffer);
    const esp_websocket_client_config_t ws_cfg = {
        .uri = ws_uri_buffer,
        .buffer_size = 4096, 
        .reconnect_timeout_ms = 5000,
        .network_timeout_ms = 20000,
        .pingpong_timeout_sec = 60,
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

void ha_rest_get_states_to_psram() {
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
        if (http_HA_client != NULL) {
            esp_http_client_close(http_HA_client);
            esp_http_client_cleanup(http_HA_client);
            http_HA_client = NULL;
        }
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }

    int content_length = esp_http_client_fetch_headers(http_HA_client);
    if (content_length <= 0 ||content_length > 500 * 1024) {
        ESP_LOGE(TAG, "内容长度超出合理范围 %d",content_length);
        if (http_HA_client != NULL) {
            esp_http_client_close(http_HA_client);
            esp_http_client_cleanup(http_HA_client);
            http_HA_client = NULL;
        }
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }
    
    // 直接在 PSRAM 中申请长度对应的内存
    char *buffer = heap_caps_malloc(content_length + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "PSRAM 申请失败！");
        if (http_HA_client != NULL) {
            esp_http_client_close(http_HA_client);
            esp_http_client_cleanup(http_HA_client);
            http_HA_client = NULL;
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
    if (http_HA_client != NULL) {
        esp_http_client_close(http_HA_client);
        esp_http_client_cleanup(http_HA_client);
        http_HA_client = NULL;
    }
}


static void websocket_reconnect_timer_cb(void* arg) {
    esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)arg;
    ESP_LOGI("RECONNECT", "正在从外部任务执行强制重启...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_websocket_client_start(client);
}

static void trigger_reconnect_timer(esp_websocket_client_handle_t ws_client) {
    static esp_timer_handle_t reconnect_timer = NULL;

    if (reconnect_timer == NULL) {
        const esp_timer_create_args_t timer_args = {
            .callback = &websocket_reconnect_timer_cb,
            .arg = (void*)ws_client,
            .name = "ws_reconnect_timer"
        };
        esp_timer_create(&timer_args, &reconnect_timer);
    }

    esp_timer_stop(reconnect_timer);
    
    // 1000000 微秒 = 1 秒后，在后台自动执行回调
    esp_timer_start_once(reconnect_timer, 1000000); 
    ESP_LOGI("RECONNECT", "定时器已启动，1秒后将重启 WebSocket");
}


// test funcion
static esp_err_t send_delay_data_to_python(void)
{
    cJSON_Hooks hooks = {
        .malloc_fn = cjson_psram_malloc,
        .free_fn = cjson_psram_free
    };
    cJSON_InitHooks(&hooks);

    cJSON *root = cJSON_CreateObject();
    cJSON *delay_array = cJSON_CreateArray();
    cJSON *rssi_array = cJSON_CreateArray();

    int rssi_sum = 0;
    for(int i = 0; i < SAMPLE_NUM; i++)
    {
        cJSON_AddItemToArray(delay_array, cJSON_CreateNumber(single_delay_buf[i]));
        cJSON_AddItemToArray(rssi_array, cJSON_CreateNumber(rssi_buf[i]));
        rssi_sum += rssi_buf[i];
    }
    float avg_rssi = (float)rssi_sum / SAMPLE_NUM;

    cJSON_AddItemToObject(root, "single_delay_ms", delay_array);
    cJSON_AddItemToObject(root, "rssi_dbm", rssi_array);
    // 上传本组平均WiFi信号强度替代手动scene标记
    cJSON_AddNumberToObject(root, "avg_rssi_dbm", avg_rssi);

    char *json_text = cJSON_PrintUnformatted(root);
    if(json_text == NULL)
    {
        cJSON_Delete(root);
        return ESP_ERR_NO_MEM;
    }

    //HTTP POST 上传
    esp_http_client_config_t http_cfg = {
        .url = PYTHON_SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000, 
    };
    esp_http_client_handle_t http_client = esp_http_client_init(&http_cfg);
    esp_http_client_set_header(http_client, "Content-Type", "application/json");
    esp_http_client_set_post_field(http_client, json_text, strlen(json_text));
    esp_err_t ret = esp_http_client_perform(http_client);

    if(ret == ESP_OK)
    {
        ESP_LOGI(TAG, "时延数据上传Python成功,本组平均RSSI: %.2f dBm", avg_rssi);
    }
    else
    {
        ESP_LOGE(TAG, "时延数据上传失败, err:%s", esp_err_to_name(ret));
    }

    //释放资源
    esp_http_client_cleanup(http_client);
    free(json_text);
    cJSON_Delete(root);

    //重置缓存，下一轮采集
    sample_index = 0;
    test_finish_flag = false;
    return ret;
}


static void ws_ha_test_envia(void *arg){
    
    while(1)
    {
        if(test_finish_flag)
        {
            send_delay_data_to_python();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ws_ha_test_init(void)
{
    xTaskCreate(ws_ha_test_envia, "ws_ha_test_envia", 16384, NULL, 5, NULL);
}

static void ws_start_delay_test_packet(void)
{
    if(test_finish_flag)
    {
        ESP_LOGW(TAG, "上一轮数据未上传完成，禁止发包");
        return;
    }
    if(!esp_websocket_client_is_connected(client))
    {
        ESP_LOGE(TAG, "HA WS未连接,无法发送测试包");
        return;
    }

    send_ts_us = esp_timer_get_time();
    char payload[192];
    snprintf(payload, sizeof(payload),
        "{\"id\":%d,\"type\":\"fire_event\",\"event_type\":\"delay_test\",\"event_data\":{\"ts\":%lld}}",
        msg_id++, send_ts_us);
    esp_websocket_client_send_text(client, payload, strlen(payload), portMAX_DELAY);
}