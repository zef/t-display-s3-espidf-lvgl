#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// I'd like to not include this here, but I need it to call `lv_timer_handler()`. See comments below.
// #include "lvgl.h"

#include "display.h"
#include "screen.h"
#include "buttons.h"

int32_t progress = 20;

void button_one(void *arg, void *user_data) {
  printf("Button One!\n");
  if (progress < 100) {
    progress += 5;
    set_progress(progress);
  }
}

void button_two(void *arg, void *user_data) {
  printf("Button Two!\n");
  if (progress > 0) {
    progress -= 5;
    set_progress(progress);
  }
}

extern "C" void app_main() {
  setup_buttons();
  setup_display();

  update_ui([]() {
    show_screen();
    set_progress(progress);
  });
}
