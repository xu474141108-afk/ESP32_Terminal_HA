#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

int get_average_cpu_usage(void) {
    static uint32_t last_idle0_time = 0;
    static uint32_t last_idle1_time = 0;
    static int64_t last_time_stamp = 0;

    int64_t now_time_stamp = esp_timer_get_time();
    
    TaskHandle_t idle0_handle = xTaskGetIdleTaskHandleForCPU(0);
    TaskHandle_t idle1_handle = xTaskGetIdleTaskHandleForCPU(1);
    
    uint32_t current_idle0_time = 0;
    uint32_t current_idle1_time = 0;
    
    if (idle0_handle) current_idle0_time = ulTaskGetIdleRunTimeCounterForCPU(0);
    if (idle1_handle) current_idle1_time = ulTaskGetIdleRunTimeCounterForCPU(1);

    int64_t total_time_diff_us = now_time_stamp - last_time_stamp;
    uint32_t idle0_diff = current_idle0_time - last_idle0_time;
    uint32_t idle1_diff = current_idle1_time - last_idle1_time;

    last_idle0_time = current_idle0_time;
    last_idle1_time = current_idle1_time;
    last_time_stamp = now_time_stamp;

    if (total_time_diff_us <= 0) return 0;

    // ESP Timer 默认 1us 递增一次，所以总可用 tick 数等于时间差值(us)
    double total_possible_ticks = (double)total_time_diff_us; 
    
    double idle0_percent = ((double)idle0_diff / total_possible_ticks) * 100.0;
    double idle1_percent = ((double)idle1_diff / total_possible_ticks) * 100.0;
    
    // 计算双核平均空闲率，并转换为 CPU 占用率
    double avg_idle_percent = (idle0_percent + idle1_percent) / 2.0;
    int cpu_usage = (int)(100.0 - avg_idle_percent);

    if (cpu_usage < 0) cpu_usage = 0;
    if (cpu_usage > 100) cpu_usage = 100;

    return cpu_usage;
}