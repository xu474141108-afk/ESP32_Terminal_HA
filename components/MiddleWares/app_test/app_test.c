#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "ha_http_req.h"
#include <string.h>
#include "esp_log.h"
#include "mdns.h"
#include "esp_netif.h"

#define TAG "HA_MDNS"

static SemaphoreHandle_t ha_poll_mutex = NULL;


esp_err_t mdns_find_homeassistant(char *out_ip, size_t max_len) {
    // 1. 初始化 mDNS 服务
    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS 初始化失败: %d", err);
        return err;
    }

    ESP_LOGI(TAG, "正在局域网中搜寻 Home Assistant 服务 (_homeassistant._tcp)...");

    mdns_result_t *results = NULL;
    // 2. 发起查询：搜索 _homeassistant 服务
    err = mdns_query_ptr("_homeassistant", "_tcp", 3000, 5, &results);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS 查询执行失败");
        return err;
    }

    if (results == NULL) {
        ESP_LOGW(TAG, "未发现任何 Home Assistant 实例。");
        return ESP_FAIL;
    }

    // 3. 解析返回的结果
    bool found = false;
    mdns_result_t *r = results;
    while (r) {
        // 判断是否存在 IP 地址链表
        if (r->addr) {
            mdns_ip_addr_t *ip_node = r->addr;
            
            esp_ip4_addr_t ip4 = ip_node->addr.u_addr.ip4;
                
            snprintf(out_ip, max_len, IPSTR, IP2STR(&ip4));
            ESP_LOGI(TAG, "成功发现 HA 实例！主机名: %s.local, IP: %s, 端口: %d", 
                     r->hostname, out_ip, r->port);
            
            found = true;
            break; // 成功拿到第一个，退出循环
        }
        r = r->next; // 切换到下一个解析出的设备
    }

    mdns_query_results_free(results);

    return found ? ESP_OK : ESP_FAIL;
}

// int get_average_cpu_usage(void) {
//     static uint32_t last_idle0_time = 0;
//     static uint32_t last_idle1_time = 0;
//     static int64_t last_time_stamp = 0;

//     int64_t now_time_stamp = esp_timer_get_time();
    
//     TaskHandle_t idle0_handle = xTaskGetIdleTaskHandleForCPU(0);
//     TaskHandle_t idle1_handle = xTaskGetIdleTaskHandleForCPU(1);
    
//     uint32_t current_idle0_time = 0;
//     uint32_t current_idle1_time = 0;
    
//     if (idle0_handle) current_idle0_time = ulTaskGetIdleRunTimeCounterForCPU(0);
//     if (idle1_handle) current_idle1_time = ulTaskGetIdleRunTimeCounterForCPU(1);

//     int64_t total_time_diff_us = now_time_stamp - last_time_stamp;
//     uint32_t idle0_diff = current_idle0_time - last_idle0_time;
//     uint32_t idle1_diff = current_idle1_time - last_idle1_time;

//     last_idle0_time = current_idle0_time;
//     last_idle1_time = current_idle1_time;
//     last_time_stamp = now_time_stamp;

//     if (total_time_diff_us <= 0) return 0;

//     // ESP Timer 默认 1us 递增一次，所以总可用 tick 数等于时间差值(us)
//     double total_possible_ticks = (double)total_time_diff_us; 
    
//     double idle0_percent = ((double)idle0_diff / total_possible_ticks) * 100.0;
//     double idle1_percent = ((double)idle1_diff / total_possible_ticks) * 100.0;
    
//     // 计算双核平均空闲率，并转换为 CPU 占用率
//     double avg_idle_percent = (idle0_percent + idle1_percent) / 2.0;
//     int cpu_usage = (int)(100.0 - avg_idle_percent);

//     if (cpu_usage < 0) cpu_usage = 0;
//     if (cpu_usage > 100) cpu_usage = 100;

//     return cpu_usage;
// }




static void ha_poll_task(void *arg)
{
    // 互斥锁：防止上一次HTTP没跑完又触发下一次
    ha_poll_mutex = xSemaphoreCreateMutex();
    if (!ha_poll_mutex) {
        ESP_LOGE("ha_poll_task", "Mutex create fail");
        vTaskDelete(NULL);
    }

    while (1)
    {
        // 拿到锁才允许执行采集
        if (xSemaphoreTake(ha_poll_mutex, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI("ha_poll_task", "Start HA poll cycle");
            // 调用你现成的HTTP+PSRAM采集函数
            get_ha_states_to_psram();

            xSemaphoreGive(ha_poll_mutex);
        }

        // 固定100ms周期延时
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void test_cpu_usage_task(void *arg) {
xTaskCreate(ha_poll_task, "ha_poll_task", 4096, NULL, 5, NULL);
}
