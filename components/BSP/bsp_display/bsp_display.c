#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/ledc.h" 
#include "sdkconfig.h"

//其他文件
#include "bsp_display.h"
#include "lvgl_port.h"
#include "bsp_xpt2046.h"
#ifdef CONFIG_BSP_LCD_ILI9341
#include "bsp_ili9341.h"
#elif CONFIG_BSP_LCD_ILI9488
#include "bsp_ili9488.h"
#endif

#define LCD_SPI_MODE 0
#define LCD_SPI_CLK_FREQ_HZ 60 * 1000 * 1000 // 60MHz
#define TOUCH_SPI_CLK_FREQ_HZ 2 * 1000 * 1000 // 2MHz
#define LCD_LIGHT_LEVEL 80 
#define LCD_BK_LIGHT_ON_LEVEL  1
#define LCD_COLOR_MODE 16

#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8   

#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define LCD_BUFFER_SIZE (LCD_WIDTH * LCD_HEIGHT * 2) // 320x240，每像素2字节
#define LCD_HOST SPI2_HOST
#define LCD_DMA_CHAN 1 

// 定义 GPIO 引脚
#define LCD_DC_GPIO_NUM 42
#define LCD_CS_GPIO_NUM 1
#define LCD_RST_GPIO_NUM 2
#define LCD_BL_GPIO_NUM 11

#define LCD_CLK_GPIO_NUM 40
#define LCD_MOSI_GPIO_NUM 41
#define LCD_MISO_GPIO_NUM 45

#define TOUCH_IRQ_GPIO_NUM 47
#define TOUCH_CS_GPIO_NUM 12

esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_touch_handle_t tp = NULL;
lvgl_panel_t panel = {0};

static const char *TAG = "bsp_display";

void bsp_display_bk_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LCD_BL_GPIO_NUM,
        .duty           = 0, // 初始亮度为 0
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

}

void bsp_display_brightness(int brightness)
{
    if (brightness < 0) brightness = 0;
    if (brightness > 100) brightness = 100;
    uint32_t duty = brightness * 82; // 将亮度值映射到 0-8191 的范围
    uint32_t default_duty = (LCD_BK_LIGHT_ON_LEVEL == 1) ? duty : (8191 - duty);
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, default_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static void bsp_display_LCD_init(void)
{
    // bsp_display_bk_init();

    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_BL_GPIO_NUM
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
    .sclk_io_num = LCD_CLK_GPIO_NUM,  
    .mosi_io_num = LCD_MOSI_GPIO_NUM,  
    .miso_io_num = LCD_MISO_GPIO_NUM,  
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = LCD_BUFFER_SIZE,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));


    ESP_LOGI(TAG, "Install panel IO");

    esp_lcd_panel_io_spi_config_t io_config = {
    .dc_gpio_num = LCD_DC_GPIO_NUM,        
    .cs_gpio_num = LCD_CS_GPIO_NUM,        
    .pclk_hz = LCD_SPI_CLK_FREQ_HZ,
    .lcd_cmd_bits = LCD_CMD_BITS,        
    .lcd_param_bits = LCD_PARAM_BITS,      
    .spi_mode = LCD_SPI_MODE,            
    .trans_queue_depth = 10, 
    
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_GPIO_NUM,      
        .color_space = ESP_LCD_COLOR_SPACE_RGB, 
        .bits_per_pixel = LCD_COLOR_MODE,     
    };
    #if CONFIG_BSP_LCD_ILI9341
        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
    #elif CONFIG_BSP_LCD_ILI9488
        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9488(io_handle, &panel_config, &panel_handle));
    #endif
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    gpio_set_level(LCD_BL_GPIO_NUM, LCD_BK_LIGHT_ON_LEVEL);
    ESP_LOGI(TAG, "Display initialized successfully");

    // bsp_display_brightness(LCD_LIGHT_LEVEL);   
}


static void bsp_display_touch_init(void)
{
    ESP_LOGI(TAG, "Initialize touch controller XPT2046");
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << TOUCH_IRQ_GPIO_NUM,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,     
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,       // 如果底层中断由触摸库接管，这里设为 DISABLE
    };
    gpio_config(&io_conf);


    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_spi_config_t tp_io_config = {      
        .cs_gpio_num = TOUCH_CS_GPIO_NUM,            
        .dc_gpio_num = GPIO_NUM_NC,                     
        .spi_mode = LCD_SPI_MODE,                                  
        .pclk_hz = TOUCH_SPI_CLK_FREQ_HZ,          
        .trans_queue_depth = 3,                         
        .on_color_trans_done = NULL,                    
        .user_ctx = NULL,                               
        .lcd_cmd_bits = LCD_CMD_BITS,                              
        .lcd_param_bits = LCD_PARAM_BITS,                            
        .flags =                                        
        {                                               
            .dc_high_on_cmd = 0,                        
            .dc_low_on_data = 0,                        
            .dc_low_on_param = 0,                        
            .octal_mode = 0,                            
            .quad_mode = 0,                             
            .sio_mode = 0,                              
            .lsb_first = 0,                             
            .cs_high_active = 0                         
        }                                               
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &tp_io_config, &tp_io_handle));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_HEIGHT,
        .y_max = LCD_WIDTH,
        .rst_gpio_num = -1,
        .int_gpio_num = TOUCH_IRQ_GPIO_NUM,
        .levels = {
            .reset = 0,
            .interrupt = 0, 
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp));
}

static void bsp_display_lvgl_init(void)
{
    panel.h_res = LCD_WIDTH;
    panel.v_res = LCD_HEIGHT;
    panel.lcd_host = LCD_HOST;     
    panel.panel_handle = panel_handle; 
    panel.io_handle = io_handle;
    panel.tp_handle = tp;
    lvgl_port_init(&panel);
}

void bsp_display_init(void)
{
    // 初始化 LCD 显示
    bsp_display_LCD_init();

    // 初始化触摸屏
    bsp_display_touch_init();

    // 初始化lvgl
    bsp_display_lvgl_init();
}



void fill_screen(esp_lcd_panel_handle_t panel_handle,int x,int y, uint16_t color) {
    int w = x;
    int h = y;

    // 1. 将颜色值转换为大端
    uint16_t color_be = __builtin_bswap16(color);

    int rows_at_once = 10;
    uint16_t *buffer = heap_caps_malloc(w * rows_at_once * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!buffer) return;
    for (int i = 0; i < w * rows_at_once; i++) {
        buffer[i] = color_be;
    }

    // 2. 分块发送
    for (int y = 0; y < h; y += rows_at_once) {
        int current_rows = (y + rows_at_once <= h) ? rows_at_once : (h - y);
        esp_lcd_panel_draw_bitmap(panel_handle, 0, y, w, y + current_rows, buffer);
    }

    free(buffer);
}


