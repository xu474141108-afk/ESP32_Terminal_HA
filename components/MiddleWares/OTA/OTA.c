
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_app_desc.h"
#include "cJSON.h"
#include "common_types.h"
#include "OTA.h"


#define CONFIG_FIRMWARE_UPG_URL "http://192.168.1.130:8000/version.json"

#define CONFIG_EXAMPLE_OTA_RECV_TIMEOUT 5000
#define CONFIG_EXAMPLE_SKIP_VERSION_CHECK 1
#define CONFIG_AUTO_CHECK_OTA_INTERVAL 60*1000

#define BUFFSIZE 1024
#define TAG "OTA"

static char ota_write_data[BUFFSIZE + 1] = { 0 };
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

ota_context_t g_ota_ctx = {
    .state = OTA_STATE_IDLE,
    .update_partition = NULL,
    .data_read = 0,
    .current_ver = "1.1.1",
    .latest_ver = {0} // 预设一个比当前版本高的版本号，方便测试
};

static void OTA_version_check_task(void *pvParameters);
static int compare_version(const char *v1, const char *v2);
static void http_cleanup(esp_http_client_handle_t client);



void OTA_autoscan_task(void *pvParameters) {
    while (1) {
        if (g_ota_ctx.state == OTA_STATE_IDLE) {
            xTaskCreate(OTA_version_check_task, "ota_check_task", 8192, NULL, 4, NULL);
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_AUTO_CHECK_OTA_INTERVAL)); 
    }
}

static void OTA_version_check_task(void *pvParameters)
{
    g_ota_ctx.state = OTA_STATE_CHECKING;
    esp_http_client_config_t http_config = {
        .url = CONFIG_FIRMWARE_UPG_URL,
        // .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = 5000,
        .keep_alive_enable = true,
    };

    const esp_app_desc_t *app_desc = esp_app_get_description();
    strncpy(g_ota_ctx.current_ver, app_desc->version, sizeof(g_ota_ctx.current_ver) - 1);
    g_ota_ctx.current_ver[sizeof(g_ota_ctx.current_ver) - 1] = '\0';

    esp_http_client_handle_t http_OTA_check_client = esp_http_client_init(&http_config);

    esp_err_t err = esp_http_client_open(http_OTA_check_client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        http_cleanup(http_OTA_check_client);
        g_ota_ctx.state = OTA_STATE_HTTP_ERROR;
        vTaskDelete(NULL); // 任务结束自毁
        return;
    }
    esp_http_client_fetch_headers(http_OTA_check_client);
    int len = esp_http_client_read(http_OTA_check_client, ota_write_data, BUFFSIZE);
    ota_write_data[len] = '\0'; // 必须截断字符串给 cJSON 用

    cJSON *root = cJSON_Parse(ota_write_data);
    if (root)
    {
        cJSON *ver = cJSON_GetObjectItem(root, "version");     
        cJSON *url = cJSON_GetObjectItem(root, "file_url");
        if (cJSON_IsString(ver) && cJSON_IsString(url)) {
            strcpy(g_ota_ctx.latest_ver, ver->valuestring);
            g_ota_ctx.latest_ver[sizeof(g_ota_ctx.latest_ver) - 1] = '\0';
            if (compare_version(g_ota_ctx.latest_ver, g_ota_ctx.current_ver) > 0) {
                strncpy(g_ota_ctx.download_url, url->valuestring, sizeof(g_ota_ctx.download_url));
                g_ota_ctx.state = OTA_STATE_READY; 
                ESP_LOGI(TAG, "发现新版本,现版本: %s，最新版本: %s", g_ota_ctx.current_ver, g_ota_ctx.latest_ver);
                
            } else
            {
                g_ota_ctx.state = OTA_STATE_NO_NEW;
                ESP_LOGI(TAG, "已是最新版本,现版本: %s, 最新版本: %s", g_ota_ctx.current_ver, g_ota_ctx.latest_ver);
            }
        }
        cJSON_Delete(root);
    }
    http_cleanup(http_OTA_check_client);
    vTaskDelete(NULL);  
}



