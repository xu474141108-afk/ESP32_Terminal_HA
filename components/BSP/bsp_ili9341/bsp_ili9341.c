#include <driver/gpio.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_commands.h>
#include <esp_log.h>
#include <esp_rom_gpio.h>
#include <esp_check.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include "bsp_ili9341.h"


static const char *TAG = "ili9341";

typedef struct
{
    uint8_t cmd;
    uint8_t data[16];
    uint8_t data_bytes;
} lcd_init_cmd_t;

typedef struct
{
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    uint8_t memory_access_control;
    uint8_t color_mode;
} ili9341_panel_t;

enum ili9341_constants
{
    ILI9341_INTRFC_MODE_CTL = 0xB0,
    ILI9341_FRAME_RATE_NORMAL_CTL = 0xB1,
    ILI9341_INVERSION_CTL = 0xB4,
    ILI9341_FUNCTION_CTL = 0xB6,
    ILI9341_ENTRY_MODE_CTL = 0xB7,
    ILI9341_POWER_CTL_ONE = 0xC0,
    ILI9341_POWER_CTL_TWO = 0xC1,
    ILI9341_POWER_CTL_THREE = 0xC5,

    ILI9341_POSITIVE_GAMMA_CTL = 0xE0,
    ILI9341_NEGATIVE_GAMMA_CTL = 0xE1,



    ILI9341_ADJUST_CTL_THREE = 0xF7,

    ILI9341_COLOR_MODE_16BIT = 0x55,
    ILI9341_COLOR_MODE_18BIT = 0x66,

    ILI9341_INTERFACE_MODE_USE_SDO = 0x00,
    ILI9341_INTERFACE_MODE_IGNORE_SDO = 0x80,

    ILI9341_IMAGE_FUNCTION_DISABLE_24BIT_DATA = 0x00,

    ILI9341_WRITE_MODE_BCTRL_DD_ON = 0x28,

    ILI9341_INIT_LENGTH_MASK = 0x1F,
    ILI9341_INIT_DONE_FLAG = 0xFF
};

static esp_err_t panel_ili9341_del(esp_lcd_panel_t *panel)
{
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);

    if (ili9341->reset_gpio_num >= 0)
    {
        gpio_reset_pin(ili9341->reset_gpio_num);
    }

    ESP_LOGI(TAG, "del ili9341 panel @%p", ili9341);
    free(ili9341);
    return ESP_OK;
}

