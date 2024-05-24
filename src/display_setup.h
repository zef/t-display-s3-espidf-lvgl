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

#include "lv_conf.h"
#include "lvgl.h"

#include "pin_config.h"

lv_display_t *display;
esp_lcd_panel_handle_t panel_handle = NULL;

static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *color_map) {
    printf("Flush Callback\n");

    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map));
    lv_disp_flush_ready(disp);
}


static bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    lv_disp_flush_ready(display);
    return false;
}
void configure_gpio() {
    const gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << LCD_PIN_RD),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    ESP_ERROR_CHECK(gpio_config(&input_conf));

    gpio_config_t output_pin_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LCD_PIN_POWER) |
                        (1ULL << LCD_PIN_BK_LIGHT)
    };
    ESP_ERROR_CHECK(gpio_config(&output_pin_config));

    gpio_set_level(LCD_PIN_POWER, 1);
    gpio_set_level(LCD_PIN_BK_LIGHT, 0);
}

void configure_lvgl() {
    lv_init();

    static lv_color_t buffer[LCD_BUFFER_SIZE];
    // static lv_color_t buffer2[LCD_BUFFER_SIZE];

    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_disp_set_default(display);
    // lv_display_set_user_data(display, panel_handle);
    lv_display_set_buffers(display, buffer, NULL, sizeof(buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush_callback);
}

void configure_lcd() {
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
        .max_transfer_bytes = LCD_BUFFER_SIZE * sizeof(lv_color_t),
        .psram_trans_align = PSRAM_DATA_ALIGNMENT,
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
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 35));

    gpio_set_level(LCD_PIN_BK_LIGHT, 1);

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

}

static void lvgl_tick_callback(void* arg) {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

// void lvgl_timer_handler(void *pvParameter) {
//     while (1) {
//         lv_timer_handler();
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }


void create_display_timers() {
    esp_timer_handle_t lvgl_timer;
    const esp_timer_create_args_t lvgl_timer_args = {
        .callback = &lvgl_tick_callback,
        .name = "LVGL Timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_timer_args, &lvgl_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_timer, pdMS_TO_TICKS(LVGL_TICK_PERIOD_MS)));

    // calling this from app_main instead because it's crashing when I do it this way or in a timer
    // xTaskCreatePinnedToCore(lvgl_timer_handler, "LVGL Task", 1024*8, NULL, 1, NULL, 0);
}

void configure_display() {
    printf("Setup Display...\n");
    configure_gpio();
    configure_lcd();
    configure_lvgl();
    create_display_timers();
}