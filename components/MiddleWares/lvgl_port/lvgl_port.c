#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lvgl.h"
#include "lvgl_port.h"
#include "common_types.h"
#include "custom.h"

#define EXAMPLE_LVGL_TICK_PERIOD_MS 1
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 10
#define LCD_BUF_HEIGHT_DIVIDER 2
#define TAG "lvgl_port"



lv_ui guider_ui;
// lock of LVGL for thread-safe
static _lock_t lvgl_api_lock;
lv_display_t *display = NULL;

static void example_lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t time_till_next_ms = 0;

    _lock_acquire(&lvgl_api_lock);      // 关门上锁
    lv_display_set_default(display);    
    setup_ui(&guider_ui); 
    _lock_release(&lvgl_api_lock);      // 开门放行
    lv_timer_create(task_OTA_state_monitor, 100, NULL);
    // lv_timer_create(task_HA_state_monitor, 100, NULL);
    ESP_LOGI(TAG, "Starting ui task");
    while (1) {
        _lock_acquire(&lvgl_api_lock);
        time_till_next_ms = lv_timer_handler();
        _lock_release(&lvgl_api_lock);
        time_till_next_ms = MAX(time_till_next_ms, EXAMPLE_LVGL_TASK_MIN_DELAY_MS);
        time_till_next_ms = MIN(time_till_next_ms, EXAMPLE_LVGL_TASK_MAX_DELAY_MS);
        vTaskDelay(pdMS_TO_TICKS(time_till_next_ms));
    }
}

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

static void example_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // because SPI LCD is big-endian, we need to swap the RGB bytes order
    lv_draw_sw_rgb565_swap(px_map, (offsetx2 + 1 - offsetx1) * (offsety2 + 1 - offsety1));
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);
}

static void example_lvgl_touch_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    esp_lcd_touch_handle_t touch_pad = lv_indev_get_user_data(indev);
    esp_lcd_touch_read_data(touch_pad);
    /* Get coordinates */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(touch_pad, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);
    
    //ESP_LOGI("TOUCH_DEBUG", "Pressed: %d, Count: %u", (int)touchpad_pressed, touchpad_cnt);
    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
        //ESP_LOGW("TOUCH_DATA", "X: %u, Y: %u", data->point.x, data->point.y);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void lvgl_port_init(lvgl_panel_t *panel)
{
    _lock_init(&lvgl_api_lock);
    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    ESP_LOGI(TAG, "Initialize LVGL draw buffers");
    display = lv_display_create(panel->h_res, panel->v_res);
    size_t draw_buffer_sz = panel->h_res * (panel->v_res / 2) * sizeof(lv_color16_t);
    void *buf1 = heap_caps_aligned_alloc(64, draw_buffer_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA); // 或者使用片内的方法：spi_bus_dma_memory_alloc
    assert(buf1);
    void *buf2 = heap_caps_aligned_alloc(64, draw_buffer_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
    assert(buf2);

    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);

    ESP_LOGI(TAG, "Install LVGL parameter");
    lv_display_set_user_data(display, panel->panel_handle);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, example_lvgl_flush_cb);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "Register io panel event callback for LVGL flush ready notification");
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = example_notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(panel->io_handle, &cbs, display));

    ESP_LOGI(TAG, "Create LVGL touch input device");
    static lv_indev_t *indev;
    indev = lv_indev_create(); // Input device driver (Touch)
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(indev, display);
    lv_indev_set_user_data(indev, panel->tp_handle);
    lv_indev_set_read_cb(indev, example_lvgl_touch_cb);

    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreatePinnedToCore(example_lvgl_port_task, "LVGL", 16384, NULL, TASK_NIEVL_LVGL_FLUSH, NULL, 0);

    ESP_LOGI(TAG, "Display LVGL Meter Widget");
}

