#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iot_button.h"
#include "pin_config.h"
// #include "driver/gpio.h"
// #include "esp_err.h"
// #include "esp_log.h"
#include "lvgl.h"

#include "display.h"
void button_one(void *arg,void *usr_data) {
    printf("Button Click!\n");
    setProgress(20);
}

void button_two(void *arg,void *usr_data) {
    printf("Button Two!\n");
    setProgress(80);
}

void setup_buttons() {
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
        .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
        .gpio_button_config = {
            .gpio_num = PIN_BUTTON_1,
            .active_level = 0,
        },
    };
    button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_one, NULL);

    button_config_t gpio_btn2_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
        .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
        .gpio_button_config = {
            .gpio_num = PIN_BUTTON_2,
            .active_level = 0,
        },
    };
    button_handle_t gpio_btn2 = iot_button_create(&gpio_btn2_cfg);
    iot_button_register_cb(gpio_btn2, BUTTON_SINGLE_CLICK, button_two, NULL);
}

void app_main(void) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    start_display();
    setup_buttons();

    while (true) {
        // I'm not sure why I can't extract this call to a separate task
        // but I've tried one on both cores and it crashes.
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}