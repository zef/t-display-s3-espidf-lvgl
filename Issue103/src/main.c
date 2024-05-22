#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_err.h"
// #include "esp_log.h"

#include "display.h"

void app_main(void) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    startDisplay();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}