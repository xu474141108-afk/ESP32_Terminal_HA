#define configGENERATE_RUN_TIME_STATS           1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() /* 留空即可，因为底层esp_timer系统会自动初始化 */
#define portGET_RUN_TIME_COUNTER_VALUE()        ((uint32_t)esp_timer_get_time())

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>

volatile int g_avg_cpu_usage_10s = 0;
static const char *MON_TAG = "CPU_MON";

int get_average_cpu_usage(void) {
    static uint32_t last_idle0_time = 0;
    static uint32_t last_idle1_time = 0;
    static int64_t last_time_stamp = 0;

    int64_t now_time_stamp = esp_timer_get_time();
    
    TaskHandle_t idle0_handle = xTaskGetIdleTaskHandleForCore(0);
    TaskHandle_t idle1_handle = xTaskGetIdleTaskHandleForCore(1);
    
    TaskStatus_t idle0_status, idle1_status;
    uint32_t current_idle0_time = 0;
    uint32_t current_idle1_time = 0;
    
    if (idle0_handle) {
        vTaskGetInfo(idle0_handle, &idle0_status, pdTRUE, eInvalid);
        current_idle0_time = idle0_status.ulRunTimeCounter;
    }
    if (idle1_handle) {
        vTaskGetInfo(idle1_handle, &idle1_status, pdTRUE, eInvalid);
        current_idle1_time = idle1_status.ulRunTimeCounter;
    }

    int64_t total_time_diff_us = now_time_stamp - last_time_stamp;
    uint32_t idle0_diff = current_idle0_time - last_idle0_time;
    uint32_t idle1_diff = current_idle1_time - last_idle1_time;

    last_idle0_time = current_idle0_time;
    last_idle1_time = current_idle1_time;
    last_time_stamp = now_time_stamp;

    if (total_time_diff_us <= 0) return 0;

    double total_possible_ticks = (double)total_time_diff_us; 
    
    double idle0_percent = ((double)idle0_diff / total_possible_ticks) * 100.0;
    double idle1_percent = ((double)idle1_diff / total_possible_ticks) * 100.0;
    
    double avg_idle_percent = (idle0_percent + idle1_percent) / 2.0;
    int cpu_usage = (int)(100.0 - avg_idle_percent);

    if (cpu_usage < 0) cpu_usage = 0;
    if (cpu_usage > 100) cpu_usage = 100;

    return cpu_usage;
}

// 替代 vTaskGetRunTimeStats 的无错版本系统快照打印
void cpu_monitor_task(void *pvParameters) {
    // 初始化基准
    get_average_cpu_usage(); 
    
    // 分配用于存储任务数组的内存（假设系统最多 20 个任务）
    UBaseType_t uxArraySize = 20;
    TaskStatus_t *pxTaskStatusArray = malloc(uxArraySize * sizeof(TaskStatus_t));

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000)); // 每 5 秒监控一次

        // 1. 先更新双核平均 10 秒占用率到全局变量
        g_avg_cpu_usage_10s = get_average_cpu_usage();
        ESP_LOGI(MON_TAG, "===============================================");
        ESP_LOGI(MON_TAG, ">> 当前计算的 5~10s 双核平均 CPU 占用率: %d%% <<", g_avg_cpu_usage_10s);

        if (pxTaskStatusArray != NULL) {
            uint32_t ulTotalRunTime;
            // 2. 获取当前真正的全系统任务快照
            UBaseType_t uxReturnedTaskCount = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);
            
            // 如果预留的 20 个不够大，动态扩容（安全保护）
            if (uxReturnedTaskCount >= uxArraySize) {
                uxArraySize = uxReturnedTaskCount + 5;
                free(pxTaskStatusArray);
                pxTaskStatusArray = malloc(uxArraySize * sizeof(TaskStatus_t));
                continue;
            }

            ESP_LOGI(MON_TAG, "--- 🔍 实时任务运行时长(Ticks)明细 ---");
            for (UBaseType_t x = 0; x < uxReturnedTaskCount; x++) {
                // 打印出任务名和它自开机以来占用的总 CPU Ticks
                printf("Task: %-16s \t Runtime Ticks: %lu\n", 
                       pxTaskStatusArray[x].pcTaskName, 
                       pxTaskStatusArray[x].ulRunTimeCounter);
            }
        }
        ESP_LOGI(MON_TAG, "===============================================");
    }
    
    if (pxTaskStatusArray) free(pxTaskStatusArray);
    vTaskDelete(NULL);
}