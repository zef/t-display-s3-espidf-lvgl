#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_conf.h"
#include "lvgl.h"
#include "esp_log.h"
#include <string.h>

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "pin_config.h"
// #include "lv_drivers/display/fbdev.h"

static const char *TAG = "Debug";

lv_display_t *display;
esp_lcd_panel_io_handle_t io_handle = NULL;
static uint16_t *display_buffer[LVGL_LCD_BUF_SIZE];
// void *buf1 = NULL;

// static bool display_is_updating = false;

static bool lvgl_ready = false;

void heartbeat(void *pvParameter) {
    const TickType_t xDelay = pdMS_TO_TICKS(4000);

    while (1) {
        printf("|\n");
        vTaskDelay(xDelay);
    }

    vTaskDelete(NULL); // Delete the task (should not reach here)
}
void fireLVGLTimer() {
    while(1) {
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

// void fillBuffer(uint16_t color) {
//     for (int i = 0; i < LVGL_LCD_BUF_SIZE; ++i) {
//         display_buffer[i] = color;
//     }
// }

// static bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
// {
//     printf("Color Trans Done.\n");
//     display_is_updating = false;
//     return false;
// }

void print_color_map(uint8_t *color_map, size_t size) {
    printf("Color Map Values:\n");
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", color_map[i]); // Print each value in hexadecimal format
        if ((i + 1) % 16 == 0) {
            printf("\n"); // Add a new line every 16 values for better readability
        }
    }
    printf("\n");
}

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    printf("Flush Ready\n");
    if (lvgl_ready) {
        // lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
        // lv_display_t disp = 
        lv_disp_flush_ready(display);
    }
    return false;
}

// typedef void (*lv_display_flush_cb_t)(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);
static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *color_map) {
    printf("Flush Callback\n");
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);

    // fillBuffer(0x001F);

    // for (int i = 0; i < LVGL_LCD_BUF_SIZE; ++i) {
    //     color_map[i] = 0x001F;
    // }

    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    printf("offsets: %i, %i, %i, %i\n", offsetx1, offsety1, offsetx2, offsety2);
    print_color_map(color_map, 16*4);
    esp_err_t result = esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);

    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error drawing bitmap: %d", result);
    }
    // size_t size = lv_area_get_size(area);
    // size_t i;
    // for (i = 0; i <= size; i++) {
    //     display_buffer[i] = 0xff;
    // }
    // esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, display_buffer);


    // esp_lcd_panel_draw_bitmap(panel_handle, 32, 32, 64, 64, [0xff]);
    lv_disp_flush_ready(disp);
}

// void lcd_init_pins() {
//     // Configure the output pins
//     gpio_config_t io_conf;
//     io_conf.intr_type = GPIO_INTR_DISABLE;
//     io_conf.mode = GPIO_MODE_OUTPUT;
//     io_conf.pin_bit_mask = (1ULL << PIN_LCD_RES) |
//                            (1ULL << PIN_LCD_CS) |
//                            (1ULL << PIN_LCD_DC) |
//                            (1ULL << PIN_LCD_WR) |
//                            (1ULL << PIN_LCD_RD); // Set RD as output
//     io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
//     gpio_config(&io_conf);

//     // Initialize the output pins to default states
//     gpio_set_level(PIN_LCD_RES, 1);
//     gpio_set_level(PIN_LCD_CS, 1);
//     gpio_set_level(PIN_LCD_DC, 1);
//     gpio_set_level(PIN_LCD_WR, 1);
//     gpio_set_level(PIN_LCD_RD, 1); // RD pin high (inactive state)
// }

void configure_pin_power_on() {
    gpio_config_t pwr_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_POWER_ON
    };
    ESP_ERROR_CHECK(gpio_config(&pwr_gpio_config));
    gpio_set_level(PIN_POWER_ON, 1);
    // // Configure the PIN_POWER_ON as an output pin
    // gpio_config_t io_conf;
    // io_conf.intr_type = GPIO_INTR_DISABLE;
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // io_conf.pin_bit_mask = (1ULL << PIN_POWER_ON);
    // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // gpio_config(&io_conf);

    // // Set PIN_POWER_ON to high to turn on the power
    // gpio_set_level(PIN_POWER_ON, 1);
}

