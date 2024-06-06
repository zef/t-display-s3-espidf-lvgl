#include <stdio.h>
#include <string>

#include "lvgl.h"

#include "display.h"
#include "screen.h"


lv_obj_t *statusLabel;
lv_obj_t *progressLabel;
lv_obj_t *progressBar;

void set_progress(int32_t value) {
    int32_t clamped_value = LV_CLAMP(0, value, 100);
    std::string labelText = std::to_string(clamped_value) + "/100";

    lvgl_lock();
    lv_bar_set_value(progressBar, clamped_value, LV_ANIM_OFF);
    lv_label_set_text(progressLabel, labelText.c_str());
    lvgl_unlock();
}

void create_progress_bar() {
    progressBar = lv_bar_create(lv_screen_active());
    lv_obj_set_size(progressBar, SCREEN_WIDTH - 40, 22);
    lv_obj_center(progressBar);
}

void create_labels() {
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);

    statusLabel = lv_label_create(lv_screen_active());
    lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 0, 24);
    lv_label_set_text(statusLabel, "Hello from T-Display!");

    progressLabel = lv_label_create(lv_screen_active());
    lv_obj_align_to(progressLabel, progressBar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void show_screen() {
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x375686), LV_PART_MAIN);
    create_progress_bar();
    create_labels();
}
