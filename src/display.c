#include "display.h"
#include "display_setup.h"

#include "lvgl.h"

lv_obj_t *statusLabel;
lv_obj_t *progressLabel;
lv_obj_t *progressBar;

void set_progress(int32_t value) {
    value = LV_CLAMP(0, value, 100);
    
    lv_bar_set_value(progressBar, value, LV_ANIM_OFF);
    
    char labelText[16];
    snprintf(labelText, sizeof(labelText), "%ld/100", value);
    lv_label_set_text(progressLabel, labelText);
}

void create_progress_bar() {
    progressBar = lv_bar_create(lv_screen_active());
    lv_obj_set_size(progressBar, 320 - 40, 22);
    lv_obj_center(progressBar);
}

void create_labels() {
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);

    statusLabel = lv_label_create(lv_screen_active());
    lv_obj_align(statusLabel, LV_ALIGN_CENTER, 0, -40);
    lv_label_set_text(statusLabel, "Hello from T-Display!");

    progressLabel = lv_label_create(lv_screen_active());
    lv_obj_align(progressLabel, LV_ALIGN_CENTER, 0, 30);
}

void start_display() {
    setup_display();
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x375686), LV_PART_MAIN);
    create_progress_bar();
    create_labels();
}
