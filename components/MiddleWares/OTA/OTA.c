
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
#include "common_types.h"
#include "OTA.h"


#define CONFIG_EXAMPLE_FIRMWARE_UPG_URL "http://192.168.1.137:8123/api/services/light/turn_on"
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
    .data_read = 0
};


static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static void __attribute__((noreturn)) task_fatal_error(void)
{
    
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

void OTA_version_check_task(void *pvParameters)
{
    g_ota_ctx.state = OTA_STATE_CHECKING;
    esp_err_t err;    
    esp_http_client_config_t config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPG_URL,
        // .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = 5000,
        .keep_alive_enable = true,
    };

    #ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
        config.skip_cert_common_name_check = true;
    #endif

    if (g_ota_ctx.http_client != NULL) {
        esp_http_client_cleanup(g_ota_ctx.http_client);
    }
    g_ota_ctx.http_client = esp_http_client_init(&config);

    err = esp_http_client_open(g_ota_ctx.http_client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(g_ota_ctx.http_client);
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
        } else {
            ESP_LOGE(TAG, "received package is not fit len");
            http_cleanup(g_ota_ctx.http_client);
            esp_ota_abort(g_ota_ctx.update_handle);
            g_ota_ctx.state = OTA_STATE_HTTP_ERROR;
            vTaskDelete(NULL); // 任务结束自毁
            return;
        }
    }
    g_ota_ctx.state = OTA_STATE_READY;
    vTaskDelete(NULL); // 任务结束自毁
}


void write_ota_data(ota_context_t *ctx)
{
    // int binary_file_length = 0;
    // esp_err_t err;
    
    // err = esp_ota_begin(ota_ctx.update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_ctx.update_handle);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
    //     http_cleanup(ota_ctx.http_client);
    //     esp_ota_abort(ota_ctx.update_handle);
    //     task_fatal_error();
    // }
    // ESP_LOGI(TAG, "esp_ota_begin succeeded");
    // if (esp_ota_write( ota_ctx.update_handle, (const void *)ota_write_data, ota_ctx.data_read) != ESP_OK) {
    //         http_cleanup(ota_ctx.http_client);
    //         esp_ota_abort(ota_ctx.update_handle);
    //         task_fatal_error();
    //     }
    //     binary_file_length += ota_ctx.data_read;
        
    // while (1) {
    //     ota_ctx.data_read = esp_http_client_read(ota_ctx.http_client, ota_write_data, BUFFSIZE);
    //     if (ota_ctx.data_read < 0) {
    //         ESP_LOGE(TAG, "Error: SSL data read error");
    //         http_cleanup(ota_ctx.http_client);
    //         task_fatal_error();
    //     } else if (ota_ctx.data_read > 0) {
    //         err = esp_ota_write( ota_ctx.update_handle, (const void *)ota_write_data, ota_ctx.data_read);
    //         if (err != ESP_OK) {
    //             http_cleanup(ota_ctx.http_client);
    //             esp_ota_abort(ota_ctx.update_handle);
    //             task_fatal_error();
    //         }
    //         binary_file_length += ota_ctx.data_read;
    //         ESP_LOGD(TAG, "Written image length %d", binary_file_length);
    //     } else if (ota_ctx.data_read == 0) {
    //        /*
    //         * As esp_http_client_read never returns negative error code, we rely on
    //         * `errno` to check for underlying transport connectivity closure if any
    //         */
    //         if (errno == ECONNRESET || errno == ENOTCONN) {
    //             ESP_LOGE(TAG, "Connection closed, errno = %d", errno);
    //             break;
    //         }
    //         if (esp_http_client_is_complete_data_received(ota_ctx.http_client) == true) {
    //             ESP_LOGI(TAG, "Connection closed");
    //             break;
    //         }
    //     }
    // }
    // // end while
    // ESP_LOGI(TAG, "Total Write binary data length: %d", binary_file_length);

    // if (esp_http_client_is_complete_data_received(ota_ctx.http_client) != true) {
    //     ESP_LOGE(TAG, "Error in receiving complete file");
    //     http_cleanup(ota_ctx.http_client);
    //     esp_ota_abort(ota_ctx.update_handle);
    //     task_fatal_error();
    // }

    // err = esp_ota_end(ota_ctx.update_handle);
    // if (err != ESP_OK) {
    //     if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
    //         ESP_LOGE(TAG, "Image validation failed, image is corrupted");
    //     } else {
    //         ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
    //     }
    //     http_cleanup(ota_ctx.http_client);
    //     task_fatal_error();
    // }

    // err = esp_ota_set_boot_partition(ota_ctx.update_partition);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
    //     http_cleanup(ota_ctx.http_client);
    //     task_fatal_error();
    // }

    // ESP_LOGI(TAG, "Prepare to restart system!");
    // esp_restart();
    
    // return ;
}