static esp_err_t panel_ili9341_reset(esp_lcd_panel_t *panel)
{
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9341->io;

    if (ili9341->reset_gpio_num >= 0)
    {
        ESP_LOGI(TAG, "Setting GPIO:%d to %d", ili9341->reset_gpio_num,
                 ili9341->reset_level);
        // perform hardware reset
        gpio_set_level(ili9341->reset_gpio_num, ili9341->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_LOGI(TAG, "Setting GPIO:%d to %d", ili9341->reset_gpio_num,
                 !ili9341->reset_level);
        gpio_set_level(ili9341->reset_gpio_num, !ili9341->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    else
    {
        ESP_LOGI(TAG, "Sending SW_RESET to display");
        esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return ESP_OK;
}

static esp_err_t panel_ili9341_init(esp_lcd_panel_t *panel)
{
    ESP_LOGI(TAG, "Initializing display");
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9341->io;

    lcd_init_cmd_t ili9341_init[] =
    {
        //{cmd, { data }, data_size}
        { ILI9341_POSITIVE_GAMMA_CTL,
            {0x1F, 0x36, 0x36, 0x3A, 0x0C,
             0x05, 0x4F, 0X87, 0x3C, 0x08,
             0x11, 0x35, 0x19, 0x13, 0x00},
            15
        },

        { ILI9341_NEGATIVE_GAMMA_CTL,
            {0x00, 0x09, 0x09, 0x05, 0x13,
             0x0A, 0x30, 0x78, 0x43, 0x07,
             0x0E, 0x0A, 0x26, 0x2C, 0x1F},
            15
        },
        { ILI9341_POWER_CTL_ONE, { 0x23 }, 1 },          // Power control 1, GVDD=4.75V 
        { ILI9341_POWER_CTL_TWO, { 0x11 }, 1 },          // Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3
        { ILI9341_POWER_CTL_THREE, { 0x43, 0x4C}, 2 },   // VCOM control 1, VCOMH=4.025V, VCOML=-0.950V
        //{0x36, { 0x2 }, 1},                             // Memory Access Control, row address/column address, bottom to top refresh
        { LCD_CMD_MADCTL, { ili9341->memory_access_control }, 1 },
        { LCD_CMD_COLMOD, { ili9341->color_mode }, 1 },  // 16BIT color mode for RGB interface
        { ILI9341_INTRFC_MODE_CTL, { ILI9341_INTERFACE_MODE_USE_SDO }, 1 },

        
        { ILI9341_FRAME_RATE_NORMAL_CTL, { 0x00,0x1B }, 2 }, // 0x00 for 70Hz, 0x01 for 100Hz, 0x1B for 60Hz

        { ILI9341_INVERSION_CTL, { 0x02 }, 1 },


        { ILI9341_FUNCTION_CTL, { 0x08, 0x82, 0x27 }, 3},    // Display function control
        { ILI9341_ENTRY_MODE_CTL, { 0x07 }, 1 },    // Entry mode set, Low vol detect disabled, normal display

        { ILI9341_ADJUST_CTL_THREE, { 0x20 }, 1 },    // Adjust control 3, default 0x20
        { LCD_CMD_NOP, { 0 }, ILI9341_INIT_DONE_FLAG },
    };

    ESP_LOGI(TAG, "Initializing ILI9341");
    int cmd = 0;
    while ( ili9341_init[cmd].data_bytes != ILI9341_INIT_DONE_FLAG )
    {
        ESP_LOGD(TAG, "Sending CMD: %02x, len: %d", ili9341_init[cmd].cmd,
                 ili9341_init[cmd].data_bytes & ILI9341_INIT_LENGTH_MASK);
        esp_lcd_panel_io_tx_param(
            io, ili9341_init[cmd].cmd, ili9341_init[cmd].data,
            ili9341_init[cmd].data_bytes & ILI9341_INIT_LENGTH_MASK);
        cmd++;
    }

    // Take the display out of sleep mode.
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Turn on the display.
    esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Initialization complete");

    return ESP_OK;
}

#define SEND_COORDS(start, end, io, cmd)                \
    esp_lcd_panel_io_tx_param(io, cmd, (uint8_t[]) {    \
        (start >> 8) & 0xFF,                            \
        start & 0xFF,                                   \
        ((end - 1) >> 8) & 0xFF,                        \
        (end - 1) & 0xFF,                               \
    }, 4)

static esp_err_t panel_ili9341_draw_bitmap(
    esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end,
    const void *color_data)
{
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);
    assert((x_start < x_end) && (y_start < y_end) &&
            "starting position must be smaller than end position");
    esp_lcd_panel_io_handle_t io = ili9341->io;

    x_start += ili9341->x_gap;
    x_end += ili9341->x_gap;
    y_start += ili9341->y_gap;
    y_end += ili9341->y_gap;

    size_t color_data_len = (x_end - x_start) * (y_end - y_start);
    
    SEND_COORDS(x_start, x_end, io, LCD_CMD_CASET);
    SEND_COORDS(y_start, y_end, io, LCD_CMD_RASET);

    // When the ILI9341 is used in 18-bit color mode we need to convert the
    // incoming color data from RGB565 (16-bit) to RGB666.
    if (ili9341->color_mode == ILI9341_COLOR_MODE_18BIT)
    {
        esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, color_data_len * 3);
    }
    else
    {
        esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, color_data_len * 2);
    }

    return ESP_OK;
}

#undef SEND_COORDS

static esp_err_t panel_ili9341_invert_color(
    esp_lcd_panel_t *panel, bool invert_color_data)
{
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9341->io;
    
    if (invert_color_data)
    {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_INVON, NULL, 0);
    }
    else
    {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_INVOFF, NULL, 0);
    }
    
    return ESP_OK;
}

static esp_err_t panel_ili9341_mirror(
    esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9341->io;
    if (mirror_x)
    {
        ili9341->memory_access_control &= ~LCD_CMD_MX_BIT;
    }
    else
    {
        ili9341->memory_access_control |= LCD_CMD_MX_BIT;
    }
    if (mirror_y)
    {
        ili9341->memory_access_control |= LCD_CMD_MY_BIT;
    }
    else
    {
        ili9341->memory_access_control &= ~LCD_CMD_MY_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &ili9341->memory_access_control, 1);
    return ESP_OK;
}

static esp_err_t panel_ili9341_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9341->io;
    if (swap_axes)
    {
        ili9341->memory_access_control |= LCD_CMD_MV_BIT;
    }
    else
    {
        ili9341->memory_access_control &= ~LCD_CMD_MV_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &ili9341->memory_access_control, 1);
    return ESP_OK;
}

