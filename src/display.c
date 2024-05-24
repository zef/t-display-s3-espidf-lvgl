#include "display.h"
#include "display_setup.h"

#include "lvgl.h"

lv_obj_t *statusLabel;
lv_obj_t *subLabel;
lv_obj_t *progressBar;
static lv_style_t largeFontStyle;

void setProgress(int32_t value) {
    lv_bar_set_value(progressBar, value, LV_ANIM_ON);
}

void startProgress() {
    progressBar = lv_bar_create(lv_screen_active());
    lv_obj_set_size(progressBar, 320 - 40, 22);
    lv_obj_center(progressBar);
}

void setupLabels() {
    lv_style_init(&largeFontStyle);
    lv_style_set_text_font(&largeFontStyle, &lv_font_montserrat_18);
    
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);
    // lv_obj_set_style_text_font(lv_screen_active(), &lv_font_montserrat_18, LV_PART_MAIN);

    statusLabel = lv_label_create(lv_screen_active());
    lv_obj_add_style(statusLabel, &largeFontStyle, LV_PART_MAIN);
    lv_obj_align(statusLabel, LV_ALIGN_CENTER, 0, -40);
    lv_label_set_text(statusLabel, "Flashing Firmware...");

    subLabel = lv_label_create(lv_screen_active());
    lv_obj_align(subLabel, LV_ALIGN_CENTER, 0, 30);
    lv_label_set_text(subLabel, "22/100");

    startProgress();
}

void start_display() {
    configure_display();

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);
    setupLabels();
}