void OTA_download_data(void *pvParameters)
{
    if(g_ota_ctx.state != OTA_STATE_DOWNLOADING) return;

    int binary_file_length = 0;
    esp_ota_handle_t update_handle = 0 ;
    g_ota_ctx.update_partition = esp_ota_get_next_update_partition(NULL);


    esp_http_client_config_t http_config = {
        .url = g_ota_ctx.download_url,
        // .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = 10000,
        .keep_alive_enable = true,
    };
    esp_http_client_handle_t http_OTA_Download_client = esp_http_client_init(&http_config);    
    esp_err_t err = esp_http_client_open(http_OTA_Download_client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        http_cleanup(http_OTA_Download_client);
        g_ota_ctx.state = OTA_STATE_HTTP_ERROR;
        vTaskDelete(NULL); 
        return;
    }
    esp_http_client_fetch_headers(http_OTA_Download_client);
    err = esp_ota_begin(g_ota_ctx.update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin 失败: %s", esp_err_to_name(err));
        http_cleanup(http_OTA_Download_client);
        esp_ota_abort(update_handle);
        g_ota_ctx.state = OTA_STATE_FAILED;
        vTaskDelete(NULL); 
    }
    
    int data_read = 0;
    while(1){
        data_read = esp_http_client_read(http_OTA_Download_client, ota_write_data, BUFFSIZE);   
        if(data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            http_cleanup(http_OTA_Download_client);
            esp_ota_abort(update_handle);
            g_ota_ctx.state = OTA_STATE_FAILED;
            vTaskDelete(NULL); 
        } else if (data_read > 0) {
            err = esp_ota_write(update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_write 失败: %s", esp_err_to_name(err));
                http_cleanup(http_OTA_Download_client);
                esp_ota_abort(update_handle);
                g_ota_ctx.state = OTA_STATE_FAILED;
                vTaskDelete(NULL); 
            }
            binary_file_length += data_read;
        } else if (data_read == 0) {
            if (esp_http_client_is_complete_data_received(http_OTA_Download_client)) {
                ESP_LOGI(TAG, "下载完成");
                http_cleanup(http_OTA_Download_client);
                g_ota_ctx.state = OTA_STATE_INSTALLING;
                break;
            } else {
                ESP_LOGE(TAG, "连接意外中断");
                http_cleanup(http_OTA_Download_client);
                esp_ota_abort(update_handle);
                g_ota_ctx.state = OTA_STATE_FAILED;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "固件校验失败 (OTA End): %s", esp_err_to_name(err));
        http_cleanup(http_OTA_Download_client);
        esp_ota_abort(update_handle);
        g_ota_ctx.state = OTA_STATE_FAILED;
    }

    // 7. 设置下次启动的分区
    err = esp_ota_set_boot_partition(g_ota_ctx.update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "设置启动分区失败: %s", esp_err_to_name(err));
        http_cleanup(http_OTA_Download_client);
        g_ota_ctx.state = OTA_STATE_FAILED;
    }

    ESP_LOGI(TAG, "OTA 成功！系统将在 10 秒后重启...");
    g_ota_ctx.state = OTA_STATE_SUCCESS;
    vTaskDelay(pdMS_TO_TICKS(1*1000));
    esp_restart();

}

static void http_cleanup(esp_http_client_handle_t client)
{
    if (client != NULL) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        client = NULL;
    }
}

/**
 * @brief 比较两个版本号
 * @return 1: v1 > v2,    -1: v1 < v2,        0: v1 == v2
 */
static int compare_version(const char *v1, const char *v2) {
    int v1_major, v1_minor, v1_patch;
    int v2_major, v2_minor, v2_patch;

    sscanf(v1, "%d.%d.%d", &v1_major, &v1_minor, &v1_patch);
    sscanf(v2, "%d.%d.%d", &v2_major, &v2_minor, &v2_patch);

    if (v1_major != v2_major) return (v1_major > v2_major) ? 1 : -1;
    if (v1_minor != v2_minor) return (v1_minor > v2_minor) ? 1 : -1;
    if (v1_patch != v2_patch) return (v1_patch > v2_patch) ? 1 : -1;

    return 0; 
}