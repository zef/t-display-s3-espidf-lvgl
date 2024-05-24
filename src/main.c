#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_err.h"
// #include "esp_log.h"

// I'd like to not include this here, but I need it to call `lv_timer_handler()`
#include "lvgl.h"

// internal code
#include "display.h"
#include "buttons.h"

void button_one(void *arg,void *user_data) {
    printf("Button Click!\n");
    setProgress(20);
}

void button_two(void *arg,void *user_data) {
    printf("Button Two!\n");
    setProgress(80);
}

void app_main(void) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    start_display();
    setup_buttons();

    while (true) {
        // I'm not sure why I can't extract this call to a separate task
        // but I've tried one on both cores and it crashes.
        // I also tired extracting a function that calls this to `display.h` and calling that function here instead
        // but even that causes crashing. I have no idea why.
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}