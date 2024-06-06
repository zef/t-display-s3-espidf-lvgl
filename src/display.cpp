#include "display.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#include "lvgl.h"

lv_display_t *display;
esp_lcd_panel_handle_t panel_handle = NULL;

static SemaphoreHandle_t lvgl_mux = NULL;

static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *color_map) {
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
        .pin_bit_mask = (1ULL << LCD_PIN_POWER) |
                        (1ULL << LCD_PIN_BK_LIGHT),
        .mode = GPIO_MODE_OUTPUT
    };
    ESP_ERROR_CHECK(gpio_config(&output_pin_config));

    gpio_set_level((gpio_num_t)LCD_PIN_POWER, 1);
    gpio_set_level((gpio_num_t)LCD_PIN_BK_LIGHT, 0);
}

void configure_lcd() {
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = LCD_PIN_DC,
        .wr_gpio_num = LCD_PIN_PCLK,
        .clk_src = LCD_CLK_SRC_DEFAULT,
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
        .on_color_trans_done = on_color_trans_done,
        .user_ctx = NULL,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .swap_color_bytes = 1, // Swap can be done in LvGL (default) or DMA
        }
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

    gpio_set_level((gpio_num_t)LCD_PIN_BK_LIGHT, 1);

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

void configure_lvgl() {
    lv_init();

    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_disp_set_default(display);

    static lv_color_t buffer[LCD_BUFFER_SIZE];
    static lv_color_t buffer2[LCD_BUFFER_SIZE];
    lv_display_set_buffers(display, buffer, buffer2, sizeof(buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_set_flush_cb(display, flush_callback);
}


static void lvgl_tick_callback(void* arg) {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

bool lvgl_lock(int timeout_ms) {
    // Convert timeout in milliseconds to FreeRTOS ticks
    // If `timeout_ms` is set to -1, the program will block until the condition is met
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
}

void lvgl_unlock() {
    xSemaphoreGiveRecursive(lvgl_mux);
}

// https://github.com/espressif/esp-idf/blob/003f3bb5dc7c8af8b71926b7a0118cfc503cab11/examples/peripherals/lcd/i80_controller/main/i80_controller_example_main.c#L163
// I was expecting to use this to call `lv_timer_handler`, but it's crashing when I do it this way, or in a timer
// so calling this from app_main instead. It seems to be related to a watchdog timer issue. I played with that a bit but couldn't get it working.
//
// would be at end of create_display_timers()
// xTaskCreate(handle_lvgl_timer, "LVGL Timer Handler Task", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL);
//
// void handle_lvgl_timer(void *pvParameter) {
//     uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
//     while (1) {
//         task_delay_ms = fire_lvgl_timer(task_delay_ms);
//     }
// }

uint32_t fire_lvgl_timer(uint32_t task_delay_ms) {
    // Lock the mutex because LVGL APIs are not thread-safe
    if (lvgl_lock()) {
        task_delay_ms = lv_timer_handler();
        lvgl_unlock();
    }
    task_delay_ms = LV_CLAMP(LVGL_TASK_MIN_DELAY_MS, task_delay_ms, LVGL_TASK_MAX_DELAY_MS);
    vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    return task_delay_ms;
}

void create_display_timers() {
    // this timer is used to call `lv_tick_inc`, which is important for animation timings.
    esp_timer_handle_t lvgl_timer;
    const esp_timer_create_args_t lvgl_timer_args = {
        .callback = &lvgl_tick_callback,
        .name = "LVGL Timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_timer_args, &lvgl_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_timer, pdMS_TO_TICKS(LVGL_TICK_PERIOD_MS)));

    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    assert(lvgl_mux);
}

void update_ui(const std::function<void()>& block) {
    if (lvgl_lock(-1)) {
        block();
        lvgl_unlock();
    } else {
        printf("Failed to lock the LVGL mutex\n");
        block();
    }
}

void setup_display() {
    configure_gpio();
    configure_lcd();
    configure_lvgl();
    create_display_timers();
}