void lcd_init_pins() {
    configure_pin_power_on();

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_LCD_RES) |
                           (1ULL << PIN_LCD_CS) |
                           (1ULL << PIN_LCD_DC) |
                           (1ULL << PIN_LCD_WR);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_config_t io_conf_rd;
    io_conf_rd.intr_type = GPIO_INTR_DISABLE;
    io_conf_rd.mode = GPIO_MODE_INPUT;
    io_conf_rd.pin_bit_mask = (1ULL << PIN_LCD_RD);
    io_conf_rd.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf_rd.pull_up_en = GPIO_PULLUP_ENABLE; // Enable pull-up if needed
    gpio_config(&io_conf_rd);

    // Initialize the output pins to default states
    gpio_set_level(PIN_LCD_RES, 1);
    gpio_set_level(PIN_LCD_CS, 1);
    gpio_set_level(PIN_LCD_DC, 1);
    gpio_set_level(PIN_LCD_WR, 1);

    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_LCD_BL
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_set_level(PIN_LCD_BL, 0);
}

void setup_display() {
    lcd_init_pins();

    // gpio_set_level(PIN_LCD_RES, 0);
    // vTaskDelay(pdMS_TO_TICKS(100));
    // gpio_set_level(PIN_LCD_RES, 1);

    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = PIN_LCD_DC,
        .wr_gpio_num = PIN_LCD_WR,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .data_gpio_nums =
        {
            PIN_LCD_D0,
            PIN_LCD_D1,
            PIN_LCD_D2,
            PIN_LCD_D3,
            PIN_LCD_D4,
            PIN_LCD_D5,
            PIN_LCD_D6,
            PIN_LCD_D7,
        },
        .bus_width = 8,
        .max_transfer_bytes = LVGL_LCD_BUF_SIZE * sizeof(uint16_t) * 2,
        // .psram_trans_align = EXAMPLE_PSRAM_DATA_ALIGNMENT,
        // .sram_trans_align = 4
    };

    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = PIN_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 20,
        .on_color_trans_done = notify_lvgl_flush_ready,
        // .on_color_trans_done = on_color_trans_done,
        .user_ctx = &display,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels =
        {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_LCD_RES,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true);

    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true);
    // the gap is LCD panel specific, even panels with the same driver IC, can
    // have different gap value
    esp_lcd_panel_set_gap(panel_handle, 0, 35);

    gpio_set_level(PIN_LCD_BL, 1);

    lv_init();
    // vTaskDelay(pdMS_TO_TICKS(1500));
    // printf("sizeof(display_buffer) %du\n", sizeof(display_buffer));
    // printf("LVGL_LCD_BUF_SIZE * sizeof(uint16_t) %du\n", LVGL_LCD_BUF_SIZE * sizeof(uint16_t));

    esp_lcd_panel_disp_on_off(panel_handle, true);

    // memset(display_buffer, 0x0F, sizeof(display_buffer));
    // ESP_LOGI(TAG, "buffer @%p", display_buffer);
    // ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 88, 88, display_buffer));

    // buf1 = heap_caps_malloc(LVGL_LCD_BUF_SIZE * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    // memset(buf1, 0xff, LVGL_LCD_BUF_SIZE * sizeof(uint16_t));

    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_disp_set_default(display);
    lv_display_set_user_data(display, panel_handle);
    lv_display_set_buffers(display, display_buffer, NULL, sizeof(display_buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
    // lv_display_set_buffers(display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush_callback);

    lvgl_ready = true;
    xTaskCreatePinnedToCore(fireLVGLTimer, "lvgl Timer", 10000, NULL, 2, NULL, 1);
}

void display_screen() {
    // lv_obj_t *scr = lv_screen_active();
    // lv_obj_t *bg = lv_obj_create(scr);
    // lv_obj_set_size(bg, SCREEN_WIDTH, SCREEN_HEIGHT);
    // lv_obj_set_style_bg_color(bg, lv_color_hex(0x003a57), LV_PART_MAIN);
    // // lv_obj_set_style_bg_color(display, lv_color_hex(0x003a57), LV_PART_MAIN);

    // lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x00), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void app_main() {
    printf("Hello!\n");
    vTaskDelay(pdMS_TO_TICKS(1500));
    xTaskCreate(heartbeat, "heartbeat", 2048, NULL, 1, NULL);
    printf("Start!\n");
    vTaskDelay(pdMS_TO_TICKS(2500));
    printf("Setup Display.\n");
    setup_display();
    // vTaskDelay(pdMS_TO_TICKS(2500));
    // printf("Screen.\n");
    // display_screen();

    // while (true) {
    //     printf("/\n");
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}