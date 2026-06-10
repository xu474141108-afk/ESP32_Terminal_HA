#include "esp_http_client.h"
#include "cJSON.h"
#include "string.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_rom_uart.h"
#include "common_types.h" 
#include "ha_http_control.h"


#define TAG "HA_HTTP"
#define HA_FIND_STATES_URL "http://192.168.1.143:8123/api/states"
#define HA_URL "http://192.168.1.137:8123/api/services/light/turn_on"
#define HA_TEST_STATES_URL "http://homeassistant.local:8123/api/states"
#define HA_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJhYTcwZmE1OWQ2NjI0ZDFiYWVkNjFiM2MxZTU3MDdlZSIsImlhdCI6MTc3NzI5Mjk5NywiZXhwIjoyMDkyNjUyOTk3fQ.Z5T5IGwJm6M56h8j40y3HeuLgPwIlMwR1bQ0DxRIinI"
// 全局缓存标记，记录上次修改时间/ETag
static char g_last_modified[128] = {0};
static char g_etag_buf[128] = {0};
ha_device_t g_HAdevice_ctx;

static void http_cleanup(esp_http_client_handle_t client);

static void* cjson_psram_malloc(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

static void cjson_psram_free(void *ptr) { 
    heap_caps_free(ptr);
}

static void init_cjson_hooks() {
    cJSON_Hooks hooks = {
        .malloc_fn = cjson_psram_malloc,
        .free_fn = cjson_psram_free
    };
    cJSON_InitHooks(&hooks);
}

void get_ha_states_to_psram() {
    g_HAdevice_ctx.state_ha = HA_STATE_SEARCHING;
    init_cjson_hooks();
    esp_http_client_config_t config = {
        .url = HA_FIND_STATES_URL,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 8000, 
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t http_HA_client = esp_http_client_init(&config);

    char auth_header[512]; 
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", HA_TOKEN);
    esp_http_client_set_header(http_HA_client, "Authorization", auth_header);
    esp_http_client_set_header(http_HA_client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_open(http_HA_client,0);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "连接失败: %s", esp_err_to_name(err));
        http_cleanup(http_HA_client);
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }

    int content_length = esp_http_client_fetch_headers(http_HA_client);
    if (content_length <= 0 ||content_length > 500 * 1024) {
        ESP_LOGE(TAG, "内容长度超出合理范围 %d",content_length);
        http_cleanup(http_HA_client);
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }
    
    // 直接在 PSRAM 中申请长度对应的内存
    char *buffer = heap_caps_malloc(content_length + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "PSRAM 申请失败！");
        http_cleanup(http_HA_client);
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
            if (strstr(eid, "light.") || strstr(eid, "switch.")) {
                if (g_HAdevice_ctx.device_count >= MAX_DEVICES) break;

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
    http_cleanup(http_HA_client);
}

static void http_cleanup(esp_http_client_handle_t client)
{
    if (client != NULL) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        client = NULL;
    }
}




void ha_http_control_get_states_to_psram() {
    g_HAdevice_ctx.state_ha = HA_STATE_SEARCHING;
    init_cjson_hooks();
    esp_http_client_config_t config = {
        .url = HA_FIND_STATES_URL,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 8000, 
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t http_HA_client = esp_http_client_init(&config);

    // 1、鉴权头
    char auth_header[512]; 
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", HA_TOKEN);
    esp_http_client_set_header(http_HA_client, "Authorization", auth_header);

    // 2、带上缓存标识，让HA判断有没有状态更新
    if (strlen(g_last_modified) > 0) {
        esp_http_client_set_header(http_HA_client, "If-Modified-Since", g_last_modified);
    }
    if (strlen(g_etag_buf) > 0) {
        esp_http_client_set_header(http_HA_client, "If-None-Match", g_etag_buf);
    }

    esp_err_t err = esp_http_client_open(http_HA_client,0);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "连接失败: %s", esp_err_to_name(err));
        http_cleanup(http_HA_client);
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }

    // 读取响应状态码
    int status_code = esp_http_client_get_status_code(http_HA_client);

    // ==========关键分支：304无更新，直接跳过全量下载解析==========
    if (status_code == 304) {
        ESP_LOGI(TAG, "304 无设备状态变更，无需解析");
        http_cleanup(http_HA_client);
        return;
    }

    // 只有200有新数据，才走原来全量PSRAM+解析逻辑
    int content_length = esp_http_client_fetch_headers(http_HA_client);
    if (content_length <= 0 ||content_length > 500 * 1024) {
        ESP_LOGE(TAG, "内容长度超出合理范围 %d",content_length);
        http_cleanup(http_HA_client);
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }

    // 存储本次新的缓存标签，给下一轮轮询用
    if (strlen(g_last_modified) > 0) {
        esp_http_client_set_header(http_HA_client, "If-Modified-Since", g_last_modified);
    }
    if (strlen(g_etag_buf) > 0) {
        esp_http_client_set_header(http_HA_client, "If-None-Match", g_etag_buf);
    }
    
    // 下面是你原本完整的PSRAM分配、读取、cJSON解析、设备筛选逻辑完全不动
    char *buffer = heap_caps_malloc(content_length + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "PSRAM 申请失败！");
        http_cleanup(http_HA_client);
        g_HAdevice_ctx.state_ha = HA_STATE_HTTP_ERROR;
        return;
    }

    int read_len = esp_http_client_read(http_HA_client, buffer, content_length);
    buffer[read_len] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    if (root && cJSON_IsArray(root)) {
        int array_size = cJSON_GetArraySize(root);
        // ESP_LOGI(TAG, "JSON 解析成功，实体总数: %d", array_size);
        g_HAdevice_ctx.device_count = 0;

        for (int i = 0; i < array_size; i++) {
            cJSON *item = cJSON_GetArrayItem(root, i);
            cJSON *eid_obj = cJSON_GetObjectItem(item, "entity_id");
            if (!cJSON_IsString(eid_obj)) continue;
            const char *eid = eid_obj->valuestring;

            if (strstr(eid, "light.") || strstr(eid, "switch.")) {
                if (g_HAdevice_ctx.device_count >= MAX_DEVICES) break;
                cJSON *attrs = cJSON_GetObjectItem(item, "attributes");
                const char *fname = eid;
                if (cJSON_IsObject(attrs)) {
                    cJSON *fname_obj = cJSON_GetObjectItem(attrs, "friendly_name");
                    if (cJSON_IsString(fname_obj)) fname = fname_obj->valuestring;
                }
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

    if(root) cJSON_Delete(root); 
    heap_caps_free(buffer); 
    http_cleanup(http_HA_client);
}