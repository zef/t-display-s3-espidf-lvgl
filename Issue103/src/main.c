#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "pin_config.h"

static const char *TAG = "example";

esp_lcd_panel_handle_t panel_handle = NULL;
void *buf1 = NULL;
void *buf2 = NULL;
size_t buff_idx = 0;
// static bool trans_done = true;

static void write_color()
{
    // if (!trans_done)
    //     return;

    // trans_done = false;
    void *buff = (buff_idx++ % 2) ? buf1 : buf2;

    ESP_LOGI(TAG, "writing color: %d, %d", buff_idx, buff_idx % 2);
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 64, 64, buff);
}

// size_t trans_done_calls = 0;

static bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    // trans_done = true;
    // trans_done_calls++;

    return false;
}

// static void tick_timer_cb(void *arg)
// {
//     ESP_LOGI(TAG, "trans_done_calls: %zu", trans_done_calls);
//     trans_done_calls = 0;
// }

void app_main(void)
{
    gpio_config_t pwr_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_PIN_POWER};
    ESP_ERROR_CHECK(gpio_config(&pwr_gpio_config));
    gpio_set_level(LCD_PIN_POWER, LCD_BK_LIGHT_ON_LEVEL);

    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_PIN_BK_LIGHT};
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_set_level(LCD_PIN_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL);

    const gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << LCD_PIN_RD),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    ESP_ERROR_CHECK(gpio_config(&input_conf));

    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = LCD_PIN_DC,
        .wr_gpio_num = LCD_PIN_PCLK,
        .data_gpio_nums = {
            LCD_PIN_DATA0,
            LCD_PIN_DATA1,
            LCD_PIN_DATA2,
            LCD_PIN_DATA3,
            LCD_PIN_DATA4,
            LCD_PIN_DATA5,
            LCD_PIN_DATA6,
            LCD_PIN_DATA7,
        },
        .bus_width = 8,
        .max_transfer_bytes = LCD_H_RES * 100 * sizeof(uint16_t),
        .psram_trans_align = EXAMPLE_PSRAM_DATA_ALIGNMENT,
        .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = LCD_PIN_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 20,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .swap_color_bytes = 1, // Swap can be done in LvGL (default) or DMA
        },
        .on_color_trans_done = on_color_trans_done,
        .user_ctx = NULL,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    // Set inversion, x/y coordinate order, x/y mirror according to your LCD module spec
    // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 35));

    gpio_set_level(LCD_PIN_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL);

    ESP_LOGI(TAG, "Initialize UI");
    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    buf1 = heap_caps_malloc(LCD_H_RES * 100 * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    buf2 = heap_caps_malloc(LCD_H_RES * 100 * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    assert(buf1);
    assert(buf2);

    memset(buf1, 0x0, LCD_H_RES * 100 * 2);
    memset(buf2, 0xff, LCD_H_RES * 100 * 2);

    ESP_LOGI(TAG, "buf1@%p, buf2@%p", buf1, buf2);

    // esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 64, 64, buf1);

    // const esp_timer_create_args_t tick_timer_args = {
    //     .callback = &tick_timer_cb,
    //     .name = "tick_timer"};
    // esp_timer_handle_t tick_timer = NULL;
    // ESP_ERROR_CHECK(esp_timer_create(&tick_timer_args, &tick_timer));
    // ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 1000000));

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    while (1)
    {
        write_color();

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}