static esp_err_t panel_ili9341_set_gap(
    esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);
    ili9341->x_gap = x_gap;
    ili9341->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_ili9341_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    ili9341_panel_t *ili9341 = __containerof(panel, ili9341_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9341->io;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    // In ESP-IDF v4.x the API used false for "on" and true for "off"
    // invert the logic to be consistent with IDF v5.x.
    on_off = !on_off;
#endif

    if (on_off)
    {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);
    }
    else
    {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPOFF, NULL, 0);
    }

    // give time for the ILI9341 to recover after an on/off command
    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

esp_err_t esp_lcd_new_panel_ili9341(
    const esp_lcd_panel_io_handle_t io,
    const esp_lcd_panel_dev_config_t *panel_dev_config,
    esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    ili9341_panel_t *ili9341 = NULL;
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG,
                      err, TAG, "invalid argument");
    ili9341 = (ili9341_panel_t *)(calloc(1, sizeof(ili9341_panel_t)));
    ESP_GOTO_ON_FALSE(ili9341, ESP_ERR_NO_MEM, err, TAG, "no mem for ili9341 panel");

    if (panel_dev_config->reset_gpio_num >= 0)
    {
        gpio_config_t cfg;
        memset(&cfg, 0, sizeof(gpio_config_t));
        esp_rom_gpio_pad_select_gpio(panel_dev_config->reset_gpio_num);
        cfg.pin_bit_mask = BIT64(panel_dev_config->reset_gpio_num);
        cfg.mode = GPIO_MODE_OUTPUT;
        ESP_GOTO_ON_ERROR(gpio_config(&cfg), err, TAG,
                          "configure GPIO for RESET line failed");
    }

    if (panel_dev_config->bits_per_pixel == 16)
    {
        ili9341->color_mode = ILI9341_COLOR_MODE_16BIT;
    }
    else
    {
        ili9341->color_mode = ILI9341_COLOR_MODE_18BIT;
    }

    ili9341->memory_access_control = LCD_CMD_MX_BIT | LCD_CMD_BGR_BIT;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
    switch (panel_dev_config->color_space)
    {
        case ESP_LCD_COLOR_SPACE_RGB:
            ESP_LOGI(TAG, "Configuring for RGB color order");
            ili9341->memory_access_control &= ~LCD_CMD_BGR_BIT;
            break;
        case ESP_LCD_COLOR_SPACE_BGR:
            ESP_LOGI(TAG, "Configuring for BGR color order");
            break;
        default:
            ESP_GOTO_ON_FALSE(false, ESP_ERR_INVALID_ARG, err, TAG,
                              "Unsupported color mode!");
    }
#else
    switch (panel_dev_config->rgb_ele_order)
    {
    case LCD_RGB_ELEMENT_ORDER_RGB:
            ESP_LOGI(TAG, "Configuring for RGB color order");
            ili9341->memory_access_control &= ~LCD_CMD_BGR_BIT;
            break;
    case LCD_RGB_ELEMENT_ORDER_BGR:
            ESP_LOGI(TAG, "Configuring for BGR color order");
            break;
        default:
            ESP_GOTO_ON_FALSE(false, ESP_ERR_INVALID_ARG, err, TAG,
                              "Unsupported color mode!");
    }
#endif

    ili9341->io = io;
    ili9341->reset_gpio_num = panel_dev_config->reset_gpio_num;
    ili9341->reset_level = panel_dev_config->flags.reset_active_high;
    ili9341->base.del = panel_ili9341_del;
    ili9341->base.reset = panel_ili9341_reset;
    ili9341->base.init = panel_ili9341_init;
    ili9341->base.draw_bitmap = panel_ili9341_draw_bitmap;
    ili9341->base.invert_color = panel_ili9341_invert_color;
    ili9341->base.set_gap = panel_ili9341_set_gap;
    ili9341->base.mirror = panel_ili9341_mirror;
    ili9341->base.swap_xy = panel_ili9341_swap_xy;
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    ili9341->base.disp_off = panel_ili9341_disp_on_off;
#else
    ili9341->base.disp_on_off = panel_ili9341_disp_on_off;
#endif
    *ret_panel = &(ili9341->base);
    ESP_LOGI(TAG, "new ili9341 panel:: @%p, %p", ili9341,&(ili9341->base));

    return ESP_OK;

err:
    if (ili9341)
    {
        if (panel_dev_config->reset_gpio_num >= 0)
        {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(ili9341);
    }
    return ret;
}




