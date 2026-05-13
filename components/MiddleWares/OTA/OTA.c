
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


#define CONFIG_EXAMPLE_FIRMWARE_UPG_URL "http://192.168.1.130:8000/version.json"
static char target_bin_url[256] = {0};

#define CONFIG_EXAMPLE_OTA_RECV_TIMEOUT 5000
#define CONFIG_EXAMPLE_SKIP_VERSION_CHECK 1

#define BUFFSIZE 1024
#define TAG "OTA"

static char ota_write_data[BUFFSIZE + 1] = { 0 };
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

ota_context_t g_ota_ctx = {
    .state = OTA_STATE_IDLE,
    .http_client = NULL,
    .update_handle = 0,
    .update_partition = NULL,
    .data_read = 0,
    .current_ver = "1.0.1",
    .latest_ver = "1.0.1" // 预设一个比当前版本高的版本号，方便测试
};


static void http_cleanup(esp_http_client_handle_t client)
{
    if (client != NULL) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        client = NULL;
    }
}


static void __attribute__((noreturn)) task_fatal_error(void)
{
    
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}



static void OTA_version_check_task(void *pvParameters)
{
    g_ota_ctx.state = OTA_STATE_CHECKING;
    esp_http_client_config_t http_config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPG_URL,
        // .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = 5000,
        .keep_alive_enable = true,
    };

    const esp_app_desc_t *app_desc = esp_app_get_description();
    strncpy(g_ota_ctx.current_ver, app_desc->version, sizeof(g_ota_ctx.current_ver) - 1);
    g_ota_ctx.current_ver[sizeof(g_ota_ctx.current_ver) - 1] = '\0';

    g_ota_ctx.http_client = esp_http_client_init(&http_config);
    esp_err_t err = esp_http_client_set_header(g_ota_ctx.http_client, "User-Agent", "ESP32 OTA Client");
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP init failed: %s", esp_err_to_name(err));
    }
    

    err = esp_http_client_open(g_ota_ctx.http_client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        http_cleanup(g_ota_ctx.http_client);
        g_ota_ctx.state = OTA_STATE_HTTP_ERROR;
        vTaskDelete(NULL); // 任务结束自毁
        return;
    }
    esp_http_client_fetch_headers(g_ota_ctx.http_client);
    int len = esp_http_client_read(g_ota_ctx.http_client, ota_write_data, BUFFSIZE);
    ota_write_data[len] = '\0'; // 必须截断字符串给 cJSON 用

    cJSON *root = cJSON_Parse(ota_write_data);
    if (root)
    {
        cJSON *ver = cJSON_GetObjectItem(root, "version");     
        cJSON *url = cJSON_GetObjectItem(root, "file_url");
        if (cJSON_IsString(ver) && cJSON_IsString(url)) {
            strcpy(g_ota_ctx.latest_ver, ver->valuestring);
            g_ota_ctx.latest_ver[sizeof(g_ota_ctx.latest_ver) - 1] = '\0';
            if (strcmp(g_ota_ctx.latest_ver, g_ota_ctx.current_ver) != 0) {
                strncpy(g_ota_ctx.download_url, url->valuestring, sizeof(g_ota_ctx.download_url));
                g_ota_ctx.state = OTA_STATE_READY; 
                ESP_LOGI(TAG, "发现新版本: %s", g_ota_ctx.latest_ver);
            } else {
                g_ota_ctx.state = OTA_STATE_NO_NEW;
                ESP_LOGI(TAG, "已经是最新版本了");
            }
        }
        cJSON_Delete(root);
    }
    http_cleanup(g_ota_ctx.http_client);
    vTaskDelete(NULL);  
}

void OTA_auto_scan_task(void *pvParameters) {
    while (1) {
        // 只有在空闲时才自动检测，避免干扰正在进行的更新
        if (g_ota_ctx.state == OTA_STATE_IDLE) {
            ESP_LOGI(TAG, "后台自动扫描更新...");
            xTaskCreate(OTA_version_check_task, "ota_check_task", 8192, NULL, 4, NULL);
        }
        // vTaskDelay(pdMS_TO_TICKS(3600 * 1000));
        vTaskDelay(pdMS_TO_TICKS(6 * 1000)); // 每6s检查一次
    }
}

