#include "esp_http_client.h"
#include "cJSON.h"
#include "string.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_rom_uart.h"
#include "common_types.h" 

#define TAG "HA_HTTP"
#define HA_ALL_STATES_URL "http://192.168.1.137:8123/api/states"


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

// 2. 获取并过滤数据的核心函数
void get_ha_states_to_psram() {
    init_cjson_hooks();
    esp_http_client_config_t config = {
        .url = HA_ALL_STATES_URL,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 8000, 
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    char auth_header[512]; 
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", HA_TOKEN);
    esp_http_client_set_header(client, "Authorization", auth_header);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_open(client,0);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "连接失败: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    // int status_code = esp_http_client_get_status_code(client);
    // ESP_LOGI(TAG, "HTTP 状态码: %d", status_code);

    int content_length = esp_http_client_fetch_headers(client);
    if (content_length <= 0 || content_length > 500 * 1024) {
        ESP_LOGE(TAG, "内容长度超出合理范围 %d",content_length);
        esp_http_client_cleanup(client);
        return;
    }

    // 直接在 PSRAM 中申请长度对应的内存
    char *buffer = heap_caps_malloc(content_length + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "PSRAM 申请失败！");
        esp_http_client_cleanup(client);
        return;
    }

    int read_len = esp_http_client_read(client, buffer, content_length);
    buffer[read_len] = '\0';

    // 解析 JSON
    cJSON *root = cJSON_Parse(buffer);
    if (root && cJSON_IsArray(root)) {
        int array_size = cJSON_GetArraySize(root);
        ESP_LOGI(TAG, "JSON 解析成功，实体总数: %d", array_size);
        
        g_device_count = 0;

        for (int i = 0; i < array_size; i++) {
            cJSON *item = cJSON_GetArrayItem(root, i);
            cJSON *eid_obj = cJSON_GetObjectItem(item, "entity_id");

            if (!cJSON_IsString(eid_obj)) continue;
            const char *eid = eid_obj->valuestring;

            // 筛选条件：只取 light 和 switch 设备
            if (strstr(eid, "light.") || strstr(eid, "switch.")) {
                if (g_device_count >= MAX_DEVICES) break;

                cJSON *attrs = cJSON_GetObjectItem(item, "attributes");
                const char *fname = eid;

                if (cJSON_IsObject(attrs)) {
                    cJSON *fname_obj = cJSON_GetObjectItem(attrs, "friendly_name");
                    if (cJSON_IsString(fname_obj)) fname = fname_obj->valuestring;
                }

                // 存入全局数组
                strncpy(g_device_list[g_device_count].entity_id, eid, 63);
                strncpy(g_device_list[g_device_count].friendly_name, fname, 63);
                g_device_count++;
            }
        }
    }
    else{
        ESP_LOGI(TAG, "JSON 解析失败");
    }

    if(root){
           cJSON_Delete(root); 
        }
    heap_caps_free(buffer); // 释放 PSRAM 内存
    esp_http_client_cleanup(client);

    if(ha_event_group != NULL){
        xEventGroupSetBits(ha_event_group, HA_DATA_READY_BIT);
    }
}

