#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// I'd like to not include this here, but I need it to call `lv_timer_handler()`. See comments below.
#include "lvgl.h"

#include "display.h"
#include "buttons.h"

int32_t progress = 20;

void button_one(void *arg,void *user_data) {
    printf("Button One!\n");
    if (progress < 100) {
        progress += 5;
        set_progress(progress);
    }
}

void button_two(void *arg,void *user_data) {
    printf("Button Two!\n");
    if (progress > 0) {
        progress -= 5;
        set_progress(progress);
    }
}

void app_main(void) {
    setup_buttons();
    start_display();
    set_progress(progress);

    while (true) {
        // I'm not sure why I can't extract this call to a separate task
        // but I've tried one on both cores and it crashes.
        // I also tired extracting a function that calls this to `display.h` and calling that function here instead
        // but even that causes crashing. I have no idea why.
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