void write_ota_data(void *pvParameters)
{
    int binary_file_length = 0;
    esp_err_t err;
    
    err = esp_ota_begin(g_ota_ctx.update_partition, OTA_WITH_SEQUENTIAL_WRITES, &g_ota_ctx.update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        http_cleanup(g_ota_ctx.http_client);
        esp_ota_abort(g_ota_ctx.update_handle);
        task_fatal_error();
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");
    if (esp_ota_write( g_ota_ctx.update_handle, (const void *)ota_write_data, g_ota_ctx.data_read) != ESP_OK) {
            http_cleanup(g_ota_ctx.http_client);
            esp_ota_abort(g_ota_ctx.update_handle);
            task_fatal_error();
        }
        binary_file_length += g_ota_ctx.data_read;
        
    while (1) {
        g_ota_ctx.data_read = esp_http_client_read(g_ota_ctx.http_client, ota_write_data, BUFFSIZE);
        if (g_ota_ctx.data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            http_cleanup(g_ota_ctx.http_client);
            task_fatal_error();
        } else if (g_ota_ctx.data_read > 0) {
            err = esp_ota_write( g_ota_ctx.update_handle, (const void *)ota_write_data, g_ota_ctx.data_read);
            if (err != ESP_OK) {
                http_cleanup(g_ota_ctx.http_client);
                esp_ota_abort(g_ota_ctx.update_handle);
                task_fatal_error();
            }
            binary_file_length += g_ota_ctx.data_read;
            ESP_LOGD(TAG, "Written image length %d", binary_file_length);
        } else if (g_ota_ctx.data_read == 0) {
           /*
            * As esp_http_client_read never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                ESP_LOGE(TAG, "Connection closed, errno = %d", errno);
                g_ota_ctx.state = OTA_STATE_FAILED;
                break;
            }
            if (esp_http_client_is_complete_data_received(g_ota_ctx.http_client) == true) {
                ESP_LOGI(TAG, "Connection closed");
                g_ota_ctx.state = OTA_STATE_FAILED;
                break;
            }
        }
    }
    // end while
    ESP_LOGI(TAG, "Total Write binary data length: %d", binary_file_length);

    if (esp_http_client_is_complete_data_received(g_ota_ctx.http_client) != true) {
        ESP_LOGE(TAG, "Error in receiving complete file");
        g_ota_ctx.state = OTA_STATE_FAILED;
        http_cleanup(g_ota_ctx.http_client);
        esp_ota_abort(g_ota_ctx.update_handle);
        task_fatal_error();
    }

    err = esp_ota_end(g_ota_ctx.update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        g_ota_ctx.state = OTA_STATE_FAILED;
        http_cleanup(g_ota_ctx.http_client);
        task_fatal_error();
    }

    err = esp_ota_set_boot_partition(g_ota_ctx.update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(g_ota_ctx.http_client);
        task_fatal_error();
    }

    ESP_LOGI(TAG, "Prepare to restart system!");
    g_ota_ctx.state = OTA_STATE_SUCCESS;
    // esp_restart();
    
    return ;
}



void OTA_version_check(void *pvParameters)
{
    g_ota_ctx.state = OTA_STATE_CHECKING;
    esp_err_t err;    
    esp_http_client_config_t config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPG_URL,
        // .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = 5000,
        .keep_alive_enable = true,
    };
    esp_app_desc_t running_app_info;
        esp_ota_get_partition_description(esp_ota_get_boot_partition(), &running_app_info);
        ESP_LOGI(TAG, "本地运行版本: %s", running_app_info.version);


    #ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
        config.skip_cert_common_name_check = true;
    #endif

    g_ota_ctx.http_client = esp_http_client_init(&config);

    err = esp_http_client_open(g_ota_ctx.http_client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        http_cleanup(g_ota_ctx.http_client);
        g_ota_ctx.state = OTA_STATE_HTTP_ERROR;
        vTaskDelete(NULL); // 任务结束自毁
        return;
    }
    esp_http_client_fetch_headers(g_ota_ctx.http_client);

    g_ota_ctx.update_partition = esp_ota_get_next_update_partition(NULL);
    assert(g_ota_ctx.update_partition != NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%"PRIx32,
             g_ota_ctx.update_partition->subtype, g_ota_ctx.update_partition->address);

    g_ota_ctx.data_read = esp_http_client_read(g_ota_ctx.http_client, ota_write_data, BUFFSIZE);
    if (g_ota_ctx.data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            http_cleanup(g_ota_ctx.http_client);
            g_ota_ctx.state = OTA_STATE_HTTP_ERROR;
            vTaskDelete(NULL); // 任务结束自毁
            return;
    } else if (g_ota_ctx.data_read > 0) {
        esp_app_desc_t new_app_info;
        if (g_ota_ctx.data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
            // check current version with downloading
            memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
            ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

            esp_app_desc_t running_app_info;
            if (esp_ota_get_partition_description(esp_ota_get_boot_partition(), &running_app_info) == ESP_OK) {
                ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
            }

            const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
            esp_app_desc_t invalid_app_info;
            if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
            }

            // check current version with last invalid partition
            if (last_invalid_app != NULL) {
                if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                    ESP_LOGW(TAG, "New version is the same as invalid version.");
                    ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                    ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
                    g_ota_ctx.state = OTA_STATE_NO_NEW;
                    http_cleanup(g_ota_ctx.http_client);
                    vTaskDelete(NULL); // 任务结束自毁
                    return;
                }
            }
            if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
                g_ota_ctx.state = OTA_STATE_NO_NEW;
                http_cleanup(g_ota_ctx.http_client);
                vTaskDelete(NULL);
                return;
            }
        } 
        else {
            ESP_LOGE(TAG, "received package is not fit len");
            http_cleanup(g_ota_ctx.http_client);
            esp_ota_abort(g_ota_ctx.update_handle);
            g_ota_ctx.state = OTA_STATE_LEN_NOFIT;
            vTaskDelete(NULL); // 任务结束自毁
            return;
        }
    }
    g_ota_ctx.state = OTA_STATE_READY;
    vTaskDelete(NULL); // 任务结束自毁
}